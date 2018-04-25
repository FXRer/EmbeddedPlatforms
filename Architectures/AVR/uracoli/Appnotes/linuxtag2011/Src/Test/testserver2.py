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
import SimpleHTTPServer
import serial, threading, time, sys, getopt
import re

# === globals =================================================================
PORT = 8080
SPORT = None
BAUD = 115000


# === functions ===============================================================

##
# Header Function for
def header():
    ret =  """<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">
<html><head>
<script type="text/JavaScript">
<!--
function timedRefresh(timeoutPeriod) {
	setTimeout("location.reload(true);",timeoutPeriod);
}
//   -->
</script>
</head>
<body onload="JavaScript:timedRefresh(1000);">
<h1>Mini Serial Server and Temperature Visualization</h1></a>\n"
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
        self.rxThread.setDaemon(1) # if a thread is not a daemon, the program needs to join all
        self.rxThread.setName("RX_%s" % self.sport.portstr)
        self.rxThread.start()
        self.Fifo = {}
        self.verbose = verbose

    def _rx_thread_(self):
        while 1:
            try:
                x = self.sport.readline().rstrip()
                if x:
                    tvals=x.split('\r')
                    for tv in  tvals:
                        robj=re.match('\s*n:\s+([0-9a-zA-Z]+),\s+t:\s+(\d+)', tv)
                        if robj:
                            station=robj.group(1)
                            temp=robj.group(2)
                            self.Fifo[station]=temp
                            print "found:  TEMP(%s) = %s  " % ( station,  temp )
                        else:
                            print "error: %s" % ( x )
                if False:
                        print x
            except:
                print "ERROR:rxthread:%s:%s" % ("RR", sys.exc_info()[1])

    def write(self,data):
        try:
            self.sport.write(data)
            ret = None
        except:
            print sys.exc_info()
            ret = "NOK: %s" % str(sys.exc_info()[1])
        return ret

    def get_text(self):
        ret = """<div style="font-size:40pt"><table border="0" cellspacing="10" cellpadding="20"><tr>"""
        idex=sorted(self.Fifo.keys())
        for idx in idex:
            ret+= "<th>%s</th>\n" % ( idx )
        ret+= """</tr><tr>\n"""

        for idx in idex:
            ret += "<td>%s</td>\n"    % ( self.Fifo[idx] )
        ret+= "</tr></table></div>\n"

        return ret
##
# Http Server Class
class CustomHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        global sdev

        print "PATH:",self.path
        error = 200
        if self.path=='/':
            txt = "<h2>Temperature Summary</h2>"
            txt += sdev.get_text()
        else:
            error = 404
            txt = "<p>??? unknown url: %s</p>" % self.path
        self.deliver_page(error, txt)

    def deliver_page(self, error, txt):
        self.send_response(error)
        self.send_header('Content-type','text/html')
        self.end_headers()
        self.wfile.write(header())
        self.wfile.write(txt) #call sample function here
        self.wfile.write(footer())

# === init ====================================================================
if __name__ == "__main__":

    opts, args = getopt.getopt(sys.argv[1:], "p:b:h")
    print opts
    for o,v in opts:
        if o == "-p":
            SPORT = v
        elif o == "-b":
            BAUD = eval(v)
        elif o == "-h":
            print __doc__
            sys.exit(0)
    if SPORT == None:
        SPORT = raw_input("Enter Port name:")

    sdev = Device(SPORT, 9600, 1)
    # try to open server on unprivileged port
    while 1:
        try:
            httpd = SocketServer.ThreadingTCPServer(('localhost', PORT),CustomHandler)
            break
        except:
            time.sleep(2)
            print "failed to open port %d" % PORT
            print sys.exc_info()
            PORT ^= 1

    print "serving at port", PORT
    httpd.serve_forever()
