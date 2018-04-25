#   Copyright (c) 2015 Daniel Thiele, Axel Wachtler
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
RDiag application Test Script

Example Usage:
   python Regression/test_fw_rdiag.py -C myconfig.cfg -d dev2,dev0 -p dev3 -v

"""

import sys
import random
import time
import string
import re
import testenv as T

class Test_Fw_RDiag(T.Test_Fw_Base):
    """Test Application rdiag"""

    channel = 17

    def setUp(self):
        self.states={'o':'>STATE_OFF', 'r':'>STATE_RX', 't':'>STATE_TX', 'z':'>STATE_SLEEP'}

    def _parse_cmd(self, cmd, expected, typefunc=int):
        for d in self.devs:
            d.comm.write(cmd)
            ln=d.comm.readline().strip()
            self.assertNotEqual(ln, '')
            s,t = ln.split(':')
            self.assertEqual(expected, s)
            return typefunc(t)

    def _setverbose(self, verbose):
        v=self._parse_cmd('V', '>VERBOSE')
        maxtry=5
        for r in range(maxtry, 0, -1):
            v=self._parse_cmd({True:'v', False:'V'}[v>verbose], '>VERBOSE')
            if v==verbose: break
        self.assertNotEqual(r, 1)

    def _setchannel(self, channel):
        """Internal helper function to set desired channel"""
        maxtry=20 # 16 2.4 GHz channels
        for r in range(maxtry, 0, -1):
            ch=self._parse_cmd('+', '>CHAN')
            if ch==channel: break
        self.assertNotEqual(r, 1)

    def test_helpscreen(self):
        self.dut.comm.write('h')
        ln=self.dut.comm.readline().strip()
        while ln != '':
            self.assertTrue(ln.split(':')>=2)
            ln=self.dut.comm.readline().strip()

    def test_state_change(self):
        for i in range(10):
            cmd=random.sample(self.states,1)[0]
            self._print(2, self.states[cmd][len('>STATE_'):], '', end='')
            self.dut.comm.write(cmd)
            self.assertEqual(self.states[cmd], self.dut.comm.readline().strip())

    def test_rxon_idle(self):
        self.devs = [self.dut]
        for i in range(10):
            t=self._parse_cmd('R', '>RXON_IDLE')
            self._print(2, t, '', end='')
            if i>0: # not the first run
                self.assertNotEqual(t,last)
            last=t

    def _cycle_param(self, cmdup, cmddwn, exp_result, min, max, cycles=20):
        """Internal helper function to cycle a parameter including wrap around"""
        curr = self._parse_cmd(cmddwn, exp_result)

        for updwn in [cmdup, cmddwn]:
            for i in range(cycles):
                p=self._parse_cmd(updwn, exp_result)
                self._print(2, p, '', end='')
                if updwn == cmdup:
                    curr += 1
                    if curr > max: curr=min
                elif updwn == cmddwn:
                    curr -= 1
                    if curr < min: curr=max
                else:
                    raise Exception()
                self.assertEqual(p, curr)

    def test_bootprint(self):
        """ Analyze boot printout """
        # Example boot printout
        # -- ERR regreset nb=10 offs=0x10 val=0x04 exp=0x00
        # -- ERR regreset nb=11 offs=0x11 val=0x22 exp=0x02
        # -- ERR regreset nb=16 offs=0x1d val=0x02 exp=0x01
        # -- OK sram_const(0x00)
        # -- OK sram_const(0x55)
        # -- OK sram_const(0xaa)
        # -- OK sram_const(0xff)
        # -- OK sram_runbit
        # -- OK sram_address
        # -- OK trxirq 31, irqs=1, badirqs=0
        # --
        # -- Radio Diag 0.30
        # -- >RADIO INIT[wdba1281]: OK (status=0x08)
        # -- Timer-Tick 3c0637bd
        # -- RADIO_PART_NUM = 0x02, RG_PART_NUM = 0x02
        # -- RADIO_VERSION_NUM = 0x01, RG_VERSION_NUM = 0x02
        # -- Config: use default settings
        #         
        self.dut.reset(self.reset_wait_time) # member of base class
        lns = map(string.strip, self.dut.comm.readlines())
        self.assertIn('Radio Diag 0.30', lns)
        
        # Uncommented since register reset table has to be reworked in diagradio.c
        # [self.assertFalse(ln.startswith('ERR')) for ln in lns]

        found=False
        for ln in lns:
            m=re.match('RADIO_PART_NUM = ([\w]{4}), RG_PART_NUM = ([\w]{4})', ln)
            if m is not None:
                found = True
                self.assertEqual(len(m.groups()), 2)
                self.assertEqual(m.groups(0), m.groups(1))
        self.assertTrue(found)

    def test_channel(self):
        """Test setting channel via +/- command"""
        self.devs = [self.dut]
        bands = [range(0,10), range(11,27)]
        band=None
        ch=self._parse_cmd('+', '>CHAN')
        for bnd in bands:
            if ch in bnd: band=bnd
        self.assertIsNotNone(band)

        self._cycle_param('+', '-', '>CHAN', band[0], band[-1])

    def test_pwr(self):
        """Test setting tx power via p/P command"""
        self.devs = [self.dut]
        # TODO: move limits to transceiver specific
        self._cycle_param('P', 'p', '>PWR', -17, 3)

    def _send_frame(self, txdev, rxdevs):
        """ Helper function, send frame to multiple Rx devices """

        self.devs = [txdev]+rxdevs
        self._setverbose(1)
        self._setchannel(self.channel) # TODO depends on used Band

        for d in rxdevs:
            d.comm.write('r')
            self.assertEqual('>STATE_RX', d.comm.readline().strip())

        txdev.comm.write('t')
        self.assertEqual('>STATE_TX', txdev.comm.readline().strip())

        txdev.comm.write('s')
        ln=txdev.comm.readline().strip()
        self._print(3, " tx: %s" % ln)
        self.assertEqual('>SEND', ln.split()[0])
        txlen = int(ln.split()[1].split('=')[1])
        ln=txdev.comm.readline().strip()
        self._print(3, " tx_done: %s" % ln)
        self.assertEqual('TX done status', ln.split('=')[0])
        self.assertEqual(0, int(ln.split('=')[1]))

        for d in rxdevs:
            ln=d.comm.readline().strip()
            self._print(3, " rx: %s" % ln)
            self.assertEqual('++FRAME', ln.split()[0])
            rxlen=int(ln.split()[1].split('=')[1])
            self.assertEqual(txlen, rxlen)

    def test_sendframe(self):
        """Send a frame and receive answer"""
        for tx, rx in ((self.dut, self.peer), (self.peer, self.dut)):
            self._print(2, ' Rx:', rx, 'Tx:', tx)
            self._send_frame(tx, [rx])

    def _parse_staticstics(self, dev, cmd):
        """ Helper function, parse print of statistics and return dictionary """

        #### Example output
        # >STAT duration: 0 ticks
        #  RX: frames: 0 crcerr: 0
        #  RX: channel 11 avg ed: -1 avg lqi:  -1
        #  TX: frames: 0
        #
        ####### 
        dev.comm.write(cmd)
        time.sleep(0.1)
        nblines = 4
        lns = [dev.comm.readline().strip() for i in range(nblines)]
        self.assertEqual(len(lns), nblines)
        self.assertTrue(lns[0].startswith('>STAT'))
        d={}
        d.update(re.match('>STAT duration: (?P<duration>\d+) ticks', lns[0]).groupdict())
        d.update(re.match('RX: frames: (?P<rx_frames>\d+) crcerr: (?P<rx_crcerr>\d+)', lns[1]).groupdict())
        d.update(re.match('RX: channel (?P<channel>\d+) avg ed: (?P<avg_ed>[0-9\-]+) avg lqi:  (?P<avg_lqi>[0-9\-]+)', lns[2]).groupdict())
        d.update(re.match('TX: frames: (?P<tx_frames>\d+)', lns[3]).groupdict())

        # convert all fields to signed integers
        [d.update({k:int(v, 10)}) for k,v in d.iteritems()]
        self._print(3, d)
        return d

    def test_statistics(self):
        """ Test for command "i", "I" for statistics """

        rx, tx = self.dut, self.peer

        # After reset command 'I', fields have to be zero
        for dev in [rx,tx]:
            d=self._parse_staticstics(dev, 'I') # reset
            d=self._parse_staticstics(dev, 'i') # fetch values
            self.assertEqual(d['avg_ed'], -1)
            self.assertEqual(d['avg_lqi'], -1)
            self.assertEqual(d['rx_frames'], 0)
            self.assertEqual(d['tx_frames'], 0)

        rep=1
        [self._send_frame(tx, [rx]) for i in range(rep)]

        # Check Rx statistics
        d=self._parse_staticstics(rx, 'i')
        self.assertNotEqual(d['avg_ed'], -1)
        self.assertNotEqual(d['avg_lqi'], -1)
        self.assertEqual(d['rx_frames'], rep)
        self.assertEqual(d['tx_frames'], 0)

        # Check Tx statistics
        d=self._parse_staticstics(tx, 'i')
        self.assertEqual(d['avg_ed'], -1)
        self.assertEqual(d['avg_lqi'], -1)
        self.assertEqual(d['rx_frames'], 0)
        self.assertEqual(d['tx_frames'], rep)

        # After reset command 'I', fields have to be zero
        for dev in [rx,tx]:
            d=self._parse_staticstics(dev, 'I') # reset
            d=self._parse_staticstics(dev, 'i') # fetch values
            self.assertEqual(d['avg_ed'], -1)
            self.assertEqual(d['avg_lqi'], -1)
            self.assertEqual(d['rx_frames'], 0)
            self.assertEqual(d['tx_frames'], 0)

    def test_unsupported(self):
        """Test for "unsupported command" error"""
        self.devs = [self.dut]
        cmd='x'
        r=self._parse_cmd(cmd, 'unsupported command', typefunc=str)
        self.assertEqual(r.strip(),cmd)

    def test_verbose(self):
        """Test setting verbose v/V allowed range: 0..2"""
        self.devs = [self.dut]
        min, max = 0,2
        v=self._parse_cmd('v', '>VERBOSE')
        for i in range(5):
            if v>min: v-=1
            self.assertEqual(v, self._parse_cmd('v', '>VERBOSE'))
        for i in range(5):
            if v<max: v+=1
            self.assertEqual(v, self._parse_cmd('V', '>VERBOSE'))

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
                                    dut = d, fw="rdiag", params = rv)
            suites.append(s)

    for dn, dut in duts.items():
        s = T.testsuite_factory(Test_Fw_RDiag,
                                "rdiag_peer=%s_dut=%s" % (peer, dut),
                                params = rv, dut = dut, peer = peer)
        suites.append(s)

    T.test_suites_run(suites, **rv)

# EOF
