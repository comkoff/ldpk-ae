#!/bin/bash
# internal/sdv: execute on $BUILDER_OSX_LDPK.

################################################################
# Run from anywhere. The script assumes that it is located     #
# at its normal place inside the LDPK-directory so that        #
# source and header directories can be reached via relative    #
# paths.                                                       #
################################################################
basedir=`dirname $PWD/$0`

################################################################
# Inside SDV, test with                                        #
# env -i PATH=/usr/bin \                                       #
#	LDPK=$ldpk                                             #
#	./make_ldpk.osx.sh				       #
# Last tested: 2023-07-25				       #
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
    lime="\033[1;38;2;127;255;0m"
     end="\033[0m"
   reset="\033[0m"

echo -e ${white}${bold}"#------------------ make_ldpk.osx.sh ---------------------------#"${end}
echo "basedir is '" $basedir "'."
################################################################
# Insert your favourite compiler here                          #
################################################################
if [ "$BUILDER_OSX_LDPK" == "applejack" ]; then
	CC="g++"
else
	CC="g++"
fi

################################################################
# Options for position independent code and dynamic libs may   #
# vary from compiler version to compiler version...            #
# Also check if the target architecture is arm64 or x86_64     #
################################################################
SHARED="-dynamiclib"
OPT="-std=c++17 -O2 -fPIC --target=arm64-apple-darwin20.2.0"
LIBTOOL="ar"
# Maybe replace by crD in case of trouble
LIBTOOL_OPTIONS="cruv"

SUFFIX="so"

echo -e ${red}"#------------------ this and that ------------------------------#"${end}
# New in version 1.1: compile helper classes into a library.
cd ../source/ldpk
INCPATH="-I ../../include"

set -x

$CC $OPT $INCPATH ldpk_model_parser.C		-c -o osx/ldpk_model_parser.o
$CC $OPT $INCPATH ldpk_plugin_loader.C		-c -o osx/ldpk_plugin_loader.o
$CC $OPT $INCPATH ldpk_table_generator.C	-c -o osx/ldpk_table_generator.o

set +x
rm -f ../../lib/osx/libldpk.a
$LIBTOOL $LIBTOOL_OPTIONS ../../lib/osx/libldpk.a osx/ldpk_model_parser.o osx/ldpk_plugin_loader.o osx/ldpk_table_generator.o

cd $basedir/../test
INCPATH="-I ../include"
TARGET_DIR=../bin/osx
set -x

$CC $OPT test_lookup_table_iter.C		$INCPATH				-o $TARGET_DIR/test_lookup_table_iter
$CC $OPT test_plugin_loader.C			$INCPATH ../lib/osx/libldpk.a -ldl	-o $TARGET_DIR/test_plugin_loader
$CC $OPT tde4_plugin_info.C			$INCPATH -I ../.. -D_SCI_BUS_64BIT ../lib/osx/libldpk.a -ldl		-o $TARGET_DIR/tde4_plugin_info

set +x
cd $basedir/../source/ldpk
INCPATH="-I ../../include"
TARGET_DIR="../../compiled/ldpk/osx/lib"

# Functions for compiling plugins. Both expect a single argument,
# the source filename without suffix. We execute the compile command
# in a subshell, for cosmetic reasons only. We don't want to see
# "set +x" in the terminal.
compile(){
	( set -x;$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH $1.C -o $TARGET_DIR/$1.$SUFFIX )
}
compile_exp(){
	( set -x;$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH $1.experimental.C -o $TARGET_DIR/$1.$SUFFIX )
}

echo -e ${cyan}"#------------------ standard -----------------------------------#"${end}
# Model "3DE4 Radial - Standard, Degree 4"
compile tde4_ldp_radial_standard_degree_4
# Model "3DE4 Radial - Stabilizer, Degree 6"
compile tde4_ldp_radial_stabilizer_degree_6
# Model "3DE4 Anamorphic - Standard, Degree 4"
compile tde4_ldp_anamorphic_standard_degree_4
# Model "3DE4 Anamorphic - Rescaled, Degree 4"
compile tde4_ldp_anamorphic_rescaled_degree_4
# Model "3DE4 Anamorphic - Standard, Degree 6"
compile tde4_ldp_anamorphic_standard_degree_6
# Model "3DE4 Anamorphic - Rescaled, Degree 6"
compile tde4_ldp_anamorphic_rescaled_degree_6
# Model "3DE4 Anamorphic - Stabilizer, Degree 6"
compile tde4_ldp_anamorphic_stabilizer_degree_6

echo -e ${magenta}"#------------------ deprecated----------------------------------#"${end}
# Model "3DE4 Radial - Fisheye, Degree 8"
compile tde4_ldp_radial_fisheye_degree_8
# Model "3DE Classic LD Model"
compile tde4_ldp_classic_ld_model

echo -e ${yellow}"#------------------ experimental--------------------------------#"${end}
# Model "3DE4 Radial - Fisheye, Equidistant, Degree 8"
compile_exp tde4_ldp_radial_fisheye_equidistant_degree_8
# Model "3DE4 Radial - Fisheye, Equisolid, Degree 8"
compile_exp tde4_ldp_radial_fisheye_equisolid_degree_8
# Model "3DE4 Radial - Fisheye, Orthographic, Degree 8"
compile_exp tde4_ldp_radial_fisheye_orthographic_degree_8
# Model "3DE4 Radial - Fisheye, Stereographic, Degree 8"
compile_exp tde4_ldp_radial_fisheye_stereographic_degree_8

echo -e ${orange}"#------------------ toymodels ----------------------------------#"${end}
#--------------------------------------------------------------#
# Toymodels, mainly internal use, for doing experiments        #
# and learning                                                 #
#--------------------------------------------------------------#
# Model "3DE4 Radial - Homomorphic, Degree 2"
compile_exp tde4_ldp_radial_homomorphic_decentered_degree_2
# Model "3DE4 Anamorphic - Homomorphic, Degree 2"
compile_exp tde4_ldp_anamorphic_homomorphic_degree_2

echo -e ${lime}"#------------------ original -----------------------------------#"${end}
# The original Brown-Conrady
compile tde4_ldp_brown_conrady_degree_6_2


