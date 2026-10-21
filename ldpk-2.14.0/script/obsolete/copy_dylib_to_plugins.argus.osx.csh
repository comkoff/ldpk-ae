#!/bin/csh
# Auf argus ausfuehren

set LDPK=/server/devel/sdv/privat/uwe/source/ldpk

# Wir kopieren die Plugins nur fuer die Nuke-Versionen, die auf vendetta installiert sind.
rm -rfv /Applications/Nuke12.0v1/Nuke12.0v1.app/Contents/MacOS/plugins/LD_3DE*.dylib

cp -v $LDPK/compiled/nuke/osx/Nuke12.0/*.dylib /Applications/Nuke12.0v1/Nuke12.0v1.app/Contents/MacOS/plugins/

echo "make tgz-archive for early adopters"
foreach VERSION ($nuke_versions)
	pushd $LDPK/compiled/nuke/osx/Nuke${VERSION}/
	rm		ldpk_nuke_${VERSION}.osx.tgz
	tar cfz		ldpk_nuke_${VERSION}.osx.tgz *.dylib
	chmod 666	ldpk_nuke_${VERSION}.osx.tgz
	popd
end
echo "early adopters' delight is here:"
ll $LDPK/compiled/nuke/osx/Nuke*/*tgz

