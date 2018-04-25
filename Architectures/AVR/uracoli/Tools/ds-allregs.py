#   Copyright (c) 2016 Axel Wachtler, Daniel Thiele
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

##
# Print overview of registers of all known transceivers
#
#
import sys
import glob
import ds2reg

##
# Main function
# Usage:
#   python ds-allregs.py
#
if __name__ == "__main__":
    globpattern = sys.argv[1]

    regs=[ds2reg.process_templatefile(fname) for fname in glob.glob(globpattern)]

    offss=[r['offset'] for rm in regs for r in rm['REGMAP']]
    minreg, maxreg=min(offss), max(offss)

    files=[r['FILE'] for r in regs]
    print ' | '.join(['Address']+files)
    print ' | '.join(['---']*len(files))
    for addr in range(minreg, maxreg):
        s=['0x%02X'%addr]
        for fn in files:
            lst=[r['name'] for rm in regs for r in rm['REGMAP'] if rm['FILE']==fn and r['offset']==addr]
            if len(lst)==0:
                s+=['']
            else:
                s+=lst
        print ' | '.join(s)
# EOF
