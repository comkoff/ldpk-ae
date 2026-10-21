#!/bin/bash

export SDV_IMAGE_COMP=/server/software/linux64/bin/sdv_image_comp

$SDV_IMAGE_COMP < exr_to_png.il -in out_flame_img_a1_undistort.001.exr -out out_flame_img_a1_undistort.png
$SDV_IMAGE_COMP < exr_to_png.il -in out_flame_img_a2_undistort.001.exr -out out_flame_img_a2_undistort.png
$SDV_IMAGE_COMP < exr_to_png.il -in out_flame_img_b_undistort.001.exr -out out_flame_img_b_undistort.png
