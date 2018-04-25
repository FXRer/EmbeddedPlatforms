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
     -p : port name, e.g. COM1 or /dev/ttyS0
     -b : baud rate

"""

# === import ==================================================================
import SocketServer
SocketServer.TCPServer.allow_reuse_address = True  # allow re-ue port after CTRL-C
import SimpleHTTPServer
import serial, threading, time, sys, getopt
import re
import time

# === globals =================================================================
PORT = 8080
SPORT = None
BAUD = 115200

GLOBDSLIST=[ "vmcu", "sht_t", "sht_rh" ]


# === functions ===============================================================

##
# Header Function for
def header():
    ret =  """
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">
<html><head>
<script type="text/JavaScript">
<!--
function timedRefresh(timeoutPeriod) {
	setTimeout("location.reload(true);",timeoutPeriod);
}
//   -->
</script>
</head>
<body onload="JavaScript:timedRefresh(2000);">
<h1>Mini Serial Server and Temperature Visualization</h1></a>\n
<hr>"""
    return ret

def footer():
    ret = "<hr></body></html>"
    return ret


# === classes =================================================================


##
# Serial Device.
class Device:
    def __init__(self,port, baud, verbose=0):
        self.sport = serial.Serial(port, baud)
        self.rxThread = threading.Thread(target = self._rx_thread_)
        self.rxThread.daemon=True  # if a thread is not a daemon, the program needs to join all
        self.rxThread.name="RX,%s" % ( self.sport.portstr )
        self.rxThread.start()
        self.sensor_data = {}
        self.verbose = verbose

    # function running in thread
    def _rx_thread_(self):
        while 1:
            try:
                x = self.sport.readline().strip()
                x = x.replace("\0","")
                x = x.strip("\r")
                if self.verbose:
                    print "DEBUG: SERIAL READ: %s" % ( repr(x) )
                y = eval("dict(%s)" % x)
                if y:
                    if self.verbose:
                        print "DICT:", str(y)
                    node=y["addr"]
                    self.sensor_data[node]=[time.time(), y]
                else:
                    print "WARNING:rxthread:no dict: %s" % ( x )
            except:
                print "ERROR:rxthread:%s:%s" % (self.rxThread.name, sys.exc_info()[1])

    # build a HTML sensor data table from sensor_data data
    def build_sensor_html_table(self):
        ret = """
        <h2>Sensor Data Summary</h2>
        <div style="font-size:40pt">
        <table border="0" cellspacing="10" cellpadding="20">
        """

        # create a list of measures as table header
        ret += "<tr><th>Sensor</th>"
        for i in GLOBDSLIST:
            ret += "<th>%s</th>" % ( i )
        ret += "</tr>\n"

        # create a list of sensor IDs as table headers
        idex=sorted(self.sensor_data.keys())

        # 2nd table row contains the data (temp values)
        for idx in idex:
            ret += "<tr>\n"
            (tim,dat)=self.sensor_data[idx]
            ret += "<td>%4x</td>" % ( idx )

            # values outdated?
            outdated=False
            if (int(time.time()) - tim) > 10:
                outdated=True

            for ik in GLOBDSLIST:
                if  outdated:
                    ret += "<td><font color=\"#FF0000\">N/A</font></td>\n"  # mark red when no update
                else:
                    ret += "<td>%s</td>\n" % ( dat[ik] )
        ret+= "</tr>"

        ret += "</table></div>\n"
        return ret

# Http Server Class
class CustomHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        global sdev
        self.send_response(200)
        self.send_header('Content-type','text/html')
        self.end_headers()
        txt = sdev.build_sensor_html_table()
        self.wfile.write(header() + txt + footer())

# === init ====================================================================
if __name__ == "__main__":

    verbose=0

    opts, args = getopt.getopt(sys.argv[1:], "p:b:hv")
    print opts
    for o,v in opts:
        if o == "-p":
            SPORT = v
        elif o == "-b":
            BAUD = eval(v)
        elif o == "-v":
            verbose = 1
        elif o == "-h":
            print __doc__
            sys.exit(0)
    if SPORT == None:
        SPORT = raw_input("Enter Port name:")

    sdev = Device(SPORT, BAUD, verbose)
    # try to open server on unprivileged port
    while 1:
        try:
            httpd = SocketServer.ThreadingTCPServer(('localhost', PORT),CustomHandler)
            break
        except:
            time.sleep(5)
            print "failed to open port %d" % PORT
            print sys.exc_info()
            # PORT ^= 1

    print "serving at port", PORT
    httpd.serve_forever()
