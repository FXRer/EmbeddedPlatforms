#   Copyright (c) 2013, 2014 Daniel Thiele, Axel Wachtler
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
#

from __future__ import print_function
import sys
import os
import serial
import glob
import time
import getopt
import unittest
import xmlrunner
import subprocess
import zipfile

sys.path.append('./Tools')
import hbm251

# === Globals ==================================================================
RUNNER_MAP = {
        "text": unittest.TextTestRunner,
        "xml" : xmlrunner.XMLTestRunner
    }

OPTIONS = {
    "peer"     : ("p", "peer", None, "set the peer device"),
    "duts"     : ("d", "duts", "", "set the dut device or dut list"),
    "boardcfg" : ("B", "boardcfg", "Config/board.cfg", "select board config file."),
    "setupcfg" : ("C", "setupcfg", "regression.cfg", "Regression config file"),
    "runner"   : ("r", "runner", "text", "select test runner ('text', 'xml')"),
    "builddir" : ("b", "builddir", "./build", "select build directory"),
    "bindir"   : ("D", "bindir", "install/bin", "select binary directory"),
    "board"    : ("b", "board", None, "select board type."),
    "noflash"  : ("f", "no-flash", False, "disable flashing")
}

# === Classes ==================================================================
class DevSerial(serial.Serial):
    def __init__(self, **kwargs):
        serial.Serial.__init__(self)
        self.setPort(kwargs['port'])
        self.setBaudrate(kwargs['baudrate'])
        self.setTimeout(1.0)
        #self.open()

    def flush(self):
        self.read(self.inWaiting())

class TestNode(object):
    def __init__(self, comm, board):
        self.comm = comm
        self.board = board

    def reset(self, reset_wait_time = 0):
        self.board.reset()
        time.sleep(reset_wait_time)

    def flash(self, appname, dryrun, verbose):
        fwname = self.board.find_app(appname)
        assert fwname != None, "firmware %s for %s:%s not found." % \
                                (appname, self.board['name'], self.board['board'])
        self.board.flash(fwname, dryrun, verbose)

    def __repr__(self):
        return '%(name)s-%(board)s-%(port)s' % (self.board)

class Test_Base(unittest.TestCase):
    """Base class for all testcases"""
    def _print(self, verbose, *objects, **kwargs):
        if verbose <= self.params['verbose']:
            print(*objects, **kwargs)
        sys.stdout.flush()


class Test_Fw_Base(Test_Base):
    """Class for firmware tests on hardware setup"""
    maxguardtime = 1.0
    reset_wait_time = 0.1

    @classmethod
    def getMyComPorts(self, cls):
        """return a list of all comports of all devices"""
        ports = []
        waittime = 0.0
        if hasattr(cls, 'dut'):
            ports.append(cls.dut.comm)
            waittime = cls.get_rwt(cls.dut)
        if hasattr(cls, 'peer'):
            ports.append(cls.peer.comm)
            # we only need to wait the longest time of both dut,peer
            waittime = max(waittime, cls.get_rwt(cls.peer))
        return ports, waittime

    @classmethod
    def get_rwt(self, dev):
        rv = float(self.dut.board.get("reset_wait_time", str(self.reset_wait_time)))
        return rv

    @classmethod
    def setUpClass(cls):
        ports, maxwait = cls.getMyComPorts(cls)
        [p.open() for p in ports]
        time.sleep(maxwait)
        [p.flush() for p in ports]

    @classmethod
    def tearDownClass(cls):
        ports, maxwait = cls.getMyComPorts(cls)        
        [p.close() for p in ports]

class Test_Unzip_File(Test_Base):
    zfname = None
    unpackdir = None
    def test_unzip(self):
        if not os.path.isdir(self.unpackdir):
            assert not os.makedirs(self.unpackdir)
        zf = zipfile.ZipFile(self.zfname, "r")
        assert not zf.extractall(self.unpackdir)

class Test_Fw_Flash(Test_Base):
    """flash firmware"""
    dut = None
    fw = None
    dryrun = 0
    def test_flash(self):
        verbose = self.params.get("verbose", 0)
        if verbose > 2: print()
        self.dut.flash(self.fw, dryrun = self.dryrun, verbose = verbose-2)

# === Functions ================================================================

