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
import ConfigParser
import os, sys, time


# === globals =================================================================

# === functions ===============================================================

# make this script to a daemon that can be launched from rcinit.
# The code was initially taken from:
# http://de.w3support.net/index.php?db=so&id=588749
def daemonize(workdir):
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

    fbase = sys.argv[0].replace(".py","")
    si = file("/dev/null", "r")
    so = file("%s/%s.log" % (workdir, fbase), "w")
    se = file("%s/%s_err.log"  % (workdir, fbase), "w")
    os.dup2(si.fileno(), sys.stdin.fileno())
    os.dup2(so.fileno(), sys.stdout.fileno())
    os.dup2(se.fileno(), sys.stderr.fileno())
    return 0

def read_config(fname):
    ret = dict(nodes = {}, workdir=None)
    CFG = cfg = ConfigParser.ConfigParser()
    cfg.readfp(open(fname))
    if cfg.has_section('general'):
        # convert items to a dictionary
        tmp = dict(cfg.items('general'))

        # create working directory if needed
        wd = os.path.abspath(tmp.get('workdir','./data'))
        if not os.path.isdir(wd):
            os.mkdir(wd)

        # fill up config structure
        ret['workdir'] = wd
        ret['rra'] = tmp.get('rra','RRA:AVERAGE:0.5:1:3000').split()
        ret['serialport'] = tmp.get('serialport', None)
        ret['baudrate'] = eval(tmp.get('baudrate', "None"))
        ret['rrdstep'] = eval(tmp.get('rrdstep', 10))
    if cfg.has_section("nodes"):
        # store reverse address to name table
        for k,v in cfg.items("nodes"):
            try:
                v = eval(v)
            except:
                pass
            ret['nodes'][v] = k
    if cfg.has_section("measurements"):
        mnames = ("min", "max", "unit", "title")
        ret['measurements'] = {}
        for k,v in cfg.items("measurements"):
            mvals = [x.strip() for x in v.split(",",4)]
            ret['measurements'][k] = dict(zip(mnames, mvals))
    return ret

def debug(msg):
    sys.stdout.write(time.asctime()+":"+msg+"\n")
    sys.stdout.flush()

def get_html_footer():
    now = time.strftime("Last update: %Y-%m-%d %H:%M:%S", time.localtime())
    ret = '<hr><div>%s</div></body></html>' % now
    return ret

def get_html_header(tmo = None):
    if tmo == None:
        ret =  '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">'\
               '<html>'\
               '<body>'
    else:

        ret =  '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">\n'\
               '<html><head><script type="text/JavaScript">\n'\
               '<!--\n'\
               'function timedRefresh(timeoutPeriod) {\n'\
               '	setTimeout("location.reload(true);",timeoutPeriod);\n'\
               '}\n'\
               '//   -->\n'\
               '</script>'\
               '</head>'\
               '<body onload="JavaScript:timedRefresh(%d);">' % int(tmo)
    return ret

# === classes =================================================================

# === init ====================================================================

