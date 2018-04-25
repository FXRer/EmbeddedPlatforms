#   Copyright (c) 2015 Axel Wachtler
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
import sys, os
import pprint
pp = pprint.pprint

# === globals =================================================================
INC_DIRS = {}
INC_FILES = {}
SRC_FILES = {}
WORK_DIR = "."
PROJECT_DIR = "."
PROJECT_FILE = None

# === functions ===============================================================
def parse_compiler_command(l):
    global INC_DIRS, SRC_FILES, WORK_DIR
    args = l.split()
    while len(args):
        arg = args.pop(0)
        if arg.startswith("-I"):
            arg = arg.replace("-I", "")
            if len(arg) < 1:
                arg = args.pop(0)
            i =  os.path.normpath(WORK_DIR + "/" + arg)
            INC_DIRS[i] = 1
        elif arg.endswith(".c"):
            s = os.path.normpath(WORK_DIR + "/" + arg)
            SRC_FILES[s] = 1

def locate_inc_file(incfile):
    rv = None
    for i in INC_DIRS.keys():
        ifn = i + "/" + incfile
        if os.path.isfile(ifn):
            rv = ifn
            break
    return rv

def find_includes_recursiv(sf, maxlevel = 0):
    global INC_FILES
    if maxlevel > 10:
        print "nesting to deep"
        return
    f = open(sf)
    for l in f.xreadlines():
        if l.find("#include") > -1:
            incfile = l.split()[1][1:-1]
            ifn = locate_inc_file(incfile)
            if ifn != None:
                do_parse = INC_FILES.get(ifn)
                INC_FILES[ifn] = 1
                if do_parse:
                    find_includes_recursiv(ifn, maxlevel + 1)


# === init ====================================================================
if __name__ == "__main__":
    for l in sys.stdin.readlines():
        if l.find("Entering") > -1:
            # we'll cut first and last quotation marks
            d = os.path.normpath(l.split()[-1][1:-1])
            if PROJECT_DIR == None:
                PROJECT_DIR = d
            WORK_DIR = d
        elif l.find("-c") > 0:
                parse_compiler_command(l)
        elif l.find(".hex") > -1:
            fn = [f for f in l.split() if f.endswith(".hex")][0]
            PROJECT = os.path.basename(fn).replace(".hex", "")
            PROJECT_FILE = PROJECT + ".cproj"
            BOARD = PROJECT.split("_")[-1]
    for sf in SRC_FILES.keys():
        find_includes_recursiv(sf)
    print PROJECT_DIR
    print PROJECT_FILE
    print pp(sorted(SRC_FILES.keys()))
    print pp(sorted(INC_FILES.keys()))

    for incfile in sorted(INC_FILES.keys()) + sorted(SRC_FILES.keys()):
        print os.path.relpath(incfile, PROJECT_DIR)

    FMT_SRCFILE = """\
    <Compile Include="%s">
      <SubType>compile</SubType>
    </Compile>
    """

    content = {
        "SRCFILES": "\n".join([FMT_SRCFILE % f  for f in sorted(SRC_FILES.keys())]),
        "INCFILES": "\n".join([FMT_SRCFILE % f  for f in sorted(INC_FILES.keys())]),
        "MCU" : "foo",
        "PROJECT" : PROJECT,
        "BOARD": BOARD,
        "MAKEFILE": "Makefile"
    }

    out = open("Templates/cproj").read() % content

    fo = open(PROJECT_FILE, "w")
    fo.write(out)
    fo.close()
    print "Wrote: %s" % fo.name
