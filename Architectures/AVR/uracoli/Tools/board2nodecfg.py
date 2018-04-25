#   Copyright (c) 2009 Axel Wachtler
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
This script generates a json file from the board.cfg files.
It calls avr-gcc to find out the details about flashend and
eepromend.

It dumps the resulting json file to stdout.

Usage:
    python board2nodecfg.py board.cfg
"""
# === import ==================================================================
import ConfigParser
import re, subprocess, sys, os, json

try:
    # convinience for debugging
    import readline, rlcompleter, pprint
    readline.parse_and_bind("tab:complete")
    pp = pprint.pprint
except:
    pass

# === globals =================================================================

# === functions ===============================================================

# === classes =================================================================

# === init ====================================================================

try:
    fname = sys.argv[1]
    if not os.path.exists(fname):
        raise Exception, "Can not find file %s" % fname
except:
    print "=" * 80
    print __doc__
    print "=" * 80
    print "Error:", sys.exc_info()


cfgp = ConfigParser.ConfigParser()
cfgp.read(fname)

# read the board cfg file
ret = dict(boards = {}, mmcus = {}, aliases = {})
for b in cfgp.sections():
    #print cfgp.items(b)
    ret["boards"][b] = dict(cfgp.items(b))

# extract flashend/eepromend with the help of avr-gcc.
for b,v in ret["boards"].items():
    mmcu = v['cpu']
    if not ret["mmcus"].has_key(mmcu):
        cmd = 'avr-gcc -mmcu=%s -dM -E -' % mmcu
        p = subprocess.Popen(cmd,
                             stdin = subprocess.PIPE,
                             stdout = subprocess.PIPE,
                             stderr = subprocess.PIPE,
                             shell = True)
        x = p.communicate("#include <avr/io.h>\n")
        flashend = re.findall("FLASHEND[ \(]*([0-9A-Fa-fxX]*)",x[0])[0]
        eepromend = re.findall("E2END[ \(]*([0-9A-Fa-fxX]*)", x[0])[0]
        ret["mmcus"][mmcu] = dict(flashend = flashend, eepromend = eepromend)

# generate the alias section
for b,v in  ret["boards"].items():
    ret['aliases'][b] = b
    for bb in v.get('aliases','').split():
        ret['aliases'][bb] = b

# write the result file
print json.dumps(ret,indent=2)
