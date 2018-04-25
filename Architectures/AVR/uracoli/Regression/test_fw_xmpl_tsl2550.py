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
Check if xmpl_tsl2550<board>.hex works as expected

Example Usage:

    python Regression/test_fw_xmpl_tsl2550.py -C myjenkins.cfg -d dev0 -vvv

"""

import re
import serial
import sys
import testenv as T

class Test_Fw_XmplTsl2550(T.Test_Fw_Base):

    def test_help(self):
        dut = self.dut
        dut.comm.write("h")
        answer = dut.comm.read(200)
        self._print(3, answer)
        self.assertEqual(re.search("Help", answer).group(), "Help")

    def test_config(self):
        dut = self.dut
        dut.comm.write("p")
        answer = dut.comm.read(200)
        self._print(3, answer)
        self.assertGreaterEqual(answer.find("power down"), 0)

        dut.comm.write("P")
        answer = dut.comm.read(200)
        self._print(3, answer)
        self.assertTrue(answer.startswith("power up"))

        dut.comm.write("e")
        answer = dut.comm.read(200)
        self._print(3, answer)
        self.assertTrue(answer.startswith("use ext. range"))

        dut.comm.write("s")
        answer = dut.comm.read(200)
        self.assertTrue(answer.startswith("use std. range"))

    def test_measurement(self):
        dut = self.dut
        dut.comm.write("m")
        answer = dut.comm.read(200)
        self._print(3, answer)
        self.assertTrue(answer.startswith("run measurement"))
        # Example line
        #adc0: 0xc0 adc1: 0xc0 E[lux]: 0
        parse = re.search("adc0: (0x[0-9a-f]{2}) adc1: (0x[0-9a-f]{2}) E\[lux\]: ([0-9]+)", answer)
        adc0, adc1, E = map(eval,parse.groups())

if __name__ == '__main__':

    rv = T.parse_test_options(__doc__,
                              ["setupcfg", "boardcfg", "bindir",
                               "duts", "noflash"])
    if not rv:
        sys.exit(1)

    testnodes = T.get_testnodes(rv["setupcfg"], rv['boardcfg'], rv['bindir'])

    duts = {dut: testnodes.get(dut) for dut in rv["duts"]}
    suites = []

    if not rv['noflash']:
        for d in duts.values():
            s = T.testsuite_factory(T.Test_Fw_Flash,
                                    "flash_xmpl_tsl2550_%s" % d,
                                    dut = d, fw="xmpl_tsl2550", params = rv)
            suites.append(s)

    for dn, dut in duts.items():
        s = T.testsuite_factory(Test_Fw_XmplTsl2550,
                                "xmpl_tsl2550_dut=%s" % dut,
                                params = rv, dut = dut)
        suites.append(s)

    T.test_suites_run(suites, **rv)


#EOF
