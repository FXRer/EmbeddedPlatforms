#   Copyright (c) 2009 Axel Wachtler
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
# @brief Wireshark related pipe class
#
# (see @ref grpContribCapture "here" for related files)

# === import ==================================================================
import os, threading, traceback, time
from ieee802154_base import UtilBase

# === globals =================================================================

# === functions ===============================================================

# === classes =================================================================

if os.name == "posix":

    class NamedPipe:
        pipe_handle = None
        pipe_open = False
        def __init__self(self):
            self.pipe_handle = None
            self.pipe_name = "???"

        def create_pipe(self,name):
            self.pipe_name = name
            if not os.path.exists(name):
                os.mkfifo(name)

        def open(self,name,mode):
            self.pipe_handle = open(name, mode)
            self.pipe_name = name
            self.pipe_open = True
            return self

        def write(self,d):
            ret = 0
            if self.pipe_open:
                ret = self.pipe_handle.write(d)
                self.pipe_handle.flush()
            return ret

        def close(self):
            if self.pipe_handle:
                self.pipe_handle.close()
            self.pipe_open = False

        def __del__(self):
            name = self.pipe_handle.name
            if os.path.exists(name):
                os.remove(name)

        def __str__(self):
            return self.pipe_name

elif os.name in ("nt",):
    import win32pipe, win32file

    #
    # The code from the Win32 Wireshark named pipes example was used here:
    # http://wiki.wireshark.org/CaptureSetup/Pipes
    #
    # It requires Python for Windows and the Python for Windows Extensions:
    # http://www.python.org
    # http://sourceforge.net/projects/pywin32/
    class NamedPipe:
        pipe_handle = None
        pipe_name = None
        pipe_open = False
        def __init__self(self):
            self.pipe_handle = None
            self.pipe_name = "???"

        def create_pipe(self,name):
            self.pipe_name = r'\\.\pipe\%s' % name
            self.pipe_handle = win32pipe.CreateNamedPipe(
                self.pipe_name,
                win32pipe.PIPE_ACCESS_OUTBOUND,
                win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
                1, 65536, 65536, 300, None)

        def open(self,name,mode=None):
            win32pipe.ConnectNamedPipe(self.pipe_handle , None)
            self.pipe_open = True
            return self

        def write(self,d):
            ret = 0
            try:
                if self.pipe_open:
                    ret = win32file.WriteFile(self.pipe_handle, d)
                    win32file.FlushFileBuffers(self.pipe_handle)
            except:
                # try a reconnect
                self.pipe_open = False
            return ret

        def close(self):
            try:
                win32file.CloseHandle(self.pipe_handle)
                win32pipe.DisconnectNamedPipe(self.pipe_handle)
            except:
                #print sys.exc_info()
                pass
            self.pipe_open = False

        def __del__(self):
            win32file.CloseHandle(self.pipe_handle)

        def __str__(self):
            return self.pipe_name
else:
    pass

## Pipe class
# this class umplements the interface to the
# libpcap/wireshark application via a named pipe.
#
class PipeOut(UtilBase):
    def __init__(self):
        self.TxPipe = NamedPipe()
    ## conctructor
    def open(self, pipename, device):
        self.PipeName = pipename
        self.TxPipe.create_pipe(self.PipeName)
        self.InputDev = device
        self.TxThread=threading.Thread(target = self.__tx__)
        self.TxThread.setDaemon(1)
        self.TxThread.start()

    ## closing threads and IO
    def close(self):
        if self.TxPipe != None:
            self.TxPipe.close()
        if self.TxThread != None:
            self.TxThread.join(self.TMO)
            self.TxThread = None

    ## reopen fifo and place pcap header
    def reopen(self):
        if self.TxPipe.pipe_open:
            self.TxPipe.close()
        self.TxPipe.open(self.PipeName,"w")
        self.TxPipe.write(self.InputDev.pcap_get_header())

    ## periodically send data retrived from the input device
    # (a instance of @ref PcapFile or @ref PcapFile).
    def __tx__(self):
        run = 1
        cnt = 0
        self.message(1,"waiting for somebody to open the pipe")
        self.reopen()
        while run:
            self.message(1,"accepted connection")
            while 1:
                try:
                    d = self.InputDev.read_packet()
                    if d:
                        self.TxPipe.write(d)
                        self.message(1,"cnt=%d pack=%d len=%d", cnt, self.InputDev.FCNT, len(d))
                        if self.VERBOSE > 1:
                            self.message(self.VERBOSE,":%08d:P:% 3d:%s",
                                         self.InputDev.FCNT,len(d),str(self.InputDev))
                            self.message(self.VERBOSE,":".join(map(hex,map(ord,d))))
                        cnt += 1
                    else:
                        time.sleep(0.1)
                except:
                    run = 0
                    traceback.print_exc()
                    break
        self.message(1,"Packets: fcnt=%d", self.InputDev.FCNT)

# === init ====================================================================
