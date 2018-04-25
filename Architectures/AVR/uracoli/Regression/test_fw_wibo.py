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
Firmware Test for WiBo
"""
import sys
import time
sys.path.append('./Src/App/WiBo/PyHost')
import wibohost
sys.path.append('./install/scripts')
import nodeaddr
import testenv as T

class Test_Fw_Wibo(T.Test_Fw_Base):

    def setUp(self):
        self.peer.comm.write("echo 12345\n")
        ln = self.peer.comm.readline().strip()
        assert ln == "OK 12345", \
               "WiBo Host does not respond: exp='OK 12345', rxd=%s" % ln

    def test_ping(self):
        """ping command"""
        assert self.ping_check()

    def test_jbootl(self):
        """jump to bootloader command"""
        saddr = self.dut.board['saddr']
        self.peer.comm.write("exit ffff\n")
        ln = self.peer.comm.readline()
        self.peer.comm.write("jbootl %04x\n" % saddr)
        ln = self.peer.comm.readline()

        assert self.ping_check(expapp = "wibo")

    def test_exit(self):
        """exit from bootloader command"""
        saddr = self.dut.board['saddr']
        self.peer.comm.write("jbootl ffff\n")
        ln = self.peer.comm.readline()
        self.peer.comm.write("exit %04x\n" % saddr)
        ln = self.peer.comm.readline()
        # wait 500ms (for psk230 ????)
        time.sleep(.5)

        #verify with ping if node runs app "wibo"
        assert self.ping_check(expapp = "wibotest")

    def test_timeout(self):
        """Measure timeout of bootloader"""
        d=self.dut
        d.reset()
        tstart=time.time()
        tmax=10 # seconds
        tmout=0
        while time.time() < tstart+tmax:
            if d.comm.readline() != '':
                tmout=time.time()-tstart
                self._print(1, "Timeout", tmout, "seconds")
                break
        self.assertGreater(tmout, 0)

    # flashing has to done last
    def test_zzz_programm_flash(self):
        """feed hex command"""
        saddr = self.dut.board['saddr']
        f = open(self.dut.board.find_app("wuart"))

        assert self.command_check('jbootl %04x\n' % saddr)

        assert self.ping_check(expapp = "wibo")

        assert self.command_check('reset %04x\n' % saddr)

        for l, hexln in enumerate(f):

            self.peer.comm.write('feedhex %04x %s\n' % (saddr, hexln.strip()))
            ln = self.peer.comm.readline()
            self._print(4, l, ln.strip(), "<",hexln.strip(),">")

        assert self.command_check('finish %04x\n' % saddr)

        # todo verify crc
        self.peer.comm.write('crc %04x\n' % saddr)
        ln = self.peer.comm.readline()
        self._print(3, ln.strip())

        assert self.command_check('exit %04x\n' % saddr)

        assert self.ping_check(expapp = "wuart")

    def command_check(self, command):
        rv_ok = True
        self.peer.comm.write(command)
        ln = self.peer.comm.readline()
        self._print(4, ln.strip())
        if not ln.startswith("OK"):
            rv_ok = False
        return rv_ok

    def ping_check(self, expapp = None):
        rv_ok = True

        self.peer.comm.write("ping %04x\n" % self.dut.board['saddr'])
        ln = self.peer.comm.readline()

        self._print(3, "dut: %(name)s, %(board)s, %(saddr)04x" % self.dut.board)
        if not ln.startswith("OK"):
            rv_ok = False
            self._print(3, "ping: %s" % ln)
        else:
            parms = eval(ln.replace("OK ",""))
            self._print(3, "ping: %(short_addr)04x, %(boardname)s, %(appname)s" % parms)

            if parms['boardname'] != self.dut.board['board']:
                self._print(1, "board fail", parms['boardname'])
                rv_ok = False
            if parms['short_addr'] != self.dut.board['saddr']:
                self._print(1, "saddr fail", parms['short_addr'])
                rv_ok = False
            if expapp != None and parms['appname'] != expapp:
                self._print(1, "appname fail", parms['appname'])
                rv_ok = False

        return rv_ok


if __name__ == '__main__':
    rv = T.parse_test_options(__doc__,
                              ['setupcfg', 'boardcfg', 'bindir',
                               'peer', 'duts', 'noflash'])
    if not rv:
        sys.exit(1)

    testnodes = T.get_testnodes(rv["setupcfg"], rv['boardcfg'], rv['bindir'])

    peer = testnodes.get(rv["peer"])

    duts = {dut: testnodes.get(dut) for dut in rv["duts"]}

    for d in [peer] + duts.values():
        d.board["saddr"] = eval(d.board["saddr"])

    suites = []

    if not rv['noflash']:
        saddr = peer.board['saddr']
        outfile = peer.board.find_app("wibohost").replace(".hex", "_%04x.hex" % saddr)
        nodeaddr.nodeaddr(board = peer.board['board'],
                          infile = peer.board.find_app("wibohost"),
                          channel = 17,
                          saddr = peer.board["saddr"],
                          outfile = outfile)
        s = T.testsuite_factory(T.Test_Fw_Flash,
                                "flash_wibohost:%s" % peer,
                                dut = peer,
                                fw = outfile,
                                params = rv)
        suites.append(s)

        for d in duts.values():
            saddr = d.board["saddr"]
            outfile = d.board.find_app("wibo").replace(".hex", "_%04x.hex" % saddr)
            nodeaddr.nodeaddr(board = d.board['board'],
                              bootloader = d.board.find_app("wibo"),
                              infile = d.board.find_app("wibotest"),
                              channel = 17, saddr = saddr,
                              outfile = outfile)
            s = T.testsuite_factory(T.Test_Fw_Flash,
                                    "flash_wibo_%s" % d,
                                    dut = d,
                                    fw = outfile,
                                    params = rv)
            suites.append(s)

    for dn, dut in duts.items():
        s = T.testsuite_factory(Test_Fw_Wibo,
                                "wibo_peer=%s_dut=%s" % (peer, dut),
                                params = rv, dut = dut, peer = peer)
        suites.append(s)

    T.test_suites_run(suites, **rv)

# EOF
