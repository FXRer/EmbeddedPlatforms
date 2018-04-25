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
# This script builds two release packages from the repository.
#
# Usage:
#   bash Tools/makerelease.sh <tagname>
#
# Example:
#    bash Tools/makerelease.sh
#     - builds the tip of the default branch
#
#    bash Tools/makerelease.sh 0.2.0
#     - builds the package from the repository tagged with 0.2.0
#       (0.2.0 is a local or global tag)
#
#    bash Tools/makerelease.sh xxo_display
#     - builds the package from the tip of the refered branch xxo_display
#

# === Variables ==========================================================
RELBASE=/tmp
RELTAG=${1:-default}
BUILDID=uracoli_build_${RELTAG}_$(hg id --id -r${RELTAG})
BUILDDIR=${RELBASE:-/non-existant}/${BUILDID}
DEVELPKG=uracoli-devel-${RELTAG}

# === Functions ==========================================================
function exit_error
{
    errcode=$1
    shift
    echo =============================
    echo ERROR[$errcode] $*
    echo =============================
    exit $errcode

}


# Make a hg archive,
# hg archive needs to run from hg root directory
CWD=$(pwd)
cd $(hg root)
hg archive -r${RELTAG} -X"Appnotes/" -t zip ${DEVELPKG}.zip ||\
    exit_error 10 "failed packing ${DEVELPKG}.zip"

# === unpack and build packages ============================================
test -d ${BUILDDIR} || mkdir -p ${BUILDDIR}

test -d ${BUILDDIR} || \
    exit_error 20 "could not create ${BUILDDIR}"

unzip -d ${BUILDDIR} ${DEVELPKG}.zip || \
    exit_error 30 "could not unzip ${DEVELPKG}.zip in ${BUILDDIR}"

scons -f SConstruct -C ${BUILDDIR}/${DEVELPKG} version=$RELTAG doc snfdoc ||\
    exit_error 41 "could build doxygen in ${BUILDDIR}/${DEVELPKG}/uracoli"

scons -f SConstruct -C ${BUILDDIR}/${DEVELPKG} version=$RELTAG \
        psrc pdoc parduino15 psniffer || \
    exit_error 42 "could build packages scons in ${BUILDDIR}/${DEVELPKG}/uracoli"

# === copy packages to $(hg root) ============================================
for i in ${BUILDDIR}/${DEVELPKG}/install/*.zip; do
    cp -rfv $i . || \
    exit_error 50 "could copy $i to $(pwd)"
done

echo ======================================================
echo "Created ${DEVELPKG} in $(hg root)"
echo
ls -1sh $(hg root)/*${RELTAG}*.zip
echo
echo "For cleanup remove ${BUILDDIR}"
echo ======================================================
