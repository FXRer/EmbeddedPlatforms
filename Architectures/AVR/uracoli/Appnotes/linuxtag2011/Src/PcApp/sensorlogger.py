#   Copyright (c) 2011 Axel Wachtler, Matthias Vorwerk
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   * Neither the name of the authors nor the names of its contributors
#     may be used to endorse or promote products derived from this software
#     without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
#   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.

# $Id$
"""
Stores the sensor data received from a serial line into a RRD database.

Usage:
    python sensorlogger.py [OPTIONS]

    Options:
     -h : print help and exit
     -v : be verbose
     -D : run as daemon
     -C : name of config file (default sensor.cfg)
"""

# === import ==================================================================
# global moduls
import getopt
import os
import rrdtool
import serial
import sys
import threading
import time
import traceback
# local modules
import utils

# === globals =================================================================
VERSION = "$Revision$"
VERBOSE = 0
RUNASDAEMON = False
CFGFILE = "sensor.cfg"
CFG = { }

# dictionaries used to store new and known sensors
NEWBIE_SENSORS = {}
SENSORS = {}

# === classes =================================================================

# Storage class for meta information of a sensor
class RRDObject(object):
    addr = None
    dbname = None
    dbkeys = None
    name = None
    def __repr__(self):
        return "RRD(a=%s,l=%s,k=%s)" % (self.addr, self.dbname, self.dbkeys)

# === functions ===============================================================

##
# The ressource consuming computations, that are needed for scaling the
# raw sensor data are done on the PC.
#
def scale_parameters(params):
    if params.has_key("sht_t"):
        raw = params["sht_t"]
        params["sht_t"] = 175.72 * raw/65536.0 - 46.85;
    if params.has_key("sht_rh"):
        raw = params["sht_rh"]
        params["sht_rh"] = 125 * raw/65536.0 - 6;
    if params.has_key("acc"):
        params['acc_x'] = params['acc'][0]/64.0
        params['acc_y'] = params['acc'][1]/64.0
        params['acc_z'] = params['acc'][2]/64.0
        del(params['acc'])
    if params.has_key("vmcu"):
        raw = params["vmcu"]
        params["vmcu"] = raw/1000.0 ;

##
# Create a RRD object for a sensor node
#
def rrd_create(tstamp, addr, **kwargs):
    global SENSORS
    r = RRDObject()
    r.addr = addr
    r.name = CFG['nodes'].get(r.addr,"xxx")
    r.dbname = "%s/sensor_%04x.rrd" % (CFG['workdir'], addr)
    r.dbkeys = kwargs.keys()
    r.dbkeys.sort()

    dsrc = []
    for k in r.dbkeys:
        dsrc.append("DS:%s:GAUGE:%d:U:U" % (k.strip(), CFG['rrdstep']))

    if not os.path.isfile(r.dbname):
        args = ["--start", str(tstamp-CFG['rrdstep']), "--step", str(CFG['rrdstep'])]
        args += dsrc
        args += CFG['rra']

        rrdtool.create(r.dbname, *args)
        if VERBOSE > 0:
            utils.debug("create addr=%s rrd_file=%s " % (addr, r.dbname))
    SENSORS[addr] = r


##
# Update the RRDB of a sensor node.
#
def rrd_update(tstamp, addr, **kwargs):
    global SENSORS
    r = SENSORS.get(addr)
    if r != None:
        values = [str(kwargs.get(v,"42")) for v in r.dbkeys]
        arg = str(tstamp)+":"+":".join(values)
        rrdtool.update(r.dbname, arg)

#==================================================================
##

if __name__ == "__main__":
    opts, args = getopt.getopt(sys.argv[1:], "hvD")
    for o,v in opts:
        if o == "-h":
            print __doc__
            sys.exit(0)
        if o == "-v":
            VERBOSE += 1
        elif o == "-D":
            RUNASDAEMON = True

    # read config file
    CFGFILE = os.path.abspath(CFGFILE)
    if not os.path.isfile(CFGFILE):
        print "error: config file %s does not exist" % ( CFGFILE )
        sys.exit(1)
    CFG = utils.read_config(CFGFILE)

    try:
        if RUNASDAEMON:
            utils.daemonize(CFG['workdir'])
    except SystemExit:
        sys.exit(0)

    sport = serial.Serial(CFG['serialport'], CFG['baudrate'])
    sport.close()
    sport.open()
    print "sensorlogger v%s\n reading config %s\n started at %s" % (VERSION, CFGFILE, time.asctime())
    sys.stdout.flush()
    while 1:
        try:
            x = sport.readline().strip()
            # eliminate 0 bytes ... bug in buffer handling sensorapp.c
            x = x.replace("\0","")
            x = x.strip("\r")

            # in the first approximation these to checks should prevent
            # dangerous code.
            if x.find("(")>=0 or x.find("import") >= 0:
                raise Exception("InsecureExpression", x )
            # evaluate the data to a dictionary.
            y = eval("dict(%s)" % x)
            if y.has_key('addr'):
                y["tstamp"] = int(time.time())
                scale_parameters(y)
                if VERBOSE > 1:
                    utils.debug("rx_serial: %s" %  y )
                if not SENSORS.has_key(y["addr"]):
                    rrd_create(**y)
                rrd_update(**y)
        except KeyboardInterrupt:
            break
        except:
            if VERBOSE > 1:
                utils.debug("rx_error: %s" % str(sys.exc_info()[1]))
            if VERBOSE > 2:
                traceback.print_exc()
