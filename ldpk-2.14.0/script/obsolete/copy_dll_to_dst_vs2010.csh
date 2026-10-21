#!/bin/csh -v

# Das Skript laeuft auf loriot.

pushd $LDPK/windows

cp LD_3DE4_NukeMultiRelease/x64/Release-Nuke10.0/*.dll ../compiled/nuke/windows/Nuke10.0/
cp LD_3DE4_NukeMultiRelease/x64/Release-Nuke10.5/*.dll ../compiled/nuke/windows/Nuke10.5/


# SDV-Tests Nuke auf Windows (virtueller Host loriot)
cp LD_3DE4_NukeMultiRelease/x64/Release-Nuke10.0/*.dll /cygdrive/c/Program\ Files/Nuke10.0v1/plugins/
cp LD_3DE4_NukeMultiRelease/x64/Release-Nuke10.5/*.dll /cygdrive/s/software/win64/nuke/Nuke10.5v1/plugins/

popd
