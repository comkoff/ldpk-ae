#!/bin/bash

export SDV_IMAGE_COMP=/server/software/linux64/bin/sdv_image_comp

$SDV_IMAGE_COMP < exr_to_png.il -in out_imgwarp_img_a1_undistort.exr -out out_imgwarp_img_a1_undistort.png
$SDV_IMAGE_COMP < exr_to_png.il -in out_imgwarp_img_a2_undistort.exr -out out_imgwarp_img_a2_undistort.png
$SDV_IMAGE_COMP < exr_to_png.il -in out_imgwarp_img_b_undistort.exr -out out_imgwarp_img_b_undistort.png
