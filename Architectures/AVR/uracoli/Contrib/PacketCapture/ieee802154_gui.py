#   Copyright (c) 2008 Axel Wachtler
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

# === import ==================================================================
#from Tkinter import *
import threading
# === globals =================================================================

# === functions ===============================================================

# === classes =================================================================

# === init ====================================================================
class CtrlGui(threading.Thread):
    pass
##    def __init__(self, quitfunc=None):
##        self.device=None
##        self.quitfunc=quitfunc
##        self.root=Tk()
##        w=Label(self.root, text='uracoli Sniffer Interface')
##        w.pack()
##
##        #button=Button(self.root,text='QUIT',command=self.cancel)
##        #button.pack()
##
##        threading.Thread.__init__(self)
##
##    def run(self):
##        self.root.mainloop()
##
##    def cancel(self):
##        self.root.quit()
##        print "QUIT GUI"
##        if self.quitfunc:
##            self.quitfunc()
##
##    def setdevice(self,device):
##        self.device=device
##        print self.device.__class__
##
##        self.v = IntVar()
##        self.v.set(0) # initialize
##
##        for c in self.device.clist:
##            b = Radiobutton(self.root, text=str(c),
##                            variable=self.v, value=c, command = self.getchannel)
##            b.pack(anchor=W,side=LEFT)
##        self.v.set(self.device.channel) # initialize
##
##    def getchannel(self):
##        if self.device:
##            newchannel = self.v.get()
##            currchannel = self.device.channel
##            if newchannel != currchannel:
##                self.device.set_channel(newchannel)
##
##
