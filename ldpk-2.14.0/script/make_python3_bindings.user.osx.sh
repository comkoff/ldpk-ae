#!/bin/bash

################################################################
# Run from anywhere. The script assumes that it is located     #
# at its normal place inside the LDPK-directory so that        #
# source and header directories can be reached via relative    #
# paths.                                                       #
################################################################

################################################################
# Inside SDV, test with                                        #
# env -i BUILDER_OSX_LDPK=... PATH=/usr/bin \		       #
#	./make_python3_bindings.user.osx.sh		       #
# Last tested: 2024-03-11				       #
################################################################

# fancy colors
     red="\033[1;31m"
   green="\033[1;32m"
  yellow="\033[1;33m"
    blue="\033[1;34m"
 magenta="\033[1;35m"
    cyan="\033[1;36m"
   white="\033[0;37m"
    bold="\033[1m"
    grey="\033[38;2;127;127;127m"
  orange="\033[1;38;2;255;127;0m"
     end="\033[0m"
   reset="\033[0m"
    lime="\033[1;38;2;127;255;0m"

echo -e ${white}${bold}"#-------------- make_python3_bindings.user.osx.sh ------------#"${end}

script=`realpath $0`
basedir=`dirname $script`
# The root directory of the LDPK.
ldpk=`realpath $basedir/..`

################################################################
# Insert your favourite compiler here                          #
################################################################
if [ $BUILDER_OSX_LDPK == "applejack" ]; then
	CC="clang++"
else
	CC="clang++"
fi

#echo -ne ${white}${bold}"# Compiler:" ${end}
#$CC --version

################################################################
# Options for position independent code and dynamic libs may   #
# vary from compiler version to compiler version...            #
################################################################
OPT="-O2 -fPIC --std=c++11"
SHARED="-dynamiclib"

################################################################
# Python version and base directory                            #
# Maybe /usr or /usr/local                                     #
################################################################
if [ -n "$1" ]; then
# Invoke the script e.g. like make_python3_bindings... 3.9 /usr/local
	PYTHON_VERSION=$1
	PYTHON_BASEDIR=$2
# When we build the python bindings for the LDPK:
	INCPYTHON="-I${PYTHON_BASEDIR}/include/python${PYTHON_VERSION}"
	LIBPYTHON="-L${PYTHON_BASEDIR}/lib -lpython${PYTHON_VERSION}"
else
# In case you invoke this script without parameters:
# The default (in my case) is Python 3.8 in /usr/lib
# and /usr/include. Please edit as needed,
	PYTHON_VERSION="3.8"
	PYTHON_BASEDIR="/usr/local"
# This looks complicated. Please find out if paths are correct on your system.
# Use this for the original python shipped with your macOS.
	INCPYTHON="-I${PYTHON_BASEDIR}/include/python${PYTHON_VERSION}"
	LIBPYTHON="-L${PYTHON_BASEDIR}/lib/python${PYTHON_VERSION}/config-${PYTHON_VERSION}-darwin/ -lpython${PYTHON_VERSION}"
fi
echo -e "${bold}Python version: ${lime}${PYTHON_VERSION}$reset"

################################################################
# Include and library options for LDPK-stuff                   #
################################################################
INCPATH="-I ../../include"

################################################################
# Target directory, will be created by the script              #
################################################################
if [ -n "$BUILDER_OSX_LDPK" ]; then
# We use this script inside sdv to build the LDPK.
	TARGET_DIR="${ldpk}/python3/osx/python${PYTHON_VERSION}"
else
# By default your module is written to /tmp, but feel free to change.
	TARGET_DIR="/tmp/osx/python${PYTHON_VERSION}"
fi

mkdir -p $TARGET_DIR

cd $ldpk/source/ldpk
set -x
$CC $OPT $SHARED tde4_lens_distortion_plugins.module.3.C $INCPYTHON $INCPATH $LIBPYTHON -o $TARGET_DIR/lens_distortion_plugins.so
