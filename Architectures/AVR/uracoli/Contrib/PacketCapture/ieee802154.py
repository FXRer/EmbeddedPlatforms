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

##
# @page pgCCUCap Application Description
#
# (@ref pgContribCapture "back")
#
# @section secCapSyn Synopsis
#
#   <tt>python ieee802154.py -p PORT|-r FILE [OPTIONS]</tt>
#
# @section secCapOpt Options
#
# @par General IO Options
#
# - @opt2{-p,portname}\n  Name of the port which is read, e.g. @c /dev/ttyUSB0
# - @opt2{-r,capfile}\n   Name of the capture file which is read @c mytrace.dat
# - @opt2{-i,pipename}\n  Name of the pipe
#
# @par Options for use with -p
#
# - @opt2{-c,channel}\n set IEEE802.15.4 channel number.
# - @opt2{-R, <r>} \n set IEEE802.15.4 data rate.
#
# @par General Options
#
# - @opt2{-N,pkgcnt}\n    Stop after @c <pkgcnt> packages are received.
# - @opt{-v}\n            Set verbose output, multiple @c -v options
#                         increase the message details.
#
# @section secCapCmd Online Commands
#
# At the prompt @c "cmd:" the following commands are available:
#
#    - @opt{reopen} - reopens the named pipe and places a pcap header,
#                 so that wireshark can reconnect.
#    - @opt2{chan,<n>} \n set new channel to sniff.
#    - @opt2{rate, <r>} \n set new data rate.
#    - @opt{devrst}   \n reinitialize the sniffer device
#    - @opt{debug}    \n start python debugger
#    - @opt{info}     \n show status of sniffer DEVIN
#    - @opt{quit}     \n end session and terminate all threads
#    - @opt{help}     \n show command help
#
# <hr/>

##
# @file
# @ingroup grpContribCapture
# @brief Capture converter script
#
# This script implements a bridge between a file or a serial port
# to a socket. This socket is the interface for wireshark/libpcap
# in order to do a live capture.
#
# How the script is used is described @ref pgContribCapture "here".
#

# === import ==================================================================
import sys, os, pdb, atexit
import traceback, getopt

# === import local moduls =====================================================
from ieee802154_base import ProgramContext
from ieee802154_io import FileIn, PortIn
from ieee802154_pipe import PipeOut

# === globals =================================================================
VERSION = "0.1"
CTX = ProgramContext()
DEVOUT = PipeOut()
DEVIN = None
# === classes =================================================================

# === functions ===============================================================

##
# Terminate all instances of the IO classes
#
def do_quit(exclude=[]):
    global CTX,DEVIN,DEVOUT,gui
    print "quit: exclude:",exclude
    if gui != None and 'gui' not in exclude:
        print "quit_gui"
        gui.cancel()
        gui = None
    if DEVIN != None:
        print "quit_DEVIN"
        DEVIN.reset()
        DEVIN.close()
        DEVIN = None
    if DEVOUT != None:
        print "quit_socket"
        DEVOUT.close()
        DEVOUT = None

def print_options():
    print """\
Usage:
  %s [OPTIONS]
  Options:
   -p PORT     Name of the serial port where the sniffer plattform
               is connected, e.g. /dev/ttyUSB0 or COM1.
   -r CAPFILE  Name of a capture file which is read @c mytrace.dat.
   -i PIPE     Name of the named pipe.
                On a Linux system, PIPE can be a absolut path name, e.g.
                /tmp/ieee802154. On a Windows system PIPE is a name in a
                special pipe directory, e.g. foo maps to \\\\.\\pipe\\foo.
                [ieee802154]
   -c CHAN     IEEE802.15.4 channel number
   -R RATE     IEEE802.15.4 data rate
   -v          increase verbose level
   -h          show help and exit.
   -V          print version and exit.

  Note:
   * -r and -p can be used only exclusively,
   * -c works only with -p.
""" % sys.argv[0]


##
# print a list of the commands available on the
# "cmd:" prompt
def print_help():
    print """\
Commands:
   reopen   - reopens the named pipe and places a pcap header,
              so that wireshark can reconnect.
   chan <n> - set IEEE 802.15.4 channel.
   rate <r> - set IEEE 802.15.4 data rate (e.g. BPSK20, OQPSK100, ...)
   devrst   - reinitialize the sniffer device
   debug    - start python debugger
   info     - show status of sniffer DEVIN
   verbose  - verbose command
   quit     - end session and terminate all threads
   help     - show command help
"""

##
# print startup message with some essential information about
# the current configuration.
#
def print_banner():
    global VERSION, DEVIN
    fmtstr = {'file':'%(file)s, frames: %(frames)s',
              'port':'%(port)s, %(platform)s, channel=%(chan)s, rate=%(crate)s',
             }
    print "\nuracoli sniffer to wireshark bridge V%s" % VERSION
    inf = DEVIN.info()
    fmt = fmtstr.get(inf['type'],"UNKNOWN INTERFACE TYPE")
    print ' OPTS:  ', " ".join(sys.argv[1:])
    print ' INPUT: ', fmt % inf
    print ' OUTPUT:', str(DEVOUT.TxPipe),'\n'
