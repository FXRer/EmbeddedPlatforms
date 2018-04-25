#   Copyright (c) 2016 Axel Wachtler
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
"""
Sniffer Test Script
"""

import time
#import unittest
import testenv as T
import sys

class Test_Fw_Sniffer(T.Test_Fw_Base):

    def test_bootprint(self):
        """Test boot message"""
        self.dut.reset(self.get_rwt(self.dut))
        ln = self.dut.comm.readline()
        self._print(3, "\nln=", ln, end="")
        assert ln.startswith('Sniffer')

    def test_parms(self):
        """test parms command"""
        self.dut.comm.write("parms\n")
        lines = []
        for i in range(15):
            time.sleep(.1)
            ln = self.dut.comm.readline().strip()
            lines.append(ln)
            self._print(3, "pos:", len(lines)-1, "read:", ln)
        assert lines[0].startswith("> parms")
        assert lines[1] == ""
        for ln,v in enumerate(["PLATFORM:", "SUPP_CMSK:",
                               "CURR_CMSK:", "CURR_CHAN:",
                               "CURR_PAGE:", "CURR_RATE:",
                               "SUPP_RATES:", "SFD:", "TIMER_SCALE:",
                               "TICK_NUMBER:", "CHKCRC:",
                               "MISSED_FRAMES:"],2):
            assert lines[ln].startswith(v), "line: %d exp: %s read: %s"  % (ln, v, lines[ln])

    def test_sniff_frame(self):
        """receive a frame"""
        # start sniff mode
        self.dut.comm.write("sniff\n")
        rxdata = self.dut.comm.readline().strip()
        assert rxdata.startswith("> sniff")
        rxdata = self.dut.comm.readline().strip()
        assert rxdata.startswith("OK")

        # send a frame from peer
        self.peer.comm.write("s")

        # verify sniffer data
        sof = ord(self.dut.comm.read(1))
        assert  sof == 1, "start of frame: exp 1, read: %d" % ordsof
        flen = ord(self.dut.comm.read(1))
        rxdata = map(ord, self.dut.comm.read(flen))
        self._print(3, "flen :", flen,"rxdata :", map(hex, rxdata))
        eof = ord(self.dut.comm.read(1))
        assert  eof == 4, "end of frame: exp 4, read: %d" % eof

    def test_idle_state(self):
        """application idle state"""
        self.dut.comm.write("idle\n")
        rxdata = self.dut.comm.readline().strip()
        assert rxdata.startswith("> idle")
        rxdata = self.dut.comm.readline().strip()
        assert rxdata.startswith("OK")

        self.dut.comm.write("I")
        rxdata = self.dut.comm.readline().strip()
        assert rxdata.startswith("IDLE")

        self.dut.comm.write(" ")
        rxdata = self.dut.comm.readline().strip()
        assert rxdata.startswith("IDLE")




# === test main ================================================================
if __name__ == '__main__':
    rv = T.parse_test_options(__doc__,
                              ["setupcfg", "boardcfg", "bindir",
                               "duts", "peer", "noflash"])
    if not rv:
        sys.exit(1)

    testnodes = T.get_testnodes(rv["setupcfg"], rv['boardcfg'], rv['bindir'])
    peer = testnodes.get(rv["peer"])
    duts = {dut: testnodes.get(dut) for dut in rv["duts"]}
    suites = []

    if not rv['noflash']:
        s = T.testsuite_factory(T.Test_Fw_Flash,
                                "flash_rdiag_%s" % peer,
                                dut = peer, fw="rdiag", params = rv)
        suites.append(s)
        for d in duts.values():
            s = T.testsuite_factory(T.Test_Fw_Flash,
                                    "flash_sniffer_%s" % d,
                                    dut = d, fw="sniffer", params = rv)
            suites.append(s)

    for dn, dut in duts.items():
        s = T.testsuite_factory(Test_Fw_Sniffer,
                                "Sniffer_peer=%s_dut=%s" % (peer, dut),
                                params = rv, dut = dut, peer = peer)
        suites.append(s)

    T.test_suites_run(suites, **rv)

# EOF


