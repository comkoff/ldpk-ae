#!/bin/csh -x

# internal/sdv: execute on vendetta.
# internal/sdv: compiler: type here

unset echo
if (`hostname` != "vendetta") then
	unset echo
	echo "***************************************************"
	echo "Please edit this script, enter your path(s) to Nuke "
	echo "and remove this error condition."
	echo "***************************************************"
	exit
endif
set echo

set basedir=`dirname $PWD/$0`
set OS="osx"
set CC="g++"
set SUFFIX="dylib"
setenv OPT "-O2 -fPIC -std=c++11 -stdlib=libc++ -Wno-deprecated-register"
setenv SHARED "-dynamiclib"
setenv INC_LDPK "-I $basedir/../include"
setenv LIB_NUKE "-lDDImage"

if `hostname` == "vendetta" then
# our compiled stuff goes here:
	setenv TARGET_DIR "compiled"
else
# your results here please:
	setenv TARGET_DIR "lib"
endif

cd $basedir/../source/ldpk

########################### Nuke 11.0 ###########################
unset echo
setenv NUKE_VERSION "11.0"
setenv NUKE_FULL_VERSION "${NUKE_VERSION}v4"
setenv INC_NUKE "-I /Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.app/Contents/MacOS/include"
setenv LIBDIR_NUKE "-L/Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.app/Contents/MacOS"
mkdir -p $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION
set echo

$CC $OPT $SHARED nuke_ld_3de4_anamorphic_degree_6.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Degree_6.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_rescaled_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Rescaled_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_degree_8.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de_classic_ld_model.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE_Classic_LD_Model.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_all_par_types.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_All_Parameter_Types.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_homomorphic_degree_2.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Homomorphic_Degree_2.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equisolid_degree_8.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equisolid_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_orthographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Orthographic_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_stereographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Stereographic_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_y_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Y_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_z_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Z_Degree_8.$SUFFIX

########################### Nuke 11.1 ###########################
unset echo
setenv NUKE_VERSION "11.1"
setenv NUKE_FULL_VERSION "${NUKE_VERSION}v1"
setenv INC_NUKE "-I /Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.app/Contents/MacOS/include"
setenv LIBDIR_NUKE "-L/Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.app/Contents/MacOS"
mkdir -p $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION
set echo

$CC $OPT $SHARED nuke_ld_3de4_anamorphic_degree_6.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Degree_6.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_rescaled_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Rescaled_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_degree_8.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de_classic_ld_model.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE_Classic_LD_Model.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_all_par_types.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_All_Parameter_Types.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_homomorphic_degree_2.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Homomorphic_Degree_2.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equisolid_degree_8.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equisolid_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_orthographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Orthographic_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_stereographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Stereographic_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_y_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Y_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_z_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Z_Degree_8.$SUFFIX

########################### Nuke 11.2 ###########################
# Wir gehen dazu ueber, die Nuke-Sachen direkt vom Installationsrechner (vendetta) zu verwenden.
unset echo
setenv NUKE_VERSION "11.2"
setenv NUKE_FULL_VERSION "${NUKE_VERSION}v2"
setenv INC_NUKE "-I /Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.app/Contents/MacOS/include"
setenv LIBDIR_NUKE "-L/Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.app/Contents/MacOS"
mkdir -p $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION
set echo

$CC $OPT $SHARED nuke_ld_3de4_anamorphic_degree_6.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Degree_6.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_rescaled_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Rescaled_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_degree_8.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de_classic_ld_model.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE_Classic_LD_Model.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_all_par_types.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_All_Parameter_Types.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_homomorphic_degree_2.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Homomorphic_Degree_2.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equisolid_degree_8.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equisolid_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_orthographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Orthographic_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_stereographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Stereographic_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_y_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Y_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_z_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Z_Degree_8.$SUFFIX

########################### Nuke 11.3 ###########################
# Wir gehen dazu ueber, die Nuke-Sachen direkt vom Installationsrechner (vendetta) zu verwenden.
unset echo
setenv NUKE_VERSION "11.3"
setenv NUKE_FULL_VERSION "${NUKE_VERSION}v1"
setenv INC_NUKE "-I /Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.app/Contents/MacOS/include"
setenv LIBDIR_NUKE "-L/Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.app/Contents/MacOS"
mkdir -p $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION
set echo

$CC $OPT $SHARED nuke_ld_3de4_anamorphic_degree_6.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Degree_6.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_rescaled_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Rescaled_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_degree_8.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de_classic_ld_model.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE_Classic_LD_Model.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_all_par_types.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_All_Parameter_Types.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_homomorphic_degree_2.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Homomorphic_Degree_2.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equisolid_degree_8.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equisolid_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_orthographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Orthographic_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_stereographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Stereographic_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_y_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Y_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_z_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Z_Degree_8.$SUFFIX

########################### Nuke 12.0 ###########################
# Wir gehen dazu ueber, die Nuke-Sachen direkt vom Installationsrechner (vendetta) zu verwenden.
unset echo
setenv NUKE_VERSION "12.0"
setenv NUKE_FULL_VERSION "${NUKE_VERSION}v1"
setenv INC_NUKE "-I /Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.Beta2.app/Contents/MacOS/include"
setenv LIBDIR_NUKE "-L/Applications/Nuke{$NUKE_FULL_VERSION}/Nuke{$NUKE_FULL_VERSION}.Beta2.app/Contents/MacOS"
mkdir -p $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION
set echo

$CC $OPT $SHARED nuke_ld_3de4_anamorphic_degree_6.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Degree_6.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_anamorphic_rescaled_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Anamorphic_Rescaled_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_degree_8.C						nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_standard_degree_4.C					nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Standard_Degree_4.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de_classic_ld_model.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE_Classic_LD_Model.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_all_par_types.C							nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_All_Parameter_Types.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_homomorphic_degree_2.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Homomorphic_Degree_2.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equisolid_degree_8.experimental.C			nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equisolid_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_orthographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Orthographic_Degree_8.$SUFFIX
$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_stereographic_degree_8.experimental.C		nuke_ld_3de4_base.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Stereographic_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_y_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Y_Degree_8.$SUFFIX
#$CC $OPT $SHARED nuke_ld_3de4_radial_fisheye_equidistant_longlat_z_degree_8.experimental.C	nuke_ld_3de4_base_nocache.C $INC_LDPK $INC_NUKE $LIBDIR_NUKE $LIB_NUKE -o $basedir/../$TARGET_DIR/nuke/$OS/Nuke$NUKE_VERSION/LD_3DE4_Radial_Fisheye_Equidistant_Longlat_Z_Degree_8.$SUFFIX

unset echo
echo "**********************************************************"
echo "Plugins should now be in LDPK/$TARGET_DIR/nuke/$OS/NukeX.Y/"
echo "Please copy them to your Nuke plugins directory."
echo "All plugin names start with 'LD_3DE...'."
echo "**********************************************************"
