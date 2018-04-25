#   Copyright (c) 2010,2011 Axel Wachtler, Matthias Vorwerk
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
Serves on local http port 8080

Usage:
    python sensorserver.py [OPTIONS]

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
import re
import rrdtool
import SimpleHTTPServer
import SocketServer
import sys
import time
import os.path
# local modules
import utils


# === globals =================================================================
PORT = 8080
WORKDIR="data"
VERBOSE = 0
RUNASDAEMON = False
CFGFILE = "sensor.cfg"
nodemap = {
#    "0003" : "Bastelkeller",
#    "0004" : "Bastelzimmer",
#    "0008" : "Bastelbad",
#    "0009" : "Bastelgarage",
#    "babe" : "Bastelflur",
#    "beef" : "Bastelschrank",
    "0005" : "wohnzimmer",
    "0008" : "diele",
    "0009" : "keller",
}

dunits={
    #name  : [ min, max, scale, unit, title ]
    #~ "vmcu" :  [ 1.0, 4.0, 1, "V", "Sensor_Supply" ],
    "sht_rh" :[ "U", "U", 1, "%RH", "Rel._Humidity" ],
    "sht_t" : [ "U", "U", 1, "degC", "Temperature" ],
    #~ "cnt" :   [ "U", "U", 1, "_", "Count" ],
    "acc" : [ "U", "U", 1, "g", "Accel" ],
    #~ "acc_y" : [ "U", "U", 1, "g", "Accel_Y" ],
    #~ "acc_z" : [ "U", "U", 1, "g", "Accel_Z" ],
}

listvalues=[ "vmcu", "sht_t", "sht_rh"]

LASTUPDATE = 0
RRDFILES = {}

# === functions ===============================================================

## Header Function
def header(tmo = None):
    ret = utils.get_html_header(tmo)
    ret += "<h1>Sensor RRD to HTTP Server</h1><hr/><br/>"
    return ret

def footer():
    return utils.get_html_footer()

