"""
    Test Wibohost class
"""

import wibohost
import pprint
import time
import unittest
import os
import sys
sys.path.append('./Tools')
import hbm251

class TestWibo(unittest.TestCase):

    def setUp(self):
        ret=self.wnwk.reset() # always broadcast, should undeaf all nodes
        self.assertEqual(ret['code'], 'OK')

    def test_echo(self):
        teststr = "Thequickbrownfox"
        ret=self.wnwk.echo(teststr)
        self.assertEqual(ret['code'], 'OK')
        self.assertEqual(ret['data'], teststr)

    def _ping(self, addr):
        ret=self.wnwk.ping(addr)
        self.assertEqual(ret['code'], 'OK')
        return ret['data']

    def test_info(self):
        ret=self.wnwk.info()
        self.assertEqual(ret['code'], 'OK')

        ch, pan, addr = [int(i, 16) for i in self.whost['nodecfg'].split(':')]
        for i,k in [(ch,'CHANNEL'),(pan,'PAN_ID'),(addr,'SHORT_ADDR')]:
            self.assertEqual(i,ret['data'][k])

    def test_ping(self):
        for n in self.wibos:
            ch, pan, addr = [int(i, 16) for i in n['nodecfg'].split(':')]
            d=self._ping(addr)
            self.assertEqual(d['boardname'], n['board'])
            self.assertEqual(d['version'], self.expected_version)

    def test_jbootl(self):
        for n in self.wibos:
            ch, pan, addr = [int(i, 16) for i in n['nodecfg'].split(':')]
            d=self._ping(addr)
            if d['appname'] != 'wibo':
                self.wnwk.jbootl(addr)
                d=self._ping(addr)
                self.assertEqual(d['appname'], 'wibo')

    def test_deaf(self):
        for n in self.wibos:
            ch, pan, addr = [int(i, 16) for i in n['nodecfg'].split(':')]
            self._ping(addr)
            self.wnwk.deaf(addr)
            ret=self.wnwk.ping(addr)
            self.assertEqual(ret['code'], 'ERR')
            self.assertEqual(ret['data'], 'ping timeout')

    def test_crc(self):
        """Test if running CRC Host-Wibo is doing the right thing"""

        smpl_hex = [
            ":10044000310C0E94CE08FC01108101810E94CE086F"
            ":10045000FC01C281D3810E94CE080F931F93DF93CA"
            ":10046000CF93FC0184851F928F9382E391E09F9349"
        ]
        smpl_crc = 0x78B1

        # after call of reset() (in setUp function) CRC has to be zero for host and all wibos
        ret=self.wnwk.crc()
        self.assertEqual(ret['code'], 'OK')
        self.assertEqual(int(ret['data'],16), 0)
        for n in self.wibos:
            ch, pan, addr = [int(i, 16) for i in n['nodecfg'].split(':')]
            ret=self._ping(addr)
            self.assertEqual(ret['crc'], 0)

        dut=self.wibos[0] # select one for test
        ch, pan, addr = [int(i, 16) for i in dut['nodecfg'].split(':')]
        self.wnwk.target(addr, 'X')
        
        for ln in smpl_hex:
            self.wnwk.feedhex(addr, ln)

        # Check CRC of Host
        ret=self.wnwk.crc()
        self.assertEqual(ret['code'], 'OK')
        self.assertEqual(int(ret['data'],16), smpl_crc)

        # Check CRC of test node
        ret=self._ping(addr)
        self.assertEqual(ret['crc'], smpl_crc)

if __name__ == '__main__':
    assert len(sys.argv) >= 2
    setupcfg = sys.argv[1]
    assert os.path.exists(setupcfg)

    testcls = TestWibo
    testcls.expected_version = 5 # make me variable
    testcls.hbm251 = hbm251.HBM251(setupcfg, './Config/board.cfg', './install/bin')

    testcls.whost = testcls.hbm251.boards[2] # arbitrary
    testcls.wibos = [testcls.hbm251.boards[0], testcls.hbm251.boards[1],testcls.hbm251.boards[3]]
    # testcls.wibos = [b for b in testcls.hbm251.boards if b!=testcls.whost]

    print "Hardware Setup, one selected as Host App, Wibo App the rest"

    for b in testcls.hbm251.boards:
        print {True:'Host',False:'Wibo'}[b==testcls.whost], b

# Lets assume, the boards are already flashed with the apps assigned above
#    brd = testcls.hbm251.boards[2]
#    brd.flash(testcls.hbm251.find_app('wuart', brd['board']), dryrun=1, verbose=1)

    testcls.wnwk=wibohost.WIBONetwork(port=testcls.whost['port'], baudrate=testcls.whost['baudrate'], timeout=0.1)
    time.sleep(0.1) # time for boot printout

    suite = unittest.TestLoader().loadTestsFromTestCase(testcls)
    unittest.TextTestRunner(verbosity=2).run(suite)

# EOF
