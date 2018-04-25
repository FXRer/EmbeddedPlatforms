#
#   Copyright (c) 2008 Axel Wachtler
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

##
# @file
# @brief Create headerfile dependency description for graphviz
#
# This tool writes a graphviz command file, that represents the
# include dependencies in graphical manner.
#
# Usage:
#		python h2dot.py [-I <incpath>] [-R <maxrec>]  file1.c [file2.c [...]]
#
# Options:
#      -I <incpath>  include search path option (like in C compiler)
#      -R <maxrec>   maximum recursion level of graph
#      file<x>.c     source file(s), which is level 0 of the graph.
#
# Example:
#    python Tools/h2dot.py -I Src/Lib/Inc/   Src/Lib/*/*.c > graph.dot
#
#    python Tools/h2dot.py -I Src/Lib/Inc/   Src/Lib/*/*.c | dot -Tpng | xv -
#     - Give a quick overview of current radio libraries include file layout.
#
# @todo arange graph so, that nodes are displayed at its max. recursion level.
# @todo implement -h and -v
# @todo show in graph that there are missing files due to end of recursion level.
#

# === Imports ==========================================================
import getopt, sys, re, os, glob
from pprint import pprint as pp

def search_incfile(fname):
    global INC_SEARCH
    ret = None
    for i in INC_SEARCH:
        ffn = i+'/'+fname
        if os.path.exists( ffn ):
            ret = ffn
    return ret

def scan_file(fname, parent, lvl):
    global RECURSION_DEPTH, SCANNED_FILES, EDGE_LIST, FILE_LIST

    if lvl >= RECURSION_DEPTH:
        return
    if not os.path.exists(fname):
        return
    bname = os.path.basename(fname)
    if bname not in FILE_LIST:
        #print " "*lvl+"scan_file: "+fname
        FILE_LIST.append(bname)
        SCANNED_FILES[bname] = lvl
        f = open(fname)
        for l in f.xreadlines():
            try:
                incfn = re.match('^[ \t]*#[ ]*include ["<]([a-zA-Z0-9_\./]*)[>"]',l).group(1)
                incf = search_incfile(incfn)
                incbn = os.path.basename(incfn)
                EDGE_LIST.append((bname,incbn))
                if incf:
                    scan_file(incf,bname,lvl + 1)
            except AttributeError:
                pass
        f.close()
    else:
        if SCANNED_FILES[bname] < lvl+1:
            SCANNED_FILES[bname] = lvl+1

if __name__ == "__main__":
    INC_SEARCH = []
    EDGE_LIST = []
    FILE_LIST = []
    SCANNED_FILES = {}
    RECURSION_DEPTH = 10
    opts,cmdargs = getopt.getopt(sys.argv[1:],"I:R:h")

    for o,v in opts:
        if o == "-I":
            p = v.strip()
            if p not in INC_SEARCH:
                INC_SEARCH.append(p)
        if o == "-R":
            RECURSION_DEPTH = eval(v)

    # we added an additional globbing run here
    # for all the windows users
    args = []
    for a in cmdargs:
    	args += glob.glob(a)

    # scan files
    for fn in args:
        scan_file(fn,os.path.basename(fn),0)

    # search max recursion depth
    maxlvl = 0
    for f in SCANNED_FILES.keys():
         l = SCANNED_FILES[f]
         maxlvl = max(l,maxlvl)


    # print graph
    print """digraph h2dotgraph {
        graph [rankdir="LR",size="14,14"];
        //bgcolor="transparent";
        node [shape = "record"];
          """

    for lvl in range(maxlvl+1):
        nodes = []
        for f in  FILE_LIST:
            if SCANNED_FILES[f] == lvl:
                nodes.append(f)
        if len(nodes)>0:
            nstr = "\n        ".join(map(lambda s : '"%s";' % s, nodes))
            print "    subgraph lvl_%d {rank=same;\n        %s\n    }" % (lvl, nstr)

    for s,d in EDGE_LIST:
        print '    "%s"->"%s";' % (s,d)
    print "}"

