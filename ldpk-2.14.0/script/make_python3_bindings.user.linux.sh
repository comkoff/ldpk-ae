#!/bin/bash

################################################################
# Run from anywhere. The script assumes that it is located     #
# at its normal place inside the LDPK-directory so that        #
# source and header directories can be reached via relative    #
# paths.                                                       #
################################################################

################################################################
# Inside SDV, test with                                        #
# env -i BUILDER_LINUX_LDPK=... PATH=/usr/bin \		       #
#	./make_python3_bindings.user.linux.sh		       #
# Last tested: 2025-03-22				       #
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

echo -e "${white}${bold}#-------------- make_python3_bindings.user.linux.sh ------------#${end}"

script=`realpath -s "$0"`
basedir=`dirname "$script"`
# The root directory of the LDPK.
ldpk=`realpath "$basedir/.."`

################################################################
# Insert your favourite compiler here                          #
################################################################
if [ "$BUILDER_LINUX_LDPK" == "gilgamesh" ]; then
	CC="g++"
elif [ "$BUILDER_LINUX_LDPK" == "bilbo" ]; then
	CC="g++-7.sh"
else
	CC="g++"
fi

echo -ne "${white}${bold}# Compiler: ${end}"
"$CC" --version

################################################################
# Options for position independent code and dynamic libs may   #
# vary from compiler version to compiler version...            #
################################################################
OPT="-O2 -fPIC --std=c++11"
SHARED="-shared"

################################################################
# Python version and base directory                            #
# Maybe /usr or /usr/local                                     #
################################################################
if [ -n "$1" ]; then
# Invoke the script e.g. like make_python3_bindings... 3.9 /usr/local
	PYTHON_VERSION="$1"
	PYTHON_BASEDIR="$2"
else
# In case you invoke this script without parameters:
# The default (in my case) is Python 3.8 in /usr/lib
# and /usr/include. Please edit as needed,
	PYTHON_VERSION="3.10"
	PYTHON_BASEDIR="/usr"
fi
echo -e "${bold}Python version: ${lime}${PYTHON_VERSION}$reset"

################################################################
# Include and library options for Python and LDPK-stuff        #
################################################################
INCPYTHON="-I${PYTHON_BASEDIR}/include/python${PYTHON_VERSION}"
LIBPYTHON="-L${PYTHON_BASEDIR}/lib -lpython${PYTHON_VERSION}"
INCPATH="-I ../../include"

################################################################
# Target directory, will be created by the script              #
################################################################
if [ -n "$BUILDER_LINUX_LDPK" ]; then
# We use this script inside sdv to build the LDPK.
	TARGET_DIR="${ldpk}/python3/linux/python${PYTHON_VERSION}"
else
# By default your module is written to /tmp, but feel free to change.
	TARGET_DIR="/tmp/linux/python${PYTHON_VERSION}"
fi

mkdir -p "$TARGET_DIR"

cd "$ldpk"/source/ldpk
set -x
$CC $OPT $SHARED tde4_lens_distortion_plugins.module.3.C $INCPYTHON $INCPATH $LIBPYTHON -o "$TARGET_DIR"/lens_distortion_plugins.so
