#   Copyright (c) 2013 Axel Wachtler
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
This script unpacks and completely builds the arduino 1.5.x package

Usage:
    python test_arduino_pkg.py [options] uracoli-arduino-<version>.zip [ARDUINO_EXE]
"""

# === import ==================================================================
import sys, zipfile, os, subprocess, glob, getopt, testenv, unittest
from utilities import execute, error, find_files_recursive

# === globals =================================================================
BUILDDIR = "build/test/arduino"
ARDUINO_EXE = "arduino"

# === classes =================================================================
class Test_Arduino_Bootloader(testenv.Test_Base):
    workdir = None

    def test_build_bootloader(self):
        rc, so, se = execute("make -C %s/hardware/uracoli/avr/bootloaders/radiofaro/ clean all" % self.workdir)
        if rc != 0:
            self._print(2, "Build Arduino Bootloader failed")
            self._print(2, "cmd: %s" % cmd)
            self._print(2, "rc = %d" % rc)
            self._print(2, "stderr: %s" % stderr.replace("\n", "\nstderr: "))
            self._print(2, "stdout: %s" % stdout.replace("\n", "\nstdout: "))
        return rc, so, se

class Test_Sketch_Build(testenv.Test_Base):
    sketch = None
    variant = None
    arduino_exe = None
    workdir = None
    def test_sketch(self):
        #print "Sketch:", self.sketch, "Variant:", self.variant
        rc, so, se = self.build_sketch(self.sketch, self.variant)
        assert not rc, "Sketch build fail"

    def build_sketch(self, sketch, variant):
        #self._print(2, sketch, variant)
        sbase = os.path.splitext(os.path.split(sketch)[1])[0]
        vbase = variant.split(":")[-1]
        cmd = "%s "\
            "--pref sketchbook.path=%s "\
            "--pref build.path=%s/build/%s_%s "\
            "--board %s "\
            "--verify "\
            "%s"

        cmd = cmd % (self.arduino_exe,
                     self.workdir,
                     self.workdir, sbase, vbase,
                     self.variant, self.sketch)

        #self._print(2, "***", cmd)
        rc, so, se = execute(cmd)
        if rc != 0:
            self._print(2, "Build Arduino Sketch failed")
            self._print(2, "cmd: %s" % cmd)
            self._print(2, "rc = %d" % rc)
            self._print(2, "stderr: %s" % se.replace("\n", "\nstderr: "))
            self._print(2, "stdout: %s" % so.replace("\n", "\nstdout: "))
        return rc, so, se

def get_variants(workdir):
    for v in glob.glob(workdir + "/hardware/uracoli/avr/variants/*"):
        vbase = v.split("/")[-1]
        variant = "uracoli:avr:" + vbase
        if rv["BOARD"] != None:
            if rv["BOARD"] in variant:
                yield variant
        else:
            yield variant

def get_sketches(workdir):
    for s in find_files_recursive(workdir + "/hardware/uracoli/avr/libraries/HardwareRadio/examples/","*.ino"):
        #if (s.find("Gateway") < 0 and s.find("LedChat") < 0):
        yield s

# === init ====================================================================
BUILDDIR = "."

if __name__ == "__main__":

    rv = testenv.parse_test_options(__doc__, ["builddir"])
    BUILDDIR = rv["builddir"]
    if not rv:
        sys.exit(1)

    arcfile = rv["ARGS"].pop(0)
    try:
        arduino_exe = rv["ARGS"].pop(0)
    except IndexError:
        arduino_exe = "arduino"

    workdir = BUILDDIR + "/" +os.path.splitext(os.path.basename(arcfile))[0]
    if not os.path.isdir(workdir):
        os.makedirs(workdir)

    zf = zipfile.ZipFile(arcfile,"r")
    assert not zf.extractall(workdir)

    suites = []
    class TS(Test_Arduino_Bootloader):
        workdir = workdir
        pass
    TS.__name__ = "Test_Arduino_Bootloader_x"
    s = unittest.TestLoader().loadTestsFromTestCase(TS)
    suites.append(s)
    # iterate over variants and sketches
    for variant in get_variants(workdir):
        for sketch in get_sketches(workdir):
            class TS(Test_Sketch_Build):
                arduino_exe = arduino_exe
                params = rv
                workdir = workdir
                variant = variant
                sketch = sketch
                pass
            TS.__name__ = "%s:%s" % (os.path.basename(sketch),variant)
            s = unittest.TestLoader().loadTestsFromTestCase(TS)
            suites.append(s)
    rv["runner"](verbosity=2).run(unittest.TestSuite(suites))
