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
XmplHif Test Script

Example Usage:
    python Regression/test_fw_xmpl_hif.py -C myconfig.cfg -d dev3,dev2  -vvv

"""
import sys
import re
import time

import testenv as T

class Test_Fw_XmplHif(T.Test_Fw_Base):

    def setUp(self):
        super(Test_Fw_XmplHif, self).setUp()
        self.dut.comm.flush()

    def test_bootselfprint(self):
        """ Parse for output """
        dut = self.dut
        dut.reset()
        time.sleep(self.get_rwt(self.dut))
        dut.comm.readline()
        outp=dut.comm.readline()
        self._print(3, "answer", len(outp), outp)
        self.assertEqual(outp.split(':')[0].strip(), 'HIF Example')
        time.sleep(0.1) # wait for complete boot printout
        dut.comm.read(dut.comm.inWaiting())

    def test_prompt(self):
        """ test prompt """
        dut = self.dut
        dut.comm.write("\n")
        answer = dut.comm.read(20).strip()
        self._print(3, "answer", answer)
        self.assertTrue(answer.startswith("uracoli"))

    def test_promptcnt(self):
        """ test if counter in prompt counts """
        dut = self.dut

        # loopcounter i does not match the prompt index!
        for i in range(3):
            dut.comm.write("\n")
            answer = dut.comm.read(20).strip()
            nb = float(re.match("uracoli\[([0-9]*)]", answer).group(1))
            if i==0:
                lastnb = nb-1 # assign at the first run
            else:
                lastnb += 1
            self._print(3, "answer:%d"%i, answer, nb)
            self.assertEqual(nb, lastnb+1)

    def test_echo(self):
        """ test if echo works """
        dut = self.dut
        test_text = "hallo welt"
        dut.comm.write(test_text + "\n")
        answer = dut.comm.read(len(test_text))
        self._print(3, "answer:", answer)
        self.assertTrue(answer.startswith(test_text))

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
                                    "flash_xmpl_hif_%s" % d,
                                    dut = d, fw="xmpl_hif", params = rv)
            suites.append(s)

    for dn, dut in duts.items():
        s = T.testsuite_factory(Test_Fw_XmplHif,
                                "xmpl_hif_dut=%s" % dut,
                                params = rv, dut = dut)
        suites.append(s)

    T.test_suites_run(suites, **rv)
# EOF
