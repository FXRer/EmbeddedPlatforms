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

# === import ==================================================================
import getopt
import os
import rrdtool
import serial
import sys
import threading
import time
import traceback


# === globals =================================================================

# application parameters
WORKDIR="data"
SPORT = None
BAUD = 115200
VERBOSE = 0
RUNASDAEMON = False

# dictionaries used to store new and known sensors
NEWBIE_SENSORS = {}
SENSORS = {}

# rrd parametrization
START = int(time.time())
STEP = 10

# === classes =================================================================

# Storage class for meta information of a sensor
class RRDObject(object):
    addr = None
    dbname = None
    dbkeys = None
    def __repr__(self):
        return "RRD(a=%s,l=%s,k=%s)" % (self.addr, self.dbname, self.dbkeys)

# Serial device class
class Device:
    def __init__(self,port, baud):
        self.sport = serial.Serial(port, baud)
        self.sport.close()
        self.sport.open()
        self.rxThread = threading.Thread(target = self._rx_thread_)
        self.rxThread.daemon=True  # if a thread is not a daemon, the program needs to join all
        self.rxThread.start()

    # The ressource consuming computations are done on the PC.
    def scale_parameters(self, params):
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

    # function running in thread
    def _rx_thread_(self):
        while 1:
            try:
                x = self.sport.readline().strip()
                # eliminate 0 bytes ... bug in buffer handling sensorapp.c
                x = x.replace("\0","")
                x = x.strip("\r")
                y = eval("dict(%s)" % x)
                print "DICT:", str(y)
                y["tstamp"] = int(time.time())
                self.scale_parameters(y)
                if VERBOSE > 1:
                    print "DEBUG: SERIAL READ: %s" % ( y )
                if not SENSORS.has_key(y["addr"]):
                    rrd_create(**y)
                rrd_update(**y)
            except:
                print "ERROR:rxthread:%s:%s" % (self.rxThread.name, sys.exc_info())
                traceback.print_exc()


# === functions ===============================================================

# create a RRD object for a sensor
def rrd_create(tstamp, addr, **kwargs):
    global SENSORS
    r = RRDObject()
    r.addr = addr
    r.dbname = "%s/sensor_%04x.rrd" % (WORKDIR, addr)
    r.dbkeys = kwargs.keys()
    r.dbkeys.sort()

    dsrc = []
    for k in r.dbkeys:
        dsrc.append("DS:%s:GAUGE:%d:U:U" % (k.strip(), STEP))

    if not os.path.isfile(r.dbname):
        args = ["--start", str(tstamp-10), "--step", str(STEP)]
        args += dsrc
        args.append("RRA:AVERAGE:0.5:1:300" )
        rrdtool.create(r.dbname, *args)
        if VERBOSE:
            print "create addr=%s rrd_file=%s " % (addr, r.dbname)
            sys.stdout.flush()
    SENSORS[addr] = r


# update the RRDB for of a sensor
def rrd_update(tstamp, addr, **kwargs):
    global SENSORS
    r = SENSORS.get(addr)
    if r != None:
        values = [str(kwargs.get(v,"42")) for v in r.dbkeys]
        arg = str(tstamp)+":"+":".join(values)
        rrdtool.update(r.dbname, arg)

# make this script to a daemon that can be launched from rcinit.
# The code was initially taken from:
# http://de.w3support.net/index.php?db=so&id=588749
def daemonize():
    pid = os.fork()
    if pid > 0:
        sys.exit(0)
    os.chdir("/")
    os.umask(0)
    os.setsid()
    pid = os.fork()	# Fork a second child.
    if pid > 0:
        sys.exit(0)
    for f in [sys.stdin, sys.stdout, sys.stderr]:
        f.flush()
    si = file("/dev/null", "r")
    so = file("%s/sensor.log" % WORKDIR, "a+")
    se = file("%s/sensor_err.log"  % WORKDIR, "a+")
    os.dup2(si.fileno(), sys.stdin.fileno())
    os.dup2(so.fileno(), sys.stdout.fileno())
    os.dup2(se.fileno(), sys.stderr.fileno())
    return 0


#==================================================================
if __name__ == "__main__":
    opts, args = getopt.getopt(sys.argv[1:], "p:b:hvD")
    for o,v in opts:
        if o == "-v":
            VERBOSE += 1
        if o == "-p":
            SPORT = v
        elif o == "-b":
            BAUD = eval(v)
        elif o == "-D":
            RUNASDAEMON = True

    # create working directory if needed
    WORKDIR = os.path.abspath(WORKDIR)
    if not os.path.isdir(WORKDIR):
        os.mkdir(WORKDIR)

    try:
        if RUNASDAEMON:
            daemonize()
    except SystemExit:
        sys.exit(0)

    sdev = Device(SPORT, BAUD)
    while 1:
        time.sleep(4)

