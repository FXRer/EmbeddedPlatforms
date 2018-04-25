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
"""
Wireless UART Test Script
"""
import time
import testenv as T
import sys
import random

class Test_Fw_Wuart(T.Test_Fw_Base):

    def xtest_bootprint(self):
        """Test boot message"""
        self.dut.reset(self.get_rwt(self.dut))
        ln = self.dut.comm.readline()
        self._print(3, "\nln=", ln)
        assert ln.startswith('Wuart')

    def test_chunk_rx(self):
        """single chunk reception"""
        teststr = "Thequickbrownfoxjumpsoverthelazydog"
        self.peer.comm.write(teststr)
        answ=''
        tstart = time.time()
        while time.time() < tstart + self.maxguardtime:
            if self.dut.comm.inWaiting()>0:
                answ += self.dut.comm.read(self.dut.comm.inWaiting())
            if answ == teststr:
                break
        self._print(3, "teststr:", teststr)
        self._print(3, "answer:", answ)
        assert answ == teststr

    def test_chunk_tx(self):
        """single chunk transmission"""
        teststr = "Thequickbrownfoxjumpsoverthelazydog"
        self.dut.comm.write(teststr)
        answ=''
        tstart = time.time()
        while time.time() < tstart + self.maxguardtime:
            if self.peer.comm.inWaiting()>0:
                answ += self.peer.comm.read(self.peer.comm.inWaiting())
            if answ == teststr:
                break
        self._print(3, "teststr:", teststr)
        self._print(3, "answer:", answ)
        assert answ == teststr


    def test_chartimeout(self):
        """measure timeout of single character"""
        testchar = 'A'
        self.dut.comm.write(testchar)
        tstart = time.time()
        duration = None
        inchar = None
        answ=''
        while time.time() < tstart + self.maxguardtime:
                if self.peer.comm.inWaiting()>0:
                    inchar = self.peer.comm.read()
                    assert inchar == testchar
                    duration = time.time() - tstart
                    break
        self._print(3, "duration:", duration,"\n",
                       "inchar:", inchar,"\n",
                       "data:",self.peer.comm.read(self.peer.comm.inWaiting()))
        self.assertNotEqual(duration, None)

        self._print(0, 'duration=%dms' % (duration*1000), end = " ")

    def test_escape_sequence(self):
        self.dut.comm.write("\n")
        time.sleep(1)
        self.dut.comm.write("+++")
        time.sleep(1)
        self.dut.comm.write("q")

        for pattern in ["MENU", "[a]", "[p]",
                        "[P]", "[c]", "[C]", "[r]",
                        "[e]", "[f]", "[q]", "==="]:
            retries = 5
            while retries > 0:
                ln = self.dut.comm.readline().strip()
                self._print(3, ">", ln)
                self._print(3, "peer>", self.peer.comm.readline())
                if len(ln): break
                retries -= 1
            self.assertGreater(retries, 0, "")
            self.assertTrue(ln.startswith(pattern), "Wrong line read: %s - %s" % (pattern, ln))
        ln = self.dut.comm.readline().strip()
        self.assertTrue(ln.startswith("discard"))

    def test_txrx_binary(self):
        for tx, rx in ((self.dut, self.peer),(self.peer, self.dut)):
            for iteration, chunksz in enumerate([5, 20, 125, 200, 250, 3]):
                self._print(3, " iter=%d, tx=%s, rx=%s, chunksz=%d" % \
                            (iteration, tx, rx, chunksz))
                txdata = "".join([chr(random.randrange(256)) for i in xrange(chunksz)])
                #txdata = "".join(["X" for i in xrange(chunksz)])
                rx.comm.read(rx.comm.inWaiting()) # flush input
                tx.comm.write(txdata)
                rxdata = ""
                tstart = time.time()
                while time.time() < tstart + 30:
                    if rx.comm.inWaiting() > 0:
                        rxdata += rx.comm.read(rx.comm.inWaiting())
                        tstart = time.time()
                    if rxdata == txdata:
                        break
                if rxdata != txdata:
                    self._print(3, "iter=%d, len=%d" % (iteration, len(txdata)))
                    self._print(3, "tx", map(hex, map(ord, txdata)))
                    self._print(3, "rx", map(hex, map(ord, rxdata)))

                assert rxdata == txdata, "TX/RX failed, [%d] %s=>%s "\
                                         "txlen=%s, rxlen=%s" % \
                                         (iteration, tx, rx, len(txdata), len(rxdata))


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
        for d in [peer] + duts.values():
            s = T.testsuite_factory(T.Test_Fw_Flash,
                                    "flash_wuart_%s" % d,
                                    dut = d, fw="wuart", params = rv)
            suites.append(s)

    for dn, dut in duts.items():
        s = T.testsuite_factory(Test_Fw_Wuart,
                                "wuart_peer=%s_dut=%s" % (peer, dut),
                                params = rv, dut = dut, peer = peer)
        suites.append(s)

    T.test_suites_run(suites, **rv)
# EOF
