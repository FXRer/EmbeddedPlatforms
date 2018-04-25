#   Copyright (c) 2007 Axel Wachtler
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
# Test script for data troughput of a serial line.
#
# Usage:
#   python -i Tools/wuarttest.py -P[PORT[:RATE]] [-P[PORT[:RATE]]] [SCRIPT [SCRIPT]]
#
#   -i is a python option that makes the python interpreter interactive.
#      Note that this option has to be placed before Tools/wuarttest.py.
#
#   -PPORT[:RATE] PORT is the name of the port (e.g. /dev/ttyS0 or COM1, ...),
#                 RATE is the optional data rate of the port ()
#
#   SCRIPT is/are one or more optional python scripts that executed after the serial
#          interfaces are opened.
# Example
#   python -i Tools/wuarttest.py -P/dev/ttyUSB1:19200  -P/dev/ttyS1 myscript.py
#
#     This command opens USB1 interface at 19200 bit/s and S1 with the default
#     baudrate 9600 and executes the python script "myscript.py"
#
# Commands
#  * d0.verbose = {0,1} set verbose output of the first interface off or on.
#  * d0.write("ATD\n") write a string to the first interface
#  * d1.write("ATD\n") write a string to the second interface
#  * tx_uni(d1,d0,cnt = 10, bs=300,) - perform a unidirectional throuput test,
#  * tx_bi(d1,d0,cnt = 10, bs=300,pattern=65, tpause=0.1) - perform a bidirectional throuput test
#

import serial, threading, traceback, md5, time, copy, sys, random
import getopt

HISTORY = "wuarttest.hist"


try:
    import readline, rlcompleter, atexit
    readline.parse_and_bind("tab:complete")
    save_hist = lambda history : readline.write_history_file(history)
    atexit.register(readline.write_history_file,HISTORY )

except:
    print "No libreadline support"
    traceback.print_exc()


try:
    readline.read_history_file(HISTORY)
except:
    pass

TXTCOL = { 'green' : '\33[32m',
           'red'   : '\33[31m',
           'norm'  : '\33[0m',
           'prompt': '\33[1;4;30m',
           'blue'  : '\33[34m'
         }

class Device:
    def __init__(self,name,port,baud):
        self.name = name
        if baud == None:
            baud = 9600
        self.sport = serial.Serial(port, baud, timeout=1)
        #self.sport.timeout = .1
        #self.sport.writeTimeout = 1
        self.blksize = 32
        self.rxThread = threading.Thread(target = self._rx_thread_)
        self.rxThread.setDaemon(1) # if a thread is not a daemon, the program needs to join all
        self.rxThread.setName("RX_%s" % self.name)
        self.rxThread.start()
        self.reset()
        self.verbose = 0

    def reset(self):
        self.rxcnt = 0
        self.txcnt = 0
        self.time = time.time()

    def statistic(self):
        now = time.time()
        rxc = self.rxcnt
        txc = self.txcnt
        dtime = now - self.time
        txbyte_per_s = self.txcnt/dtime
        rxbyte_per_s = self.rxcnt/dtime
        print "Datarate %s: \n  txr=%.1f byte/s %.1fbit/s [%d]\n  rxr=%.1f byte/s %.1fbit/s [%d]" % (self.name,txbyte_per_s,txbyte_per_s*8, txc, rxbyte_per_s, rxbyte_per_s*8, rxc)

    def _rx_thread_(self):
        while 1:
            try:
                x = self.read()
                if x:
                    self.rxcnt += len(x)
                    if self.verbose:
                        sys.stdout.write(TXTCOL[self.txtcol] + x + TXTCOL['norm'])# + ": %d/%d :" % (len(x), self.rxcnt))
                        sys.stdout.flush()
            except:
                sys.stderr.write("ERROR:rxthread:%s:%s" % (self.name, sys.exc_info()[1]))


    def write(self,data):
        self.sport.write(data)
        #self.sport.flush()
        self.txcnt += len(data)

    def read(self):
        # blocking read for 1.st byte
        x = self.sport.read(1)
        n = self.sport.inWaiting()
        if n:
            x += self.sport.read(n)
        return x

def reset():
    global DEVICES
    for d in DEVICES:
        d.reset()

def tx_uni(dtx, drx, cnt=100, bs=10, tpause=0.01, pattern=None):
    random.seed(2)
    rxbytes = cnt * bs
    dtx.reset()
    drx.reset()
    if pattern == None:
        txdata = "".join(map(lambda x: chr(ord('A')+(x&15) ), range(bs)))
    else:
        txdata = "".join(map(lambda x: chr(pattern), range(bs)))
    for i in range(cnt):
        #txdata = "".join(map(lambda x: chr(random.randint(ord('A'),ord('Z'))), range(bs)))
        dtx.write(txdata)
        dtx.sport.flush()
        time.sleep(.001)
    try:
        while drx.rxcnt < bs*cnt:
            time.sleep(tpause)
    except:
        pass
    print
    dtx.statistic()
    drx.statistic()
    sys.stdout.flush()

def tx_bi(dtx, drx, cnt=100, bs=10, tpause=0.01):
    random.seed(2)
    rxbytes = cnt * bs
    dtx.reset()
    drx.reset()
    for i in range(cnt):
        txdata = "".join(map(lambda x: chr(random.randint(ord('A'),ord('Z'))), range(bs)))
        dtx.write(txdata)
        dtx.sport.flush()
        time.sleep(.01)
        drx.write(txdata)
        dtx.sport.flush()
        time.sleep(.01)
    time.sleep(.1)
    try:
        while drx.rxcnt < bs*cnt:
            time.sleep(tpause)
    except:
        pass
    print

    sys.stdout.flush()
    print
    dtx.statistic()
    drx.statistic()
    sys.stdout.flush()

if __name__ == "__main__":

    opts,args = getopt.getopt(sys.argv[1:],"P:")

    interface_list = []
    for o,v in opts:
        if o == "-P":
            try:
                # try to split /dev/ttyS0:9600
                tmp = tuple(v.split(":"),2)
                tmp[1] = eval(tmp[1])
            except:
                tmp = (v, None)

            interface_list.append(tmp)
        # process other options
    scnt = 0
    DEVICES=[]
    col = ["green","red","blue"]
    for interface,baudrate in interface_list:
        dname = "d%d" % scnt
        d = Device(dname, interface, baudrate)
        d.txtcol = col[scnt]
        exec("%s = d" %dname)
        exec("DEVICES.append(%s)" %dname)
        print "created d%d on %s" % (scnt,interface)
        scnt += 1
    for a in args:
        print "exec:",a
        execfile(a)

def test_rxtx(n=10):
    mtx = md5.md5()
    mrx = md5.md5()
    # clean buffers
    d0.r(2000)
    d1.r(2000)
    rxdata =""
    txdata =""
    f = open("/dev/urandom")
    for i in xrange(n):
        if i % (n/10) == 0:
            print "\b+",
            sys.stdout.flush()
        d = f.read(16)
        d0.w(d)
        txdata += d
        mtx.update(d)
        r = d1.r(64)
        rxdata += r
        mrx.update(r)
        if mtx.digest() != mrx.digest():
            print "Mismatch after %d sz=%d/%d bytes " % (i,len(txdata),len(rxdata))
            raise "ErrorRxTx"
    print
    print "tx sz=%d, md5=%r" % (len(txdata), mtx.hexdigest())
    print "rx sz=%d, md5=%r" % (len(rxdata), mrx.hexdigest())
    return (txdata,rxdata)


def paysize(x):
    ret = 0
    for l in x:
        ret += l
    return (ret - 9 * len(x))
