#   Copyright (c) 2016 Axel Wachtler
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

# === import ==================================================================
import bottle, serial.tools.list_ports as list_ports, serial
from bottle.ext.websocket import GeventWebSocketServer, websocket
from gevent import Timeout
import time
import json
import os
import md5
import threading
from Queue import Queue
from pprint import pprint as pp
import traceback
import sys

ppd = lambda x: pp(dir(x))

# === globals =================================================================

CONFIG = {}
WORKERS = {}
# === helper functions =========================================================

def load_cfg(cfg_file = "dude_app.json"):
    cfg = {}
    with open(cfg_file, "r") as f:
        cfg = json.load(f)
        f.close()
    # initialize missing entries
    for p in list_ports.comports():

        port = p[0]
        if port.find("AMA0")>-1:
            continue
        if not cfg['ports'].get(port):
            cfg['ports'][port] = {}
    return cfg

def save_cfg(cfg_file = "dude_app.json"):
    with open(cfg_file, "w") as f:
        json.dump(CONFIG, f, sort_keys=True, indent=4, separators=(',', ': '))
        f.close()

# generate parameters for "main" view.
def get_com_ports_dict():
    res = []
    for p in sorted(list_ports.comports()):
        port = p[0]
        user = CONFIG['ports'][port].get('user')
        if user:
            used_by = ' <em style="color:red">... used by %s</em>' % user
        else:
            used_by = ""
        d = dict( port = port, user = user,
                  used_by = used_by,
                  board = CONFIG['ports'][port].get("board","-- undefined --"))
        res.append(d)
    return dict(res =  res, time = time.asctime())

# generate parameters for "terminal" view.
def get_terminal_dict(port, user=None):
    rv = dict(user = user,
                port=port,
                time = time.asctime(),
                cfg = str(json.dumps(CONFIG['ports'][port], sort_keys=True, indent=4)),
                files = get_list_of_files(CONFIG['files'],
                                          CONFIG['ports'][port].get("board")))
    return rv

def get_list_of_files(cfg, board):
    rv = []
    for fmd5,c in cfg.items():
        fn = c['filepath'] + "/" + c['filename']
        c['fn'] = fn
        if os.path.isfile(fn):
            with open(fn) as f:
                hmd5 = md5.md5()
                hmd5.update(f.read())
                f.close()
                if hmd5.hexdigest() == fmd5 and board == c['board']:
                    rv.append(c)
    return rv

class SerialWorker:
    def __init__(self, port, baudrate = 38400):
        self.webQueue = {}
        self.TxQueue = Queue()
        self.port = port
        self.baudrate = baudrate
        self.sport = serial.Serial()
        self.sport.setPort( self.port )
        self.sport.setBaudrate( self.baudrate )
        self.sport.setTimeout( 0.01 )
        self.sport.open()
        self.rxThread = threading.Thread(target = self._serial_worker_)
        self.rxThread.setDaemon(1) # if a thread is not a daemon, the program needs to join all
        self.rxThread.setName("RX_%s" % port)
        self.rxThread.start()

    def attach_queue(self, webq):
        print "attach[%s,%s]" % (self.port, id(webq))
        self.webQueue[id(webq)] = webq

    def detach_queue(self, webq_id):
        print "detach[%s,%s]" % (self.port, webq_id)
        del(self.webQueue[webq_id])

    def _serial_worker_(self):
        while 1:
            try:
                isopen = self.sport.isOpen()
                if isopen:
                    if not self.TxQueue.empty():
                        msg = self.TxQueue.get()
                        self.sport.write(msg)
                        print "wq-get[%s,%d]: %d [%s]" % (self.port, self.TxQueue.qsize(), len(msg), repr(msg))
                    t1 = time.time()
                    message = self.sport.read(1)
                    inw = self.sport.inWaiting()
                    if inw > 0:
                        message = self.sport.read(inw)
                    t2 = time.time()
                    if len(message):
                        #print "%f:len=%d [%s] %s" % (t2-t1, len(message), message.strip(), self.webQueue)
                        for wsid, ws in self.webQueue.items():
                            print "wq-put[%s,%d,%d]: %d [%s]" % (self.port, ws.qsize(), wsid , len(message), repr(message))
                            ws.put(message)
                    else:
                        time.sleep(.1)
            except Exception, e:
                if str(e).find("WebSoc"):
                    print "ws:warning:",self.port,e
                    self.sport.close()
                    self.sport.open()
                    #traceback.print_exc()
                else:
                    print "ws:error:", e
                    traceback.print_exc()
                    break
                time.sleep(.5)

        print "exit _serial_worker_ thread"

