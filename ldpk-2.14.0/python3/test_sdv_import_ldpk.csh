#!/bin/tcsh

# This is sdv-internal only. We test if the pythonmodules
# are imported in all versions of python. The crucial part
# is importing binary lens_distortion_models.so, which
# must be provided per platform and per python version.

sdv_use_python 3.8 > /dev/null
python3.8 -m ldpk
sdv_use_python 3.9 > /dev/null
python3.9 -m ldpk
sdv_use_python 3.10 > /dev/null
python3.10 -m ldpk
sdv_use_python 3.11 > /dev/null
python3.11 -m ldpk
sdv_use_python 3.12 > /dev/null
python3.12 -m ldpk
sdv_use_python 3.13 > /dev/null
python3.13 -m ldpk
