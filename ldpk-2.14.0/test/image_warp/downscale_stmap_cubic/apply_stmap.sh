#!/bin/bash

export TDE4_ROOT=/usr/local/3DE4_linux64_r7.1_mod

$TDE4_ROOT/bin/sdv_image_warp.py -command warp_by_stmap -infile grid_with_marks.png -outfile out_from_downscale_1.png -stmapfile out_downscale_1.exr
$TDE4_ROOT/bin/sdv_image_warp.py -command warp_by_stmap -infile grid_with_marks.png -outfile out_from_downscale_2.png -stmapfile out_downscale_2.exr
$TDE4_ROOT/bin/sdv_image_warp.py -command warp_by_stmap -infile grid_with_marks.png -outfile out_from_downscale_4.png -stmapfile out_downscale_4.exr
$TDE4_ROOT/bin/sdv_image_warp.py -command warp_by_stmap -infile grid_with_marks.png -outfile out_from_downscale_8.png -stmapfile out_downscale_8.exr
