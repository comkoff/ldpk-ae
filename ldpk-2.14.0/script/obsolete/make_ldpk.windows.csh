#!/bin/tcsh -x
# internal/sdv: execute on "helboy2"

unset echo
if (`hostname` != "demon") then
	echo "${yellow}run on demon{end}"
	echo 'e.g. ssh ldpk@$demon "cd $LDPK;./make_ldpk.windows.csh"'
	exit 1
endif

set basedir=`dirname $PWD/$0`
set CC="x86_64-w64-mingw32-g++-posix"
set SUFFIX="so"
set SHARED="-shared"
set OPT="-O2 -fPIC"
set LIBTOOL="ar"
set LIBTOOL_OPTIONS="crv"


echo ${red}"#------------------ this and that ------------------------------#"${end}
# New in version 1.1: compile helper classes into a library.
cd $LDPK/source/ldpk
set INCPATH="-I ../../include -I $INCPATH_DL_SDV"
set echo

$CC $OPT $INCPATH ldpk_model_parser.C		-c -o windows/ldpk_model_parser.o
$CC $OPT $INCPATH ldpk_plugin_loader.C		-c -o windows/ldpk_plugin_loader.o
$CC $OPT $INCPATH ldpk_table_generator.C	-c -o windows/ldpk_table_generator.o
rm -f ../../lib/windows/libldpk.a
$LIBTOOL $LIBTOOL_OPTIONS ../../lib/windows/libldpk.a windows/ldpk_model_parser.o windows/ldpk_plugin_loader.o windows/ldpk_table_generator.o

unset echo
cd $basedir/../test
set INCPATH="-I ../include -I $INCPATH_DL_SDV"
set LIBPATH="-L $LIBPATH_DL_SDV"
set TARGET_DIR=$LDPK/bin/windows
set echo

$CC $OPT test_lookup_table_iter.C		$INCPATH						-o $TARGET_DIR/test_lookup_table_iter.exe
$CC $OPT test_plugin_loader.C			$INCPATH ../lib/windows/libldpk.a $LIBPATH -ldl		-o $TARGET_DIR/test_plugin_loader.exe
$CC $OPT tde4_plugin_info.C			$INCPATH -I ../.. -D_SCI_BUS_64BIT ../lib/windows/libldpk.a $LIBPATH -ldl		-o $TARGET_DIR/tde4_plugin_info.exe

unset echo
cd $basedir/../source/ldpk
set INCPATH="-I ../../include"
set TARGET_DIR="../../compiled/ldpk/windows/lib"

echo ${cyan}"#------------------ standard -----------------------------------#"${end}
set echo
# Model "3DE4 Anamorphic - Standard, Degree 4"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_deg_4_rotate_squeeze_xy.C -o $TARGET_DIR/tde4_ldp_anamorphic_deg_4_rotate_squeeze_xy.$SUFFIX
# Model "3DE4 Anamorphic - Rescaled, Degree 4"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_deg_4_rotate_squeeze_xy_rescaled.C -o $TARGET_DIR/tde4_ldp_anamorphic_deg_4_rotate_squeeze_xy_rescaled.$SUFFIX
# Model "3DE4 Anamorphic - Standard, Degree 6"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_deg_6_rotate_squeeze_xy.C -o $TARGET_DIR/tde4_ldp_anamorphic_deg_6_rotate_squeeze_xy.$SUFFIX
# Model "3DE4 Anamorphic - Rescaled, Degree 6"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_deg_6_rotate_squeeze_xy_rescaled.C -o $TARGET_DIR/tde4_ldp_anamorphic_deg_6_rotate_squeeze_xy_rescaled.$SUFFIX
# Model "3DE4 Radial - Standard, Degree 4"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_decentered_deg_4_cylindric.C -o $TARGET_DIR/tde4_ldp_radial_decentered_deg_4_cylindric.$SUFFIX

unset echo
echo ${magenta}"#------------------ deprecated----------------------------------#"${end}
set echo
# Model "3DE4 Anamorphic - Degree 6"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_deg_6.C -o $TARGET_DIR/tde4_ldp_anamorphic_deg_6.$SUFFIX
# Model "3DE4 Radial - Fisheye, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_deg_8.C -o $TARGET_DIR/tde4_ldp_radial_deg_8.$SUFFIX
# Model "3DE Classic LD Model"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_classic_3de_mixed.C -o $TARGET_DIR/tde4_ldp_classic_3de_mixed.$SUFFIX

unset echo
echo ${yellow}"#------------------ experimental--------------------------------#"${end}
set echo
# Model "3DE4 Radial - Fisheye, Equidistant, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_equidistant_deg_8.experimental.C -o $TARGET_DIR/tde4_ldp_radial_fisheye_equidistant_deg_8.$SUFFIX
# Model "3DE4 Radial - Fisheye, Equisolid, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_equisolid_deg_8.experimental.C -o $TARGET_DIR/tde4_ldp_radial_fisheye_equisolid_deg_8.$SUFFIX
# Model "3DE4 Radial - Fisheye, Orthographic, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_orthographic_deg_8.experimental.C -o $TARGET_DIR/tde4_ldp_radial_fisheye_orthographic_deg_8.$SUFFIX
# Model "3DE4 Radial - Fisheye, Stereographic, Degree 8"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_fisheye_stereographic_deg_8.experimental.C -o $TARGET_DIR/tde4_ldp_radial_fisheye_stereographic_deg_8.$SUFFIX

unset echo
echo ${orange}"#------------------ toymodels ----------------------------------#"${end}
set echo
#-------------------------------------------------------------#
# Toymodels, mainly internal use, for doing experiments       #
# and learning                                                #
#-------------------------------------------------------------#
# Model "3DE4 Radial - Homomorphic, Degree 2"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_radial_homomorphic_decentered_deg_2.experimental.C -o $TARGET_DIR/tde4_ldp_radial_homomorphic_decentered_deg_2.$SUFFIX
# Model "3DE4 Anamorphic - Homomorphic, Degree 2"
$CC $OPT $SHARED -DLDPK_COMPILE_AS_PLUGIN_SDV $INCPATH tde4_ldp_anamorphic_homomorphic_deg_2.experimental.C -o $TARGET_DIR/tde4_ldp_anamorphic_homomorphic_deg_2.$SUFFIX
