#   Copyright (c) 2014 ... 2015 Axel Wachtler
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
import ds2reg, boardcfg, zipfile, stat
import ConfigParser
from SCons.Script import *
import pprint
import json, hashlib, copy

# === builders =================================================================

# Template processor for writing transceiver header files
def WriteRadioHeaderFile(target, source, env):
    ds2reg.generate_header(str(source[0]), str(target[0]))
##
# Read and expand a board.cfg file.
# @return dictionary of board classes.
#
# Example:
# @code
# {
#      "board_name" : board_object,
# }
# @endcode
def GetBoardCfg(fname):
    return boardcfg.generate_board_dictionary_expanded(boardcfg.get_boards(fname))

##
# Convert a board dictionary (see GetBoardCfg()) to string, that is e.g. used
# in scons help function.
def ListBoards(boards):
    rv = []
    for b in sorted(boards.keys()):
        board = boards[b]
        if not board.get("isalias"):
           bn = board['name']
           txt = "|".join([bn] + board.get("aliases", "").split()) + ":\n"
           txt += "      %s" % board.get("comment","").replace("\n","\n    ")
           rv.append(  txt )
    return "\n   ".join(rv)

##
# Returns a dictionary of type {"boardname" : "boardmmcu", ...} as pprint
# formated string.
#
# This string is used when generating install/wibo/nodeaddr.py.
#
# @param boards
#        a dictionary of the form {'boardname': <boardcfg.BoardCfg instance>}
#        The dictionary contains all boards defined in the board.cfg file.
#
def GetBoardMmcuTable(boards):
    rv = dict( [(b, boards[b].get("cpu")) for b in boards.keys()])
    return pprint.pformat( rv )

def GetApplicationCfg(fname):
    rv = {}
    cfgp = ConfigParser.RawConfigParser()
    cfgp.read(fname)

    # Read sections
    for sec in cfgp.sections():
        rv[sec] = {"name" : sec}
        app_dict = dict(cfgp.items(sec))
        rv[sec].update(app_dict)
    for app in rv.values():
        app['requires'] = set(app.get('requires','').split(' '))
        app['excludes'] = set(app.get('excludes','').split())
        app['sources'] = app.get('sources','')
        if len(app['sources']):
            app['sources'] = app['sources'].split()
        else:
            app['sources'] = ["%s.c" % app['name']]
        app['headers'] = app.get('headers',"").split()
        app['ingroup'] = app.get('ingroup',"")
    return rv

def GetPackagesCfg(fname):
    rv = {}
    cfgp = ConfigParser.ConfigParser()
    cfgp.read(fname)
    for s in cfgp.sections():
        rv[s] = {
                    "alias"   : s,
                    "name"    : s,
                    "boards" : [],
                    "depends" : [],
                    "files" : [],
                    "relocate" : {},
                    "comment"  : None,
                    "unprefix" : None
                 }
        d = rv[s]
        for k,v in (cfgp.items(s)):
            if k == "depends":
                d[k] = v.replace("\n"," ").split(" ")
            if k == "boards":
                d[k] = v.replace("\n"," ").split(" ")
            elif k == "files":
                d[k] = v.replace("\n"," ").split(" ")
            elif k == "relocate":
                d[k] = dict(map(lambda x: x.split(":"),v.replace("\n"," ").split(" ")))
            elif k == "comment":
                v = v.strip()
                if len(v) > 0:
                    d[k] = v
                else:
                    d[k] = None
            else:
                d[k] = v.strip()
    return rv

##
# return a dictionary of all toolchains, defined in a given toolchain config
# file <fname>.
#
# @param fname - name of the toolchain config file.
#
def GetToolChainsCfg(fname):
    rv = {}
    cfgp = ConfigParser.RawConfigParser()
    cfgp.read(fname)
    for s in cfgp.sections():
        rv[s] = dict(cfgp.items(s))
    return rv

##
# Assign the scons build variables
#
# @param board - board dictionary as read from board.cfg
# @param env   - scons build environment, e.g. derived from com.Clone()
# @param tc    - specific toolchain dictionary as read from toolchains.cfg
#                e.g. ARM or AVR configuration for CC, LD, ...
#
def SetToolChain(board, env ,tc):
    for k in tc.keys():
        ku = k.upper()
        if k in ["CC", "LD", "AR", "OBJCOPY", "RANLIB"]:
            env[ku] = tc[k].strip()
        else:
            env[ku] = [o % board for o in tc.get(k,"").split()]

##
# Get a dictionary with build flags.
# @param env
#        Build environment to operate on.
# @param flagtype
#        Flag suffix to be used.
# @param flags
#        List of flag names (default: "CCFLAGS", "LINKFLAGS", "LIBS")
#
# @return Dictionary with current build flags.
#
# The function first tries to get "<FLAG>_<FLAGTYPE>" and
# if this is not successful, it returns "<FLAG>" as default/fallback value.
#
def GetBuildFlags(env, flagtype, flags = ["CCFLAGS", "LINKFLAGS", "LIBS"]):
    rv = {}
    ft = flagtype
    for k in flags:
        rv[k] = env.get("%s_%s" % (k, ft), env[k])
    return rv
