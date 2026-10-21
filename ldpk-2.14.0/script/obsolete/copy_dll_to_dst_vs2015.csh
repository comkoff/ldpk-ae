#!/bin/csh

# Skript laeuft auf doom/cygwin
# Direkt nach dem Kompilieren aufrufen.

pushd $LDPK/windows



set releases = ('11.0' '11.1' '11.2' '11.3' '12.0.Beta2')

echo "creating paths if needed: $ldpk/nuke/windows/NukeX.Y"
foreach RELEASE ($releases)
	mkdir -p $ldpk/compiled/nuke/windows/Nuke${RELEASE}
	chmod 777 $ldpk/compiled/nuke/windows/Nuke${RELEASE}
end

echo "copying DLLs to $ldpk/nuke/windows/NukeX.Y"
foreach RELEASE ($releases)
	cp LD_3DE4_NukeMultiRelease/x64/Release-Nuke${RELEASE}/*.dll $ldpk/compiled/nuke/windows/Nuke${RELEASE}/
	chmod 666 LD_3DE4_NukeMultiRelease/x64/Release-Nuke${RELEASE}/*.dll
end




set releases = ('10.0' '10.5' '11.0' '11.1' '11.2' '11.3' '12.0.Beta2')

echo "compress DLLs for early adopters"
foreach RELEASE ($releases)
	echo "building archive $ldpk/compiled/nuke/windows/Nuke${RELEASE}/ldpk_nuke_${RELEASE}.zip"
	zip --quite $ldpk/compiled/nuke/windows/Nuke${RELEASE}/ldpk_nuke_${RELEASE}.zip $ldpk/compiled/nuke/windows/Nuke${RELEASE}/*.dll -x $ldpk/compiled/nuke/windows/Nuke${RELEASE}/LD_3DE4_All_Parameter_Types.dll
	chmod 666 $ldpk/compiled/nuke/windows/Nuke${RELEASE}/ldpk_nuke_${RELEASE}.zip
end




# SDV-Tests Nuke auf Windows (doom)
echo "copying DLLs to /cygdrive/c/Program\ Files/NukeX.YvZ/plugins"

##### 11.0 #####################################################
set RELEASE=11.0
set VERSION=11.0v4
cp LD_3DE4_NukeMultiRELEASE/x64/RELEASE-Nuke$RELEASE/*.dll /cygdrive/c/Program\ Files/Nuke$VERSION/plugins
chmod 740 /cygdrive/c/Program\ Files/Nuke$VERSION/plugins/LD_3DE*.dll

##### 11.1 #####################################################
set RELEASE=11.1
set VERSION=11.1v2
cp LD_3DE4_NukeMultiRELEASE/x64/RELEASE-Nuke$RELEASE/*.dll /cygdrive/c/Program\ Files/Nuke$VERSION/plugins
chmod 740 /cygdrive/c/Program\ Files/Nuke$VERSION/plugins/LD_3DE*.dll

##### 11.2 #####################################################
set RELEASE=11.2
set VERSION=11.2v2
cp LD_3DE4_NukeMultiRELEASE/x64/RELEASE-Nuke$RELEASE/*.dll /cygdrive/c/Program\ Files/Nuke$VERSION/plugins
chmod 740 /cygdrive/c/Program\ Files/Nuke$VERSION/plugins/LD_3DE*.dll

##### 11.3 #####################################################
set RELEASE=11.3
set VERSION=11.3v1
cp LD_3DE4_NukeMultiRELEASE/x64/RELEASE-Nuke$RELEASE/*.dll /cygdrive/c/Program\ Files/Nuke$VERSION/plugins
chmod 740 /cygdrive/c/Program\ Files/Nuke$VERSION/plugins/LD_3DE*.dll

##### 12.0 ##################################################### BETA!!!
set RELEASE=12.0
set VERSION=12.0v1.Beta2
cp LD_3DE4_NukeMultiRELEASE/x64/RELEASE-Nuke$RELEASE/*.dll /cygdrive/c/Program\ Files/Nuke$VERSION/plugins
chmod 740 /cygdrive/c/Program\ Files/Nuke$VERSION/plugins/LD_3DE*.dll

echo "now edit /cygdrive/c/Program\ Files/NukeX.YvZ/plugins/menu.py and start Nuke."

popd