##
# Standard command line options for test cases are parsed with this function.
# @param docstr: Documentation of the test
# @return None in case of help - then test shall be skipped
#         otherwise a dictionary with the option values
def parse_test_options(docstr = "", options = []):
    option_doc = """\
Options:
    -h, --help
        show help text
    -v, --verbose
        increase verbose level, each -v increases verbosity by 1.\n"""

    rv = dict(
         BOARD = None,
         verbose = 0
    )

    short_options = "hv"
    long_options = [ "help", "verbose="]
    opt_lookup = {}
    options += ["runner"]
    for o in sorted(options):
        if OPTIONS.has_key(o):
            so,lo,dfl,doc = OPTIONS[o]
            short_options += "%s:" % so
            long_options.append("%s=" % lo)
            rv[o] = dfl
            opt_lookup["-%s" % so] = o
            opt_lookup["--%s" % lo] = o

            option_doc += "    -%s <%s>, --%s=<%s>:\n"\
                          "        %s, default: %s\n" % (so,o,lo,o,doc,dfl)
    option_doc += "Arguments:\n"\
                 "     the arguments that are passed to the test script\n"

    opts, args = getopt.getopt(sys.argv[1:], short_options,long_options)
    do_exit = False
    for o,v in opts:
        if o in ("-h", "--help"):
            print(docstr % {"script": sys.argv[0]} + option_doc)
            do_exit = True
        elif o in ("-v", "--verbose"):
            rv["verbose"] += 1
        elif opt_lookup.has_key(o):
            try:
                rv[opt_lookup[o]] = eval(str(v))
            except:
                rv[opt_lookup[o]] = str(v)
    # let the runner mapping do the run() routine at the moment
    if rv.has_key("duts"):
        rv["duts"] = [d.strip() for d in rv["duts"].split(",") if len(d.strip())]
    rv['ARGS'] = args
    if do_exit:
        rv = None
    return rv

def get_testnodes(setupcfg, boardcfg, bindir):
    """Deliver a dict of TestNode classes from cfg file"""
    hbm = hbm251.HBM251(setupcfg, boardcfg, bindir)
    testnodes = {}
    for b in hbm.boards:
        if b['port'] != '':
            tn = TestNode(DevSerial(**b), b)
            bn = b['name']
            testnodes[bn] = tn
    return testnodes

def testsuite_factory(testclass, testname, **kwargs):
    class TS(testclass):
        pass
    for k,v in kwargs.items():
        setattr(TS, k, v)
    TS.__name__ = str(testname)
    return unittest.TestLoader().loadTestsFromTestCase(TS)

def test_unzip_file(zfname, unpackdir, **kwargs):
    s = testsuite_factory(Test_Unzip_File,
                          "unzip",
                          zfname = zfname, unpackdir = unpackdir)
    test_suites_run([s], **kwargs)


def get_runner(**kwargs):
    runner_cls = RUNNER_MAP.get(kwargs["runner"], unittest.TextTestRunner)
    if runner_cls == xmlrunner.XMLTestRunner:
        runner = runner_cls(verbosity = kwargs["verbose"],
                            output = "test_reports")
    else:
        runner = runner_cls(verbosity = kwargs["verbose"])
    return runner

def test_suites_run(suites, **kwargs):
    runner = get_runner(**kwargs)
    runner.run(unittest.TestSuite(suites))

##
# Zip an artifact file:
# @param zipfn
#               name of the zip file
# @param globlist
#               list of tuples (glob, dirpfx)
#               glob: is a globbing pattern
#               dirpfx: is the prefix each file is prepended in the Zip file.
# Example:
# @code
#   zip_results( "x.zip", [ ("Regression/*.py", "x/test"),
#                           ("Config/board.cfg","x/test"),
#                           (workdir + "/bin/*.hex","x/bin") ])
# @endcode
#
def zip_results(zipfn, globlist = []):
    ziplist = []
    zf = zipfile.ZipFile(zipfn, "w")
    for p,d in globlist:
        xx = glob.glob(p)
        for f in xx:
            dn, fn = os.path.split(f)
            ofn = "%s/%s" % (d,fn)
            zf.write(f, ofn, zipfile.ZIP_DEFLATED)
    zf.close()

##
# Execute a shell command
# @param cmd    command line string that is going to be executed
#
def execute(cmd, verbose = 0, verbose_onerror = 1):
    proc = subprocess.Popen( cmd,
                             shell = True,
                             stdin = subprocess.PIPE,
                             stderr = subprocess.PIPE,
                             stdout = subprocess.PIPE)
    stdout, stderr = proc.communicate()
    if verbose or (verbose_onerror and proc.returncode != 0):
        print("# %s" % cmd)
        if len(stdout):
            print (": stdout :\n: ", stdout.replace("\n", "\n: "))
        if len(stderr):
            print (": stderr :\n: ", stderr.replace("\n", "\n: "))
    return proc.returncode, stdout, stderr

if __name__ == '__main__':
    raise Exception("Do not use this")
# EOF
