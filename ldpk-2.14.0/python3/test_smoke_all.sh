#!/bin/bash

function test_smoke() {
	python_basedir="$1"
	python_command="$2"
	python_version="$3"

# Create python test program
	prg=$(cat <<EOF
import sys
sys.path.append("$ldpk_basedir/python3")
sys.path.append("$ldpk_basedir/python3/$os/python${python_version}")
print(sys.version)

import ldpk
print(ldpk.ldp.__file__)
#for m in ldpk.list_of_models:
#	print(m)
m = ldpk.opencv_radial_degree_6_decentered_degree_2()

EOF
)
# Write test program to target host.
	ssh ${user}@${host} "cat > ${tmpdir}/test.py" <<< "$prg" 2>&1 >> /dev/null
# Run test program
	cmd="${python_basedir}/${python_command} '${tmpdir}/test.py'"
	ssh ${user}@${host} ${cmd}
	}

user=ldpk

# MacOS
host=applejack
os=osx
ldpk_basedir=/server/devel/sdv/privat/uwe/source/ldpk
tmpdir="/tmp"

test_smoke /usr/local/bin python3.8 3.8
test_smoke /usr/local/bin python3.9 3.9
test_smoke /usr/local/bin python3.10 3.10
test_smoke /usr/local/bin python3.11 3.11
test_smoke /usr/local/bin python3.12 3.12
test_smoke /usr/local/bin python3.13 3.13

# Linux
host=belsazar
os=linux
ldpk_basedir=/server/devel/sdv/privat/uwe/source/ldpk
tmpdir="/tmp"

#test_smoke /usr/local/bin python3.8 3.8
#test_smoke /usr/local/bin python3.9 3.9
#test_smoke /usr/local/bin python3.10 3.10
#test_smoke /usr/local/bin python3.11 3.11
#test_smoke /usr/local/bin python3.12 3.12
#test_smoke /usr/local/bin python3.13 3.13

host=azrael
os=windows
ldpk_basedir=T:/devel/sdv/privat/uwe/source/ldpk
tmpdir="C:/Users/uwe/Desktop"

test_smoke /cygdrive/c/"Program\ Files"/Python38 python.exe 3.8
test_smoke /cygdrive/c/"Program\ Files"/Python39 python.exe 3.9
test_smoke /cygdrive/c/"Program\ Files"/Python310 python.exe 3.10
test_smoke /cygdrive/c/"Program\ Files"/Python311 python.exe 3.11
test_smoke /cygdrive/c/"Program\ Files"/Python312 python.exe 3.12
test_smoke /cygdrive/c/"Program\ Files"/Python313 python.exe 3.13


