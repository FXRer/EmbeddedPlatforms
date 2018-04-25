#   Copyright (c) 2008, 2009 Axel Wachtler
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
# @file
# @ingroup grpContribCapture
# @brief Wireshark related socket class
#
# (see @ref grpContribCapture "here" for related files)
#

# === import ==================================================================
import sys, threading, socket, os
from ieee802154_base import *

# === globals =================================================================

# === functions ===============================================================

# === classes =================================================================

## Socket class
# this class umplements the interface to the
# libpcap/wireshark application.
class SocketOut(PcapBase):
    ## conctructor
    def open(self, sockname, devin):
        self.sockname = sockname
        self.InputDev = devin
        if os.path.exists(self.sockname):
            self.message(1, "remove old socket")
            os.remove(self.sockname)
        self.TxSocket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.TxSocket.bind(self.sockname)
        self.TxThread=threading.Thread(target = self.__tx__)
        self.TxThread.setDaemon(1)
        self.TxThread.start()

    ## closing the socket
    def close(self):
        self.TxSocket.close()
        if self.TxThread != None:
            self.TxThread.join(self.TMO)
            self.TxThread = None

    ## periodically send data retrived from the input device
    # (a instance of @ref PcapFile or @ref PcapFile).
    def __tx__(self):
        run = 1
        cnt = 0
        while run:
            self.TxSocket.listen(1)
            conn, addr = self.TxSocket.accept()
            self.message(1,"accepted connection sock=%s addr=%s", conn, addr)
            while 1:
                try:
                    d = self.InputDev.read_packet()
                    if d:
                        conn.sendall(d,socket.MSG_EOR)
                        self.message(1,"cnt=%d pack=%d len=%d", cnt, self.InputDev.FCNT, len(d))
                        if self.VERBOSE > 1:
                            self.message(self.VERBOSE,":%08d:P:% 3d:%s",
                                         self.InputDev.FCNT,len(d),str(self.InputDev))
                            self.message(self.VERBOSE,":".join(map(hex,map(ord,d))))
                        cnt += 1
                except socket.error:
                    si = sys.exc_info()
                    if si[1].args[0]!=32:
                        self.exc_handler("SocketError: %s" % str(si[1].args))
                    break
                except:
                    self.exc_handler("End Thread cnt=%d" % cnt)
                    run = 0
                    break
        self.message(1,"Packets: fcnt=%d", self.InputDev.FCNT)

# === EOF ====================================================================