##
# Interpretation of command line options.
# @param argv  list with command line elements.
#
def process_command_line_args(argv):
    global CTX
    opts,args = getopt.getopt(argv,"i:hp:r:vVNR::c:x")
    for o,v in opts:
        if o == "-i":
            v = v.strip()
            if os.path.isfile(v) or os.path.islink(v):
                print "Can't overwrite existing file or link with a pipe"
                sys.exit(1)
            CTX.SOCKNAME = v
        if o == "-r":
            CTX.CAPFILE = v.strip()
            CTX.PORT = None
        if o == "-p":
            CTX.CAPFILE = None
            CTX.PORT = v.strip()
        if o == "-v":
            CTX.VERBOSE += 1
        if o == "-N":
            CTX.MAXPACKETS = eval(v)
        if o == "-c":
            CTX.CHANNEL = eval(v)
        if o == "-R":
            CTX.RATE = v.strip()
        if o == "-x":
            CTX.GUI = True
        if o == "-h":
            print_options()
            print_help()
            sys.exit(0)
        if o == "-V":
            print "%s V%s" % (sys.argv[0],VERSION)
            sys.exit(0)

    if not CTX.CAPFILE and not CTX.PORT:
        CTX.PORT = raw_input('Enter Capture Port:')

##
# process interactive command
#
def process_command(cmd):
    ret = 1
    cmd = cmd.strip()
    if len(cmd) == 0:
        pass
    elif cmd == "help":
        print_help()
    elif cmd == "quit":
        do_quit()
        ret = 0
    elif cmd == "info":
        inf = DEVIN.info()
        ks = inf.keys()
        ks.sort()
        fmt = " % 9s : %s"
        print "INPUT:"
        for k in ks:
            print fmt  % (k,inf[k])
        print "OUTPUT:"
        print fmt % ("pipe",DEVOUT.TxPipe)
    elif cmd == "reopen":
        DEVOUT.reopen()
    elif cmd.find("chan")>=0:
        try:
            CTX.CHANNEL = eval(cmd.split(" ")[1])
        except:
            CTX.CHANNEL = eval(raw_input("channel:"))
        DEVIN.set_channel(CTX.CHANNEL)
    elif cmd.find("rate")>=0:
        CTX.RATE = cmd.split(" ")[1].strip()
        DEVIN.set_rate(CTX.RATE)
    elif cmd.find("debug")>=0:
        pdb.set_trace()
    elif cmd.find("N")>=0:
        CTX.MAXPACKETS = eval(cmd.split(" ")[1])
        DEVIN.FCNT=0
    elif cmd.find("devrst")>=0:
        DEVIN.init()
        DEVIN.set_channel(CTX.CHANNEL)
    elif cmd.find("verbose") == 0:
        try:
            lvl = eval(cmd.split(" ")[1].strip())
        except:
            print "command '%s' was ignored" % cmd
        else:
            setverbose(lvl)
            print "verbose level is now %d" % CTX.VERBOSE
    else:
        print "\n  Unknown Command: \"%s\"," %cmd
        print "  type \"help\" to see a list of available commands.\n"
    return ret

def setverbose(level):
    CTX.VERBOSE = level
    DEVOUT.setverbose(CTX.VERBOSE)
    DEVIN.setverbose(CTX.VERBOSE)
    
##
# exit handler, prompts on all Non-*NIX systems for "ENTER" in
# order to see the last error message in the terminal.
def do_exit(*argv,**argd):
    do_quit()
    if os.name != 'posix':
        raw_input("Type Enter to Exit...")

# === init ====================================================================
if __name__ == "__main__":
    try:
        import rlcompleter
        import readline
        readline.parse_and_bind("tab:complete")
    except:
        pass

    atexit.register(do_exit)

    ## This class instance stores the global programm context
    process_command_line_args(sys.argv[1:])

    # create input reader classes
    if CTX.CAPFILE:
        DEVIN = FileIn()
        DEVIN.open(CTX.CAPFILE)
    elif CTX.PORT:
        DEVIN = PortIn()
        DEVIN.open(CTX.PORT)
        if CTX.CHANNEL != None:
            DEVIN.set_channel(CTX.CHANNEL)
        if CTX.RATE != None:
            DEVIN.set_rate(CTX.RATE)
    else:
        raise Exception, 'no port or capture file given'

    #DEVOUT = PcapSocket(CTX.SOCKNAME, DEVIN)
    DEVOUT.open(CTX.SOCKNAME, DEVIN)
    setverbose(CTX.VERBOSE)
    DEVIN.maxpackets = CTX.MAXPACKETS

    if CTX.GUI:
        gui = CtrlGui(lambda : do_quit(['gui']))
        gui.start()
        gui.setDEVIN(DEVIN)
    else:
        gui = None

    run = 1
    print_banner()
    while run:
        try:
            cmd = raw_input("cmd:")
            run = process_command(cmd)
        except:
            traceback.print_exc()
            break
