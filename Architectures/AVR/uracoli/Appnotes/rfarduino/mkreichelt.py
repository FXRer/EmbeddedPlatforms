# create a Reichelt Warenkorb from annoted BOM list(s).
#
# Usage: 
#    python mkreichelt.py parts_reichelt_v1.1.txt
#     - just remove the comments
#    python mkreichelt.py parts_reichelt_v1.1.txt:5
#     - multiply each entry by 5 (for making small series)
#    python mkreichelt.py parts_reichelt_v1.1.txt:5 joergs_wunschlist.txt
#     - combine multiple BOMS
#
# Example for annotated BOM
#---------------------------------------------------------------------------
#  ## $Id$
#  # This is the RFArduino V1.1 Partlist for Reichelt-Elektronik
#  #
#  
#  # (1) 100nF C
#  X7R-G0805 100N;7
#  # (2) 10k R
#  SMD-0805 10,0K;1
#  # (3) 1k R
#  SMD-0805 1,0K;7
#  #
#---------------------------------------------------------------------------
import sys, pprint


## read an annotated file and return a dictionary
# @param fname : name of the annotated file
# @param factor : multiplier
# @param parts : dictionary
def read_anno_warenkorb(fname, factor, parts = {}):
    fin = open(fname,"r")
    for l in fin.readlines():
        # strip comment
        l = l[:l.find("#")].strip()
        if len(l):
            part,cnt = l.split(";")
            cnt = eval(cnt)*factor
            try:
                parts[part] += cnt
            except:
                parts[part] = cnt
    return parts

def make_warenkorb(parts):
    pkeys=parts.keys()
    pkeys.sort()
    for p in pkeys:
        print "%s;%d" % (p.strip(),parts[p])
    

if __name__ == "__main__":
    parts = {}
    for f in sys.argv[1:]:
        f = f.strip()
        try:
            idx = f.rindex(":")
            fname = f[:idx]
            factor = eval(f[idx+1:])
        except:
            factor = 1
            fname = f
        parts = read_anno_warenkorb(fname,factor,parts)
    #pprint.pprint(parts)
    make_warenkorb(parts)
