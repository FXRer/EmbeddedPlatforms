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

# === import ==================================================================
from Tkinter import *
import serial, traceback

# === globals =================================================================
PORTNAME = "/dev/ttyUSB1"
BAUDRATE = 9600
SPORT = None

# === functions ===============================================================

# === classes =================================================================
class LCD:
    global SPORT
    def toggle_sym(self,name):
        try:
            val = getattr(mylcd, name)
        except:
            val = 0
            setattr(mylcd, name, val)
        val ^= 1
        cmd = "sym %s %d\n" % (name, val)
        SPORT.write(cmd)
        setattr(mylcd,name, val)

    def print_lcd(self,text):
        text = text.strip()
        cmd = "print %s\n" % text
        SPORT.write(cmd)

    def put_lcd(self, text):
        text = text.strip()
        try:
            text = eval(text)
            if type(text) == type(1):
                cmd = "puti %d\n" % text
                SPORT.write(cmd)
        except:
            print "Argument is not intger [%s]" % text

    def cls(self):
        SPORT.write("cls\n")

# === init ====================================================================

if __name__ == "__main__":
    SPORT = serial.Serial(PORTNAME, BAUDRATE, timeout=3)
    SPORT.close()
    SPORT.open()


    mylcd = LCD()

    root = Tk()
    root.title("Raven LCD Debugger")


    e = Entry(root, width=50)
    e.pack()

    Button(master = root, text = "PRINT",
           command = lambda : mylcd.print_lcd(e.get())).pack(fill=X)
    Button(master = root, text = "PUT",
           command = lambda : mylcd.put_lcd(e.get())).pack(fill=X)

    cmd = "HiRaven!"
    SPORT.write("echo %s\n" % cmd)
    rsp = SPORT.readline()
    if cmd != rsp.strip():
        # raven is absent
        print "no echo from raven", cmd, rsp
        sys.exit(1)

    # === Symbol field =================================================
    symfrm = Frame(root)
    symfrm.pack(fill = X)

    SPORT.write("listsymbols\n")
    row = 0
    col = 0
    while 1:
        try:
            x = SPORT.readline().strip()
            sym = x.split()[1]
            cmd = eval("lambda : mylcd.toggle_sym('%s')" % sym)
            Button(master = symfrm, text = sym, command = cmd, width = 10).grid(row=row, column=col)
            col += 1
            if col > 4:
                row += 1
                col = 0
        except:
            traceback.print_exc()
            break

    Button(master = root, text = "CLS", command = mylcd.cls).pack(fill=X)
    Button(master = root, text = "QUIT", command = root.quit).pack(fill=X)
    root.mainloop()
