#!/usr/bin/env python2
#
# This wrapper installs Radiofarogw package locally.
#

import pip, sys, glob, os
ardutools_path = os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), "ardutools")
if not os.path.exists(ardutools_path):
    pkgname = "radiofarogw"
    cmd = "install "\
          "--isolated "\
          "--prefix %s "\
          "--extra-index-url https://testpypi.python.org/pypi"\
          " %s" % (ardutools_path, pkgname)
    print cmd
    pip.main(cmd.split())

site_packages = glob.glob(os.path.join(ardutools_path, "*", "*", "site*"))[0]
sys.path.insert(0, site_packages)
GW_ROOT = os.path.join(ardutools_path,"radiofaro")
execfile(os.path.join(ardutools_path, "bin", "radiofarogw.py"))