# === web routing funcions =====================================================

@bottle.route('/<filepath:path>')
def server_static(filepath):
    return bottle.static_file(filepath, root = "dude_app")

@bottle.route("/")
@bottle.view("main")
def main():
    return get_com_ports_dict()

@bottle.post("/q")
@bottle.view("main")
def post_show_serials():
    port = bottle.request.forms.get("port")
    CONFIG['ports'][port]["user"] = None
    return get_com_ports_dict()

@bottle.post("/upload")
@bottle.view("terminal")
def upload():
    port = bottle.request.forms.get("port")
    user = bottle.request.forms.get("user")
    upload = bottle.request.files.get('firmware')

    file_path = "/tmp/dude_app/%s" % (user)
    if not os.path.isdir(file_path):
        os.makedirs(file_path)
    upload.save(file_path, overwrite = True)

    with open (file_path + "/" + upload.filename) as f:
        fmd5 = md5.md5()
        fmd5.update(f.read( ))
        f.close()

    file_key = fmd5.hexdigest()
    CONFIG['files'][file_key] = {
            "filepath" : file_path,
            "filename" : upload.filename,
            "user" : user,
            "date" : time.asctime(),
            "board" : CONFIG['ports'][port]['board'],
            "comment" : ""
            }
    CONFIG['ports'][port]["file"] = file_path + "/" + upload.filename
    save_cfg()
    return get_terminal_dict(port, user)

@bottle.post("/reset")
@bottle.view("terminal")
def reset():
    port = bottle.request.forms.get("port")
    user = bottle.request.forms.get("user")
    print "***", user, port,CONFIG['ports']
    cmd = CONFIG['ports'][port]["resetcmd"]

    os.system(cmd)
    return get_terminal_dict(port, user)

@bottle.post("/flash")
@bottle.view("terminal")
def flash():
    port = bottle.request.forms.get("port")
    user = bottle.request.forms.get("user")
    file = bottle.request.forms.get("file")
    if not file:
        file = CONFIG['ports'][port]["file"]
    if os.path.isfile(file):
        cmd = CONFIG['ports'][port]["flashcmd"] % {"fw":file, "port":port}
        print "***", cmd
        CONFIG['ports'][port]["file"] = file
        os.system(cmd)
    return get_terminal_dict(port, user)

@bottle.route("/terminal")
@bottle.view("terminal")
def terminal():
    auth = bottle.request.headers.get('Authorization')
    port = bottle.request.query.port
    command = bottle.request.forms.get("file")
    print command
    if auth:
        user, _ = bottle.parse_auth(auth)
        CONFIG['ports'][port]['user'] = user
    if command == "reset":
        cmd = CONFIG['ports'][port]["resetcmd"]
        os.system(cmd)

    return get_terminal_dict(port, user)

@bottle.route("/tdata")
def term_content():
    yield time.asctime() + " > "
    yield "hello "
    yield "world "
    yield " !\n"

@bottle.get('/terminal/ws', apply=[websocket])
def handle_websocket(wsock):
    port = bottle.request.query.port
    user = bottle.request.query.user
    InQueue = Queue()
    WORKERS[port].attach_queue(InQueue)
    try:
        while True:
            with Timeout(0.01, False):  # from gevent import Timeout
                x = wsock.receive()
                print "ws-rx[%s] %s" % (port, repr(x))
                WORKERS[port].TxQueue.put(str(x))
            if not InQueue.empty():
                x = InQueue.get()
                message = ''.join([i if ord(i) < 128 else '?' for i in x])
                print "ws-tx[%s] %s" %(port, repr(message))
                wsock.send(json.dumps({"msg" : message.encode("ascii")}))
            time.sleep(.2)
    except Exception, e:
        WORKERS[port].detach_queue(id(InQueue))
        del(InQueue)
        traceback.print_exc()
    print "exit web sockehandler"

# === main =====================================================================

if __name__ == "__main__":
    print "ENTER MAIN"
    CONFIG = load_cfg("dude_app.json")
    for sp in CONFIG['ports'].keys():
        if sp.find("AMA0")<0:
            WORKERS[sp] = SerialWorker(sp)
            print "started thread for %s" % sp
    print "\n".join(map(str,threading.enumerate()))
    bottle.TEMPLATE_PATH.insert(0, "./dude_app/views")
    bottle.run(host = "localhost",
               debug = 1,
               reloader = 0,
               server = GeventWebSocketServer)
    print "EXIT MAIN"