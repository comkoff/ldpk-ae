#!/bin/csh -x

# Most recent test: 2022-08-31

set echo

set basedir=`dirname $PWD/$0`

################################################################
# See documentation for appropriate version of gcc/clang/llvm  #
################################################################
set CC="g++"

################################################################
# Think about this: is it correct for your version of Nuke?    #
# How about compiling according to C++14 standard? -std=c++14  #
# If you compile by means of this script you might want to set #
# the target architecture, like                                #
# --target=x86_64-apple-darwin20.6.0                           #
# Suppress some warnings: -Wno-deprecated-register             #
################################################################
set OPT="-O2 -fPIC -std=c++14 -stdlib=libc++ -Wno-deprecated-register --target=x86_64-apple-darwin18.7.0"

set SUFFIX="dylib"
set SHARED="-dynamiclib"
set INC_LDPK="-I $basedir/../include"
set LIB_NUKE="-lDDImage"

################################################################
# Switch to ldpk source directory                              #
################################################################
cd $basedir/../source/ldpk

unset echo

################################################################
# Please edit these according to your needs:                   #
################################################################
set NUKE_VERSION="13.0"
set NUKE_V_SUFFIX="v1"
# On MacOS this the default location of Nuke as suggested by The Foundry's installer.
set INC_NUKE="-I /Applications/Nuke$NUKE_VERSION$NUKE_V_SUFFIX/Nuke$NUKE_VERSION$NUKE_V_SUFFIX.app/Contents/MacOS/include"
set LIBDIR_NUKE="-L/Applications/Nuke$NUKE_VERSION$NUKE_V_SUFFIX/Nuke$NUKE_VERSION$NUKE_V_SUFFIX.app/Contents/MacOS"

################################################################
# your results here please:                                    #
################################################################
set TARGET_DIR="/tmp/lib/nuke/osx/Nuke$NUKE_VERSION"
mkdir -p $TARGET_DIR
set echo

# SDV LD-Plugins
$CC $OPT $SHARED nuke_ld_3de4_radial_standard_degree_4.C				nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Radial_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_standard_degree_4.C				nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Anamorphic_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_rescaled_degree_4.C				nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Anamorphic_Rescaled_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_standard_degree_6.C				nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Anamorphic_Standard_Degree_6.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_rescaled_degree_6.C				nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Anamorphic_Rescaled_Degree_6.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_stabilizer_degree_6.C				nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Anamorphic_Stabilizer_Degree_6.$SUFFIX

# Experimental stuff, if you like
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_degree_8.experimental.C	nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Radial_Fisheye_Equidistant_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equisolid_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Radial_Fisheye_Equisolid_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_orthographic_degree_8.experimental.C	nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Radial_Fisheye_Orthographic_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_stereographic_degree_8.experimental.C	nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Radial_Fisheye_Stereographic_Degree_8.$SUFFIX

# Deprecated. Classic and Fisheye are still in use and will be for quite some time 
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_degree_6.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Anamorphic_Degree_6.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_degree_8.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Radial_Fisheye_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de_classic_ld_model.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE_Classic_LD_Model.$SUFFIX

# Toymodels
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_homomorphic_degree_2.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Anamorphic_Homomorphic_Degree_2.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_homomorphic_degree_2.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Radial_Homomorphic_Degree_2.$SUFFIX

# Original
$CC $OPT $SHARED nuke_ld_3de4_brown_conrady_degree_6_2.C				nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $TARGET_DIR/LD_3DE4_Brown_Conrady_Degree_6_2.$SUFFIX

unset echo
echo "**********************************************************"
echo "Plugins should now be in $TARGET_DIR."
echo "Please copy them to your Nuke plugins directory."
echo "All plugin names start with 'LD_3DE...'."
echo "**********************************************************"