##
# compare the "cpu" defined in the board dictionary with the mcus defined in
# the toolchain dictionary.
# e.g. atmega1281 is found in "atmega" of the AVR toolchain or
#      samr21 is found in "sam" of the ARM toolchain.
#
# @param board - board dictionary as read from board.cfg
# @param tcs   - toolchain dictionary as read from toolchains.cfg
#
def FindToolChain(board, tcs):
    rv = None
    for k,v in tcs.items():
        for m in v["mcus"].split():
            if board["cpu"].find(m) == 0:
                rv = k
                break
    return rv

def GetDocsCfg(fname):
    rv = {}
    cfgp = ConfigParser.RawConfigParser()
    cfgp.read(fname)
    for s in cfgp.sections():
        rv[s] = dict(cfgp.items(s))
    return rv

def WriteBoardHeaderFile(target, source, env):
    boardcfg.write_board_header_file(
                env['BOARDS'], str(target[0]),
                boardnames = env.get('SELECTED_BOARDS', None),
                preselectedboard = env.get('PRESELECTED_BOARD', None))

def TextTransformer(target, source, env):
    # read template file
    f = open(str(source[0]), "r")
    in_txt = f.read()
    f.close()
    # do transform
    fo = open(str(target[0]), "w")
    fo.write(in_txt % env["TEXT_PARAMS"])
    fo.close()

def WriteBoardDoxFile(target, source, env):
    rv = boardcfg.boards_to_dox(env['BOARDS'])
    fo = open(str(target[0]), "w")
    fo.write(rv)
    fo.close()

def UpdateArduinoPackageJson(target, source, env):
    # load input package structure
    with open(str(source[0]), "r") as jin:
        jd = json.loads(jin.read())
        jin.close()

    # retrieve package information
    pkgfile = str(env.File(env["pkg"]))
    with open(pkgfile) as pf:
        pkg_data = pf.read()
        pkg_size = len(pkg_data)
        pkg_fn = os.path.basename(pkgfile)
        pf.close()
    md = hashlib.sha256()
    md.update(pkg_data)

    # create package data structure
    pkg_data = copy.deepcopy(jd['packages'][0])
    pkg_data["platforms"][0]["archiveFileName"] = pkg_fn
    pkg_data["platforms"][0]["checksum"] = "SHA-256:" + md.hexdigest()
    pkg_data["platforms"][0]["size"] = "%d" % pkg_size
    pkg_data["platforms"][0]["version"] = env["VERSION"]
    pkg_data["platforms"][0]["url"] = env["PKG_URL"] + "/" + pkg_fn
    jd['packages'].insert(0, pkg_data)

    with open(str(target[0]), "w") as jout:
        jout.write(json.dumps(jd, indent = 2, sort_keys = True))
        jout.close()

##
# This the Zipfile Builder Function for generating
# arbitrary Zipfiles.
#
# The function walk_tree is used to traverse any directory
# given and recursively adding the files therein.
#
# @param target
#        Name of the Zipfile
# @param source
#        List of the source files packed into the archive
# @param env
#        From env the following keys will be used:
#        - PREFIX :
#          prefix that is prepended to each file packed in the archiv.
#        - UNPREFIX :
#          prefix that is removed from source file path.
#          It must not start with "/"
#        - RELOCATE :
#          Dicitionry that is used for translating file and directory names.
#        - COMMENT :
#          maximum 65535 byte long string that is added as comment.
#
def CreateZipFile(target, source, env):
    target = str(target[0])
    print "Zip", target
    sources = map(str,source)
    prefix = env.get("PREFIX","")
    relocate = env.get("RELOCATE",{})
    unprefix = env.get("UNPREFIX", None)
    comment = env.get("COMMENT",None)
    # generate list of sourcefiles and its name mappings in zipfile
    ziplist = []
    for src in sources:
        for indirname, filelst in walk_tree(src):
            # normalize dirname to have / seperator to do dictionary lookup
            indn = indirname.replace("\\","/")
            outdirname = relocate.get(indn, indn)
            if unprefix:
                outdirname = os.path.relpath(outdirname, unprefix)
            for infilename in filelst:
                outfilename = relocate.get(infilename,infilename)
                infn = os.path.join(indirname,infilename)
                outfn = os.path.join(prefix, outdirname, outfilename)
                ziplist.append( (infn, outfn) )

    # write zipfile
    zf = zipfile.ZipFile(target,"w")
    for fn_in, fn_out in ziplist:
        zf.write(fn_in, fn_out, zipfile.ZIP_DEFLATED)
    if comment != None:
        zipfile.ZipFile.comment = comment
    zf.close()

