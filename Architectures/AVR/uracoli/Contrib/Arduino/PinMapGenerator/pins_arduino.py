import json as json
import jinja2, sys
from pprint import pprint as pp
TEMPLATE="pins_arduino.h.jinja"

def filter_comments(fn):
    rv = []
    f = open(fn)
    lines = []
    for l in f.xreadlines():
        #l = l.strip()
        if len(l):
            cm = l.find("#")
            if l > 0:
                lines.append(l[:cm])
            else:
                lines.append(l)
    f.close()
    return "\n".join(lines)

def get_port_list(pins):
    ports = {p.get("port").upper() : p.get("port").upper() for p in pins}
    port_end = ord(max(ports.keys()))
    rv = []
    for p in range(ord("A"), port_end + 1):
        rv.append(ports.get(chr(p)))
    return rv

def split_port_functions(pins):
    for p in pins:
        p['func'] = [x.strip() for x in p['func'].split("/")]

def enumerate_pins(pins):
    pincnt = 0
    for p in pins:
        if p['name'].startswith('d'):
           p['id'] = pincnt
           pincnt += 1

def find_function(pins, function):
    rv = []
    for p in pins:
        a = None
        for f in p['func']:
            if f.startswith(function):
                a = f
        rv.append(a)
    return rv

def count_digital_pins(pins):
    return sum([1 for p in pins if p['name'].startswith("d")])

def count_analog_pins(pins):
    return sum([1 for p in pins if p.get("aname")])

def get_function_to_pin_map(pins):
    rv = {}
    for pin in pins:
        for func in pin['func']:
            rv[func] = pin
    return rv

def get_digitalPinHasPWM(funcs):
    accu = 0
    for func,pin in funcs.items():
        if func.startswith("OC"):
            accu += (1 << pin["id"])

    macro = "((0x%08x & (uint32_t)(1<<p)) != 0) /* %s */" % (accu, bin(accu))
    return macro

def get_analogInputToDigitalPin(pins):
    amap = []
    for pin in pins:
        an = pin.get("aname")
        if an:
            aid = int(an[1:])
            did = pin["id"]
            amap.append( (did, aid) )

    nb_ana_pins = len(amap)
    ana_pin_offsets = set([dp -ap for dp, ap in amap ])
    if len(ana_pin_offsets) != 1:
        raise Exception("only one pin_offset supported")
    rv = "(((p) < %d) ? (p) + %d : -1)" % (nb_ana_pins, ana_pin_offsets.pop())
    return rv

def get_digitalPinToInterrupt(funcs):
    imap = []
    for func, pin in sorted(funcs.items()):
        if func.startswith("INT"):
            int_id = int(func.replace("INT",""))
            pin_id = pin["id"]
            imap.append((pin_id, int_id))
    rv = "(\\\n           " + \
         "\\\n           ".join(["((p)==%d) ? %d : " %(p,i) for p,i in imap]) +\
         "NOT_AN_INTERRUPT)"
    return rv

if __name__ == "__main__":
    boardfile = sys.argv[1]
    fd = filter_comments(boardfile)
    jd = json.loads(fd)
    pin_list = jd.get("pins")
    split_port_functions(pin_list)
    enumerate_pins(pin_list)
    jd['ports'] = get_port_list(pin_list)
    jd['timers'] = find_function(pin_list, "TIMER")
    jd['pwms'] = find_function(pin_list, "OC")
    jd['num_dig_pins'] = count_digital_pins(pin_list)
    jd['num_ana_pins'] = count_analog_pins(pin_list)
    jd['func'] = get_function_to_pin_map(pin_list)
    jd['macro_digitalPinHasPWM'] = get_digitalPinHasPWM(jd['func'])
    jd['macro_analogInputToDigitalPin'] = get_analogInputToDigitalPin(pin_list)
    jd['macro_digitalPinToInterrupt'] = get_digitalPinToInterrupt(jd['func'])
    #pp(jd)
    #pp(pin_list)
    f = open(TEMPLATE)
    t = jinja2.Template(f.read())
    o = t.render(**jd)
    print o
