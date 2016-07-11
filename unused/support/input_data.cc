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

#include "input_data.h"
#include <string>

using namespace mf;

input_data poznan_blocks() {	
	std::string dir = "/data/test_sequences/poznan_blocks/";

	std::string im = "Poznan_Blocks_1920x1080_texture_cam";
	std::string di = "Poznan_Blocks_1920x1080_depth_cf420_8bps_cam";
	std::string cm = "Poznan_Blocks_Physical_and_Virtual_View_camera_parameters.txt";
	std::string pr = "param_cam";

	std::size_t w = 1920, h = 1080;
	int sampling = 420;
	real z_far = 100.0, z_near = 15.0;

	depth_projection_parameters dparam;
	dparam.z_near = z_near;
	dparam.z_far = z_far;
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;

	vsrs_camera_array cams(dir + cm, dparam, {w, h}, true);

	input_data data {
		w,
		h,
		sampling,
		dparam
	};
	
	for(int i = 0; i <= 9; ++i) {	
		std::string cam_name = pr + std::to_string(i);
		input_visual vis {
			camera_type(cams[cam_name], make_ndsize(w, h)),
			dir + im + std::to_string(i) + ".yuv",
			dir + di + std::to_string(i) + ".yuv"
		};
		vis.camera.flip_pixel_coordinates();
		data.visuals.push_back(vis);
	}
	
	return data;
}


input_data poznan_blocks_scaled() {	
	std::string dir = "poznan_blocks/";

	std::string im = "im";
	std::string di = "di";
	std::string cm = "camera.txt";
	std::string pr = "param_cam";

	std::size_t w = 960, h = 540;
	int sampling = 420;
	real z_far = 100.0, z_near = 15.0;

	depth_projection_parameters dparam;
	dparam.z_near = z_near;
	dparam.z_far = z_far;
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;

	vsrs_camera_array cams(dir + cm, dparam, {w, h}, true);

	input_data data {
		w,
		h,
		sampling,
		dparam
	};
	
	for(int i = 0; i <= 9; ++i) {	
		std::string cam_name = pr + std::to_string(i);
		input_visual vis {
			camera_type(cams[cam_name], make_ndsize(w, h)),
			dir + im + std::to_string(i) + ".yuv",
			dir + di + std::to_string(i) + ".yuv"
		};
		vis.camera.flip_pixel_coordinates();
		data.visuals.push_back(vis);
	}
	
	return data;
}
