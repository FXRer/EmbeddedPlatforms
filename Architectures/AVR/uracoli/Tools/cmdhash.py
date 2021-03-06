#
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
# Generator script for command hash code enums.
#
# Usage:
#    python cmdhash.py keyword1 [keyword2 .. [keywordN]]
#
# A enumeration is printed on StdOut.
# Example:
#  python Tools/cmdhash.py foo bar foobar
#  /* Generated by:
#   * Tools/cmdhash.py foo bar foobar
#   */
#  typedef enum {
#       /** Hashvalue for command 'bar' */
#       CMD_BAR = 0x33,
#       /** Hashvalue for command 'foobar' */
#       CMD_FOOBAR = 0x74,
#       /** Hashvalue for command 'foo' */
#       CMD_FOO = 0xea,
#       /** Hashvalue for empty command  */
#       CMD_EMPTY = 0x00,
#  } cmd_hash_t;
#
# If you include this programm, then the function
# get_cmd_hash (shown below) will return 0xea if
# called with get_cmd_hash("foo");
#
# Note: The hash algorithm is very empirically implemented.
# Better implementations with the same or less effort of 
# memory, better collission resistance and/or less computational
# complexity are highly apreciated.
#
# static cmd_hash_t get_cmd_hash(char *cmd)
# {
# cmd_hash_t h, accu;
#     h = 0;
#     while (*cmd != 0)
#     {
#         accu = h & 0xe0;
#         h = h << 5;
#         h = h ^ (accu >> 5);
#         h = h ^ *cmd;
#         h &= 0xff;
#         cmd ++;
#     }
#     return h;
# }
#
import sys, pprint

def shash(cmd):
    h = 0
    for ki in map(ord,cmd):
        accu = h & 0xe0
        h = h << 5
        h = h ^ (accu >> 5)
        h = h ^ ki
        h &= 0xff
    return (h,cmd)

tab = {}
for c in sys.argv[1:]:
    h,c = shash(c.strip())
    if not tab.has_key(h):
        tab[h] = [c]
    else:
        print "#warning \"Warning: collision", h, tab[h], c,"\""
        tab[h].append(c)

tmp = tab.keys()
tmp.sort()
print "/* Generated by:\n * %s\n */" % " ".join(sys.argv)
print "typedef enum {"
for k in tmp:
    v = tab[k][0]
    print "     /** Hashvalue for command '%s' */" % v
    print "     CMD_%s = 0x%02x," % (v.upper(), k)

print "     /** Hashvalue for empty command  */"
print "     CMD_EMPTY = 0x00,"

print "} cmd_hash_t;"

for k in tmp:
    v = tab[k][0]
    print "#define %s_STR \"%s\"" % (v.upper(),v)
    print "#define %s_HSH (0x%02x)" % (v.upper(),k)

print "{"
for k in tmp:
    v = tab[k][0];
    print "    {%s_HSH, %s_STR}," % (v.upper(),v.upper())    
print "};" 
