#!/usr/bin/tcsh

# Last tested and revised: 2022-12-12

# We use to work with cygwin, but maybe you prefer PowerShell.
# In case you want to compile for a specific version of python3
# feel free to modify this script or set up a Visual Studio Solution.

# It is unlikely that this will run on your machine without
# modifications. Please check carefully. Maybe the pre-compiled
# python pyd-modules in $LDPK/python3/windows already provide what you need.
# As of 2023 we provide python bindings for python 3.7 to 3.11.

# Script must be started from $LDPK/script.

echo -e ${white}${bold}"#-------------- make_python3_bindings.windows.sh ---------------#"${end}

set PYTHON_VERSION=$1
set PYTHON_VERSION_WIN=`echo $PYTHON_VERSION | sed s/"\."/""/g`
echo "including headers at C:/Program Files/Python${PYTHON_VERSION_WIN}/include"

echo ${lime}"Compiling ${white}${bold}../source/ldpk/tde4_lens_distortion_plugins.module.3.C ${green}to ${white}${bold}../source/ldpk/windows/tde4_lens_distortion_plugins.module.3.obj"${reset}

# /c: Compile, don't link. Tp: Compile as C++.
# /W1: show serious warnings (/W3 would be better).
# /EHsc: Exception handling, see Microsoft Docs.
# /D_WINDOWS: A macro we use, /D_WINDLL: Used by someone else.
cl	/c \
	/Tp ../source/ldpk/tde4_lens_distortion_plugins.module.3.C \
	/Fo"../source/ldpk/windows/tde4_lens_distortion_plugins.module.3.obj" \
	/EHsc /W1 /nologo \
	/D_WINDOWS /D_WINDLL \
	/I "../include" \
	/I "C:/Program Files/Python${PYTHON_VERSION_WIN}/include" \
	/I "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/include/" \
	/I "C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/shared" \
	/I "C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/ucrt" \
	/I "C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/um"

mkdir -p ../python3/windows/python${PYTHON_VERSION}
echo ${lime}"Linking ${white}${bold}../source/ldpk/windows/tde4_lens_distortion_plugins.module.3.obj to ${white}${bold}../python3/windows/python${PYTHON_VERSION}/lens_distortion_plugins.pyd"${reset}

link	/DLL /DYNAMICBASE /NOIMPLIB /NOEXP /NOLOGO\
	/OUT:"../python3/windows/python${PYTHON_VERSION}/lens_distortion_plugins.pyd" \
	"../source/ldpk/windows/tde4_lens_distortion_plugins.module.3.obj" \
	"kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" \
	"comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" \
	"oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" \
	/LIBPATH:"C:/Program Files/Python${PYTHON_VERSION_WIN}/libs" \
	/LIBPATH:"C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/lib/x64/" \
	/LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/ucrt/x64" \
	/LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x64"

echo ${green}"done."${reset}

# This copies the python module to Windows' python installation.
# Please check if you really want this.
cp ../python3/windows/python${PYTHON_VERSION}/lens_distortion_plugins.pyd /cygdrive/c/"Program Files"/Python$PYTHON_VERSION_WIN/DLLS/