def content_update():
    global LASTUPDATE
    global RRDFILES
    global HTML
    now = time.time()
    if now - LASTUPDATE > 10:
        LASTUPDATE = now
        imglist = ""
        x = glob.glob("%s/sensor_*.rrd" % WORKDIR)
        x.sort()
        for fname in x:
            rinfo = rrdtool.info(fname)
            fn = rinfo["filename"]
            if not RRDFILES.has_key(fn):
                RRDFILES[fn] = get_data_sources(rinfo)

            # map sensor name
            sensname=(os.path.basename(fn).rsplit(".")[0]).replace("sensor_", "")
            sensaddr = eval("0x%s" % sensname)
            tmp = CFG['nodes'].get(sensaddr)
            if tmp != None:
                sensname = "%04x : %-8s" % (sensaddr, tmp)
            imglist += "<p/><big>%s: </big>" % sensname
            img_cnt = 0

            # first list some actual numbers
            rdlast=rrdtool.last(fn)
            if time.time() - rdlast > 20:
                imglist += "<b><font color=\"#FF0000\">(no update)</font></b><br/>"
            else:
                rdinfo = rrdtool.info(fn)
                if rdinfo:
                    imglist += "<b>("
                    for i in listvalues:
                        ik="ds[%s].last_ds" % ( i )
                        if rdinfo.has_key(ik):
                            if dunits.has_key(i):
                                pstr="%s=%.2f%s " % ( i, float(rdinfo[ik]), dunits[i][3] )
                                imglist +=  pstr
                    imglist += ")</b><br/>"
            for ds in RRDFILES[fn]:
                # only plot measures listed in dunits
                if  dunits.has_key(ds):
                    vmin, vmax, vscale, vlabel, vtitle = dunits.get(ds, ("U", "U", 1, "?", "??"))
                    param = dict(rrd=fname,
                                 pic = fname.replace(".rrd","_%s.png" % ds),
                                 ds=ds, vlabel=vlabel, vscale=vscale,
                                 vmin=vmin, vmax=vmax, vtitle=vtitle)
                    sargs = "%(pic)s -a PNG -w 180 --start N-3000 --end N "\
                        "DEF:%(ds)s=%(rrd)s:%(ds)s:AVERAGE "\
                        "CDEF:%(ds)s_sc=%(ds)s,%(vscale)s,* "\
                        "LINE2:%(ds)s_sc#ff0000:%(ds)s "\
                        "--title %(vtitle)s --vertical-label %(vlabel)s --upper-limit %(vmax)s --lower-limit %(vmin)s "\
                        % param
                    rrdtool.graph(sargs.split())
                    imglist += "<img src='%s' style='border:3'/>" % os.path.basename(param["pic"])
                    img_cnt += 1
                    if img_cnt == 3:
                        imglist += "<br/>"
                        img_cnt = 0
            # check if we can plot the accel vales
            if "acc_x" in RRDFILES[fn]:
                # lets assume we also have _y and _z available
                if  dunits.has_key("acc"):
                    vmin, vmax, vscale, vlabel, vtitle = dunits.get("acc", ("U", "U", 1, "?", "??"))
                    param = dict(rrd=fname,
                                 pic = fname.replace(".rrd","_%s.png" % ds),
                                 acc_x="acc_x", acc_y="acc_y", acc_z="acc_z", vlabel=vlabel, vscale=vscale,
                                 vmin=vmin, vmax=vmax, vtitle=vtitle)
                    sargs = "%(pic)s -a PNG -w 180 --start N-3000 --end N "\
                        "DEF:%(acc_x)s=%(rrd)s:%(acc_x)s:AVERAGE "\
                        "DEF:%(acc_y)s=%(rrd)s:%(acc_y)s:AVERAGE "\
                        "DEF:%(acc_z)s=%(rrd)s:%(acc_z)s:AVERAGE "\
                        "LINE2:%(acc_x)s#ff0000:%(acc_x)s "\
                        "LINE2:%(acc_y)s#00ff00:%(acc_y)s "\
                        "LINE2:%(acc_z)s#0000ff:%(acc_z)s "\
                        "--title %(vtitle)s --vertical-label %(vlabel)s --upper-limit %(vmax)s --lower-limit %(vmin)s "\
                        % param
                    rrdtool.graph(sargs.split())
                    imglist += "<img src='%s' style='border:3'/>" % os.path.basename(param["pic"])
                    img_cnt += 1
                    if img_cnt == 3:
                        imglist += "<br/>"
                        img_cnt = 0

        currtime = time.strftime("%Y-%m-%d %H:%M:%S",time.localtime())
        HTML = imglist


def get_data_sources(rinfo):
    ret = []
    rkeys = rinfo.keys()
    for k in rkeys:
        if k.find(".type")>=0:
            ds = re.findall("\[(.*)\]",k)[0]
            print k, rinfo[k], ds
            ret.append(ds)
    ret.sort()
    return ret

# === classes =================================================================

# Http Server Class
class CustomHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        global HTML
        content_update()
        if self.path.endswith(".png"):
            cont_type = 'img/png'
            txt = file("%s/%s" % (WORKDIR, self.path) ).read()
        else:
            cont_type = 'text/html'
            reload_tmo = None
            if self.requestline.find("reload") > 0:
                reload_tmo = 5000
            txt = header(reload_tmo)
            txt += HTML
            txt += footer()
        self.send_response(200)
        self.send_header('Content-type', cont_type)
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
            VERBOSE = 1
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
    WORKDIR = os.path.abspath(CFG['workdir'])
    if not os.path.isdir(WORKDIR):
        print "error: working directory %s does not exist" % ( WORKDIR )
        sys.exit(1)


    try:
        if RUNASDAEMON:
            utils.daemonize(WORKDIR)
    except SystemExit:
        sys.exit(0)



    # try to open server on unprivileged port
    try:
        # allow re-ue port after CTRL-C
        SocketServer.TCPServer.allow_reuse_address = True
        httpd = SocketServer.ThreadingTCPServer(('', PORT),CustomHandler)
    except:
        time.sleep(2)
        print "failed to open port %d" % PORT
        print sys.exc_info()
        sys.exit(1)

    print "serving at port", PORT
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print "shutting down server"
        httpd.socket.close()
