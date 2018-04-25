#   Copyright (c) 2013 - 2016 Axel Wachtler
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
This test unpacks and builds the uracoli package completely.

Usage:
    python %(script)s [options] uracoli-<version>.zip\n\n"""


# === import ==================================================================
import sys
import glob
import os
import testenv as T
import zipfile, unittest
# === globals =================================================================

# === functions ===============================================================

# === classes =================================================================
class Test_Compile(T.Test_Base):
    """compile"""
    MAKEFILE = None
    TARGET = "all"
    BUILDDIR = None
    def test_compile(self):
        pn, fn = os.path.split(self.MAKEFILE)
        cmd = "make -C %s/%s -f %s %s" % (self.BUILDDIR, pn, fn, self.TARGET)
        rc, stdout, stderr = T.execute(cmd)
        self.assertEqual(rc, 0)

# === main =====================================================================
def load_tests(loader, tests, pattern):
    global arcfile, builddir
    suite = unittest.TestSuite()
    with zipfile.ZipFile(arcfile) as z:
        mfl = [f for f in z.namelist() if f.endswith(".mk") or f.find("Makefile")>0]
        targets = {}
        for m in mfl:
            targets[m] = [l.split() for l in  z.read(m).split("\n") if l.startswith("all:")][0][1:]
    for k,v in targets.items():
        for t in v:
            class T(Test_Compile):
                MAKEFILE = k
                TARGET = t
                BUILDDIR = builddir
            T.__name__ = "compile_%s_%s" % ("_".join(k.split(os.sep)[-2:]), t)
            suite.addTest(loader.loadTestsFromTestCase(T))
    return suite

if __name__ == "__main__":
    rv = T.parse_test_options(__doc__, ["builddir"])
    if not rv:
        sys.exit(1)
    arcfile = rv['ARGS'].pop(0)
    builddir = rv['builddir']

    T.test_unzip_file(arcfile, builddir, **rv)
    runner = T.get_runner(**rv)
    T.unittest.main(testRunner = runner, argv = [sys.argv[0]] + rv['ARGS'])