def GetLibMakeRules(env, boardnames = None):
    boards = env["BOARDS"]
    if not boardnames:
        boardnames = sorted(boards.keys())
    all_rule = "all: " + " ".join(boardnames)
    all_rule += "\n\nlist:\n"
    for bn in boardnames:
        all_rule += "\t @echo '  %-16s : %s'\n" % (bn, boards[bn].get("comment").split("\n")[0])

    rules = []
    for bn in boardnames:
        board = boards[bn]
        r = "%(name)s:\n\t$(MAKE) TC=%(toolchain)s BOARD=%(name)s "\
            "MCU=%(cpu)s F_CPU=%(f_cpu)s "\
            "$(TARGETS)\n" % boards[bn]
        rules.append(r)
    board_rules = "\n".join(rules)
    return all_rule, board_rules

def GetAppMakeRules(env, appname, appgroup = "APPLICATIONS", boardnames = None):
    boards = env["BOARDS"]
    app = env[appgroup][appname]
    if not boardnames:
        boardnames = sorted(boards.keys())
    supported_boards = []
    for bn in boardnames:
        if app['requires'].issubset(boards[bn]['provides']) and \
            not bn in app.get("excludes",[]):
            supported_boards.append(bn)
    all_rule = "all: " + " ".join(supported_boards)
    all_rule += "\n\nlist:\n"
    for bn in supported_boards:
        all_rule += "\t @echo '  %-16s : baudrate=% 6s, %s'\n" % (bn,
                                    boards[bn].get("baudrate", "-"),
                                    boards[bn].get("comment").split("\n")[0])
    rules = []
    for bn in supported_boards:
        board = boards[bn]
        r = "%(name)s:\n\t$(MAKE) -f $(CURRENT_MAKEFILE) TC=%(toolchain)s BOARD=%(name)s "\
            "MCU=%(cpu)s F_CPU=%(f_cpu)s "\
            "$(TARGETS)\n\t"\
            "@echo\n\t@echo %(name)s : baudrate=%(baudrate)s-8-N-1\n\t@echo\n" % boards[bn]
        rules.append(r)
    board_rules = "\n".join(rules)
    return all_rule, board_rules

def GetMakeToolDefs(env, flagtype = None):
    rv = "# toolchain settings\n"
    fmt = "   CC=%(cc)s\n"\
          "   AR=%(ar)s\n"\
          "   RANLIB=%(ranlib)s\n"\
          "   OBJCOPY=%(objcopy)s\n\n"\
          "   CCFLAGS+=%(ccflags)s\n"\
          "   LDFLAGS+=%(linkflags)s\n"\
          "   LDFLAGS+=$(addprefix -l, %(libs)s)\n"

    mkvars = dict(cpu = "$(MCU)", name = "$(BOARD)", f_cpu="$(F_CPU)", bootoffset = "$(BOOTOFFSET)")

    for tcn, tc in env["TOOLCHAINS"].items():
        x = {}
        x.update(tc)
        build_flags = GetBuildFlags(tc, flagtype, flags=["ccflags", "linkflags","libs"])
        build_flags = {k.lower() : v.replace("\n", " ") for k,v in build_flags.items()}
        x.update(build_flags)
        rv += "ifeq ($(TC), %s)\n" % tcn
        rv += (fmt % x) % mkvars
        rv += "else "
    rv += "\n"
    d = {k : "@echo undefined %s for $(TC)" % k for k in "cc ar ranlib objcopy ccflags linkflags libs".split(" ") }
    rv += fmt % d
    rv += "endif\n"
    return rv

# === functions ===============================================================
#
# This function comes from:
# http://code.activestate.com/recipes/200131-recursive-directory-listing-in-html/
def walk_tree(top = ".", depthfirst = True):
    if os.path.isdir(top):
        names = os.listdir(top)
        if not depthfirst:
            yield top, names
        for name in names:
            try:
                st = os.lstat(os.path.join(top, name))
            except os.error:
                continue
            if stat.S_ISDIR(st.st_mode):
                for (newtop, children) in walk_tree(os.path.join(top, name), depthfirst):
                    yield newtop, children
        if depthfirst:
            yield top, names
    else:
        dn, fn = os.path.split(top)
        yield dn, [fn]


##
# This function registers all builders in ListOfBuilder in the given environment.
#
# @param env
#       Scons environment
def add_builders(env):
    global ListOfBuilders
    for bd in ListOfBuilders:
        env['BUILDERS'][bd['action'].__name__] = Builder(**bd)

# === globals =================================================================
ListOfBuilders = [
        dict(action = WriteRadioHeaderFile),
        dict(action = WriteBoardHeaderFile),
        dict(action = TextTransformer),
        dict(action = CreateZipFile),
        dict(action = WriteBoardDoxFile),
        dict(action = UpdateArduinoPackageJson),
    ]

