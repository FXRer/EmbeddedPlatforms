#   Copyright (c) 2010, 2011 Axel Wachtler, Matthias Vorwerk
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
Serves a local RRD DBt on http port 8081

Usage:
    python tableserver.py [OPTIONS]

    Options:
     -h : print help and exit
     -v : be verbose
     -P <port> : port number to serve at.
     -D : run as daemon
     -C : name of config file (default sensor.cfg)
"""

# === import ==================================================================

# global modules
import getopt
import glob
import os
import pprint
import re
import rrdtool
import serial
import SimpleHTTPServer
import SocketServer
import sys
import threading
import time

# local modules
import utils


# === globals =================================================================
PORT = 8081
RUNASDAEMON = False
CFGFILE = "sensor.cfg"
CFG = {'nodes':{}}
VERBOSE = 0

# === functions ===============================================================

##
# Header Function for
def header(tmo = None):
    ret = utils.get_html_header(tmo)
    ret += "<h2>Mini RRD to HTTP Server</h2><hr/><br/>"
    return ret

def footer():
    return utils.get_html_footer()

# build a HTML sensor data table from sensor_data data
def build_sensor_html_table():
    ret = """
    <div>
    <table border="1" cellspacing="5" cellpadding="5">
    <tr>
    <th>Sensor</th>
    """

    snames = CFG['measurements'].keys()
    snames.sort()
    for sname in snames:
        if not CFG['measurements'].has_key(sname):
            CFG['measurements'][sname] = dict(min="U",max="U", unit="?", title = sname)
        ret += "<th>%(title)s<br />/%(unit)s</th>" % CFG['measurements'][sname]
    ret+= "</tr>"

    x = glob.glob("%s/sensor_*.rrd" % CFG['workdir'])
    x.sort()
    for fname in x:
        sensname=(os.path.basename(fname).rsplit(".")[0]).replace("sensor_", "")
        print fname
        # try to get label from node.
        sensaddr = eval("0x%s" % sensname)
        tmp = CFG['nodes'].get(sensaddr)
        if tmp != None:
            sensname = "%04x : %-8s" % (sensaddr, tmp)
        rdlast=rrdtool.last(fname)
        ret+= "<tr><td>%s</td>" % ( sensname )
        if time.time() - rdlast > 15:
            for dsi in snames:
                ret += "<td align='center'><font color=\"#FF0000\">error</font></td>"
        else:
            rinfo = rrdtool.info(fname)
            fn = rinfo["filename"]
            for dsi in snames:
                ik="ds[%s].last_ds" % ( dsi )
                if rinfo.has_key(ik):
                    ret += "<td align='center'>%.3f</td>" % ( float(rinfo[ik]) )
                else:
                    ret += "<td align='center'>n/a</td>"
        ret += "</tr>"
    ret+= "</table></div>\n"
    return ret

# Http Server Class
class CustomHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.requestline.find("reload") > 0:
            reload_tmo = 5000
        else:
            reload_tmo = None
        txt = header(reload_tmo)
        txt += build_sensor_html_table()
        txt += footer()
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(txt)


# === init ====================================================================

if __name__ == "__main__":

    opts, args = getopt.getopt(sys.argv[1:], "hvDP:C:")
    for o,v in opts:
        if o == "-h":
            print __doc__
            sys.exit(0)
        elif o == "-v":
            VERBOSE += 1
        elif o == "-D":
            RUNASDAEMON = True
        elif o == "-P":
            PORT = eval(v)
        elif o == "-C":
            CFGFILE = v

    # read config file
    CFGFILE = os.path.abspath(CFGFILE)
    if not os.path.isfile(CFGFILE):
        print "error: config file %s does not exist" % ( CFGFILE )
        sys.exit(1)
    CFG = utils.read_config(CFGFILE)

    # check if working directory exists
    if not os.path.isdir(CFG['workdir']):
        utils.debug("error: working directory %s does not exist" % ( CFG['workdir'] ))
        sys.exit(1)

    try:
        if RUNASDAEMON:
            utils.daemonize(CFG['workdir'])
    except SystemExit:
        sys.exit(0)

    # try to open server on unprivileged port
    while 1:
        try:
            # allow re-ue port after CTRL-C
            SocketServer.TCPServer.allow_reuse_address = True
            httpd = SocketServer.ThreadingTCPServer(('', PORT),CustomHandler)
            break
        except:
            time.sleep(5)
            print "failed to open port %d" % PORT
            print sys.exc_info()

    print "serving at port", PORT
    httpd.serve_forever()
