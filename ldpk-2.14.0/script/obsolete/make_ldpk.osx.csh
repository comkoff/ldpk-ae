#!/bin/tcsh -x
# internal/sdv: execute on $BUILDER_OSX_LDPK.

unset echo
if (`hostname` != $BUILDER_OSX_LDPK) then
	echo "run on $BUILDER_OSX_LDPK"
	echo 'e.g. ssh ldpk@$BUILDER_OSX_LDPK "cd $LDPK;./make_ldpk.osx.csh"'
	exit 1
endif

set basedir=`dirname $PWD/$0`
set CC="g++"
# Usually the suffix for dynamic objects is .dylib
# but this works as well, and some parts of our software
# do not scan for .dylib...
set SUFFIX="so"
set SHARED="-dynamiclib"
set OPT="-std=c++17 -O2 -fPIC --target=arm64-apple-darwin20.2.0"

echo "#------------------ this and that ------------------------------#"
# New in version 1.1: compile helper classes into a library.
cd $LDPK/source/ldpk
set INCPATH="-I ../../include"
set echo

$CC $OPT $INCPATH ldpk_model_parser.C		-c -o osx/ldpk_model_parser.o
$CC $OPT $INCPATH ldpk_plugin_loader.C		-c -o osx/ldpk_plugin_loader.o
$CC $OPT $INCPATH ldpk_table_generator.C	-c -o osx/ldpk_table_generator.o
rm -f ../../lib/osx/libldpk.a
ar cruv ../../lib/osx/libldpk.a osx/ldpk_model_parser.o osx/ldpk_plugin_loader.o osx/ldpk_table_generator.o

unset echo
cd $basedir/../test
set INCPATH="-I ../include"
set TARGET_DIR=$LDPK/bin/osx
set echo

$CC $OPT test_lookup_table_iter.C		$INCPATH							-o $TARGET_DIR/test_lookup_table_iter
$CC $OPT test_plugin_loader.C			$INCPATH ../lib/osx/libldpk.a -ldl				-o $TARGET_DIR/test_plugin_loader
$CC $OPT tde4_plugin_info.C			$INCPATH -I ../.. -D_SCI_BUS_64BIT ../lib/osx/libldpk.a -ldl	-o $TARGET_DIR/tde4_plugin_info

unset echo
cd $basedir/../source/ldpk
set INCPATH="-I ../../include"
set TARGET_DIR="../../compiled/ldpk/osx/lib"

echo "#------------------ standard -----------------------------------#"
set echo
# Model "3DE4 Anamorphic - Standard, Degree 4"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_standard_degree_4.C -o $TARGET_DIR/tde4_ldp_anamorphic_degree_4_rotate_squeeze_xy.$SUFFIX
# Model "3DE4 Anamorphic - Rescaled, Degree 4"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_rescaled_degree_4.C -o $TARGET_DIR/tde4_ldp_anamorphic_degree_4_rotate_squeeze_xy_rescaled.$SUFFIX
# Model "3DE4 Anamorphic - Standard, Degree 6"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_standard_degree_6.C -o $TARGET_DIR/tde4_ldp_anamorphic_degree_6_rotate_squeeze_xy.$SUFFIX
# Model "3DE4 Anamorphic - Rescaled, Degree 6"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_rescaled_degree_6.C -o $TARGET_DIR/tde4_ldp_anamorphic_degree_6_rotate_squeeze_xy_rescaled.$SUFFIX
# Model "3DE4 Radial - Standard, Degree 4"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_standard_degree_4.C -o $TARGET_DIR/tde4_ldp_radial_standard_degree_4.$SUFFIX

unset echo
echo "#------------------ deprecated----------------------------------#"
set echo
# Model "3DE4 Anamorphic - Degree 6"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_degree_6.C -o $TARGET_DIR/tde4_ldp_anamorphic_degree_6.$SUFFIX
# Model "3DE4 Radial - Fisheye, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_degree_8.C -o $TARGET_DIR/tde4_ldp_radial_degree_8.$SUFFIX
# Model "3DE Classic LD Model"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_classic_ld_model.C -o $TARGET_DIR/tde4_ldp_classic_ld_model.$SUFFIX

unset echo
echo "#------------------ experimental--------------------------------#"
set echo
# Model "3DE4 Radial - Fisheye, Equidistant, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_equidistant_degree_8.experimental.C -o $TARGET_DIR/tde4_ldp_radial_fisheye_equidistant_degree_8.$SUFFIX
# Model "3DE4 Radial - Fisheye, Equisolid, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_equisolid_degree_8.experimental.C -o $TARGET_DIR/tde4_ldp_radial_fisheye_equisolid_degree_8.$SUFFIX
# Model "3DE4 Radial - Fisheye, Orthographic, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_orthographic_degree_8.experimental.C -o $TARGET_DIR/tde4_ldp_radial_fisheye_orthographic_degree_8.$SUFFIX
# Model "3DE4 Radial - Fisheye, Stereographic, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_stereographic_degree_8.experimental.C -o $TARGET_DIR/tde4_ldp_radial_fisheye_stereographic_degree_8.$SUFFIX
# Model "3DE4 Anamorphic - Standard - Stabilizer, Degree 6"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_standard_stabilizer_degree_6.experimental.C -o $TARGET_DIR/tde4_ldp_anamorphic_standard_stabilizer_degree_6.$SUFFIX
# Model "3DE4 Anamorphic - Rescaled - Stabilizer, Degree 6"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_rescaled_stabilizer_degree_6.experimental.C -o $TARGET_DIR/tde4_ldp_anamorphic_rescaled_stabilizer_degree_6.$SUFFIX

unset echo
echo "#------------------ toymodels ----------------------------------#"
set echo
#-------------------------------------------------------------#
# Toymodels, mainly internal use, for doing experiments       #
# and learning                                                #
#-------------------------------------------------------------#
# Model "3DE4 Radial - Homomorphic, Degree 2"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_homomorphic_decentered_degree_2.experimental.C -o $TARGET_DIR/tde4_ldp_radial_homomorphic_decentered_degree_2.$SUFFIX
# Model "3DE4 Anamorphic - Homomorphic, Degree 2"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_homomorphic_degree_2.experimental.C -o $TARGET_DIR/tde4_ldp_anamorphic_homomorphic_degree_2.$SUFFIX

unset echo
echo "#------------------ original------------------------------------#"
set echo
# Model "3DE4 Radial - Brown-Conrady, Degree 6/2"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_brown_conrady_degree_6_2.C -o $TARGET_DIR/tde4_ldp_brown_conrady_degree_6_2.$SUFFIX
