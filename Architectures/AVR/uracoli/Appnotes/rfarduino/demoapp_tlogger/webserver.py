#   Copyright (c) 2010 Axel Wachtler
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
Serves a local COM port on http port 8080 or 8081

Usage:
    python testserver.py [OPTIONS]

    Options:
     -h : print help and exit
     -v : print version and exit
     -C : config file
"""

# === import ==================================================================
import SocketServer
import SimpleHTTPServer
import serial, threading, time, sys, getopt, os, traceback
import re
from ConfigParser import RawConfigParser

# === globals =================================================================
PORT = 8081
SPORT = None
BAUD = 38400
CFGFILE = "webserver.cfg"
CFGDATA = RawConfigParser()
VERSION = "1.0"

# === classes =================================================================

##
# Serial Device.
class Device:
    def __init__(self,port, baud, verbose=0):
        self.sport = serial.Serial(port, baud)
        self.rxThread = threading.Thread(target = self._rx_thread_)
        self.rxThread.setDaemon(1) # if a thread is not a daemon, the program needs to join all
        self.rxThread.setName("RX_%s" % self.sport.portstr)
        self.rxThread.start()
        self.Fifo = {}
        self.verbose = verbose

    def _rx_thread_(self):
        while 1:
            try:
                line = self.sport.readline().strip()
                if len(line)>10:
                    if "\0" in line:
                        line = line.replace("\0","")
                    curr_time = time.time()
                    x = eval("dict(%s)" % line)
                    x['time'] = curr_time
                    x['strtime'] = time.strftime("%Y%m%d - %H:%M:%S", time.localtime(curr_time))
                    station = x.get("ADDR", 0xffff)
                    if self.Fifo.has_key(station):
                        self.Fifo[station].update(x)
                    else:
                        self.Fifo[station] = x
                        try:
                            self.Fifo[station]["label"] = CFGDATA.get("labels", "0x%04x" % station)
                        except:
                            self.Fifo[station]["label"] = "-"
            except SyntaxError:
                print >>sys.stderr, "error: can not parse line\n"\
                                    "       [%s]" % line
            except:
                print >>sys.stderr, "error: in rxthread, %s\n"\
                                    "       rxdata:[%s]"\
                                    % (str(sys.exc_info()[1]), line)

    def write(self,data):
        try:
            self.sport.write(data)
            ret = None
        except:
            ret = "NOK: %s" % str(sys.exc_info()[1])
        return ret

    def get_text(self):
        ret = CFGDATA.get("settings", "table_header")
        for i in sorted(self.Fifo.keys()):
            try:
                if (time.time() -  self.Fifo[i]['time']) > 10:
                    ret += CFGDATA.get("settings", "table_row_tmo") % self.Fifo[i]
                else:
                    ret += CFGDATA.get("settings", "table_row_std") % self.Fifo[i]
            except:
                ret += CFGDATA.get("settings", "table_row_err") % str(self.Fifo[i])
                print >>sys.stderr, sys.exc_info()
        ret += CFGDATA.get("settings","table_footer")
        return ret
##
# Http Server Class
class CustomHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):

    def do_GET(self):
        global sdev
        error = 200
        if self.path=='/':
            txt = sdev.get_text()
        else:
            error = 404
            txt = CFGDATA.get("settings", "html_error_404") % self.path
        self.deliver_page(error, txt)

    def deliver_page(self, error, txt):
        self.send_response(error)
        self.send_header('Content-type','text/html')
        self.end_headers()
        self.wfile.write(CFGDATA.get("settings", "html_header"))
        self.wfile.write(txt)
        self.wfile.write(CFGDATA.get("settings", "html_footer"))

# === init ====================================================================
if __name__ == "__main__":

    opts, args = getopt.getopt(sys.argv[1:], "hvC:")
    do_exit = False
    for o,v in opts:
        if o == "-h":
            print >>sys.stderr, __doc__
            do_exit = True
        elif o == "-v":
            print >>sys.stderr, "webserver.py Version", VERSION
            do_exit = True
        elif o == "-C":
            CFGFILE = v

    if do_exit:
        sys.exit(0)
    # read config file
    if not os.path.exists(CFGFILE):
        print >>sys.stderr,\
        "error: can not open config file %s\n"\
        "       Create this file in the current directory\n"\
        "       or specify one with the -C option.\n"\
        % CFGFILE
        sys.exit(1)

    print >>sys.stderr, "reading config file", CFGFILE
    CFGDATA.read(CFGFILE)

    print >>sys.stderr, "processing config data"

    PORT = CFGDATA.getint("settings", "http_port")
    PORT_MAX = CFGDATA.getint("settings","http_port_max")
    SPORT = CFGDATA.get("settings","serial_port")
    BAUD = CFGDATA.getint("settings","baudrate")

    print >>sys.stderr, "open serial port %s:%d" % (SPORT, BAUD)
    sdev = Device(SPORT, BAUD, 1)

    success = False
    for x in range(20):
        try:
            httpd = SocketServer.ThreadingTCPServer(('localhost', PORT),CustomHandler)
            success = True
            break
        except:
            print >>sys.stderr, " - failed to open port %d" % PORT
            PORT += 1
            if PORT > PORT_MAX:
                PORT = CFGDATA.getint("settings", "http_port")
            time.sleep(.5)
    if success:
        print >> sys.stderr, "HTTP server running on port %d" % PORT
        httpd.serve_forever()
    else:
        print >> sys.stderr, "could not start HTTP server - giving up and exit"
