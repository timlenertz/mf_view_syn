/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef PROG_INPUT_DATA_H_
#define PROG_INPUT_DATA_H_

#include <cstdint>
#include <vector>
#include <mf/camera/projection_image_camera.h>
#include <mf/config/vsrs_camera_array.h>
#include "common.h"

using camera_type = mf::projection_image_camera<depth_type>;

struct input_visual {
	camera_type camera;
	std::string image_yuv_file;
	std::string depth_image_yuv_file;
};

struct input_data {
	std::size_t image_width;
	std::size_t image_height;
	int yuv_sampling;
	mf::depth_projection_parameters projection_parameters;
	
	std::vector<input_visual> visuals;
};

input_data poznan_blocks();
input_data poznan_blocks_scaled();

#endif
