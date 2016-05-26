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

#include "view_synthesis.h"
#include "rs_camera_array.h"
#include "filter/depth_post_process.h"
#include "filter/depth_warp.h"
#include "filter/image_post_process.h"
#include "filter/image_reverse_warp.h"
#include "filter/result_blend.h"
#include "filter/result_post_process.h"
#include <mf/filter/exporter.h>
#include <mf/filter/importer.h>
#include <mf/io/yuv_importer.h>
#include <mf/io/video_exporter.h>
#include <mf/flow/sync_node.h>

#include <iostream>

namespace vs {

using namespace mf;


view_synthesis::view_synthesis(const std::string& configuration_file) :
	config_(configuration_file) { }


image_post_process_filter& view_synthesis::setup_branch_(bool right_side) {
	std::string image_file_parameter = (right_side ? "RightViewImageName" : "LeftViewImageName");
	std::string depth_file_parameter = (right_side ? "RightDepthMapName" : "LeftDepthMapName");
	std::string camera_name_parameter = (right_side ? "RightCameraName" : "LeftCameraName");
	std::string z_near_parameter = (right_side ? "RightNearestDepthValue" : "LeftNearestDepthValue");
	std::string z_far_parameter = (right_side ? "RightFarthestDepthValue" : "LeftFarthestDepthValue");
	auto camera_parameter_ptr = (right_side ? &right_camera_parameter_ : &left_camera_parameter_);

	auto shape = make_ndsize(config_.get_int("SourceHeight"), config_.get_int("SourceWidth"));
	int yuv_sampling = 420;

	auto& image_source = graph_.add_filter<flow::importer_filter<yuv_importer>>(
		config_.get_string(image_file_parameter), shape, yuv_sampling
	);
	auto& depth_source = graph_.add_filter<flow::importer_filter<yuv_importer>>(
		config_.get_string(depth_file_parameter), shape, yuv_sampling
	);
	
	depth_projection_parameters dparam;
	dparam.z_near = config_.get_real(z_near_parameter);
	dparam.z_far = config_.get_real(z_far_parameter);
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;
	
	rs_camera_array camera_array(config_.get_string("CameraParameterFile"), dparam, shape);
	camera_type source_camera(camera_array[config_.get_string(camera_name_parameter)], flip(shape));
	camera_type destination_camera(camera_array[config_.get_string("VirtualCameraName")], flip(shape));
	source_camera.flip_pixel_coordinates();
	destination_camera.flip_pixel_coordinates();
	
	auto& depth_warp = graph_.add_filter<depth_warp_filter>();
	depth_warp.source_camera.set_constant(source_camera);
	depth_warp.destination_camera.set_constant(destination_camera);
	connect_with_color_converter_(depth_warp.input, depth_source.output);
	*camera_parameter_ptr = &depth_warp.source_camera;
	virtual_camera_parameter_ = &depth_warp.destination_camera;
	
	auto& depth_post = graph_.add_filter<depth_post_process_filter>();
	depth_post.input.connect(depth_warp.output);
	
	auto& image_warp = graph_.add_filter<image_reverse_warp_filter>();
	image_warp.source_camera.set_constant(source_camera);
	image_warp.destination_camera.set_constant(destination_camera);
	connect_with_color_converter_(image_warp.source_image_input, image_source.output);
	image_warp.destination_depth_input.connect(depth_post.output);
	
	auto& image_post = graph_.add_filter<image_post_process_filter>();
	image_post.right_side.set_constant(right_side);
	image_post.input.connect(image_warp.destination_image_output);
	
	return image_post;
}


void view_synthesis::setup() {
	auto& left_branch = setup_branch_(false);
	auto& right_branch = setup_branch_(true);
	
	auto& blend = graph_.add_filter<result_blend_filter>();
	blend.left_source_camera.set_mirror(*left_camera_parameter_);
	blend.right_source_camera.set_mirror(*right_camera_parameter_);
	blend.virtual_camera.set_mirror(*virtual_camera_parameter_);
	blend.left_image_input.connect(left_branch.output);
	blend.right_image_input.connect(right_branch.output);
	
	auto& result_post = graph_.add_filter<result_post_process_filter>();
	result_post.input.connect(blend.virtual_image_output);

	auto shape = make_ndsize(config_.get_int("SourceHeight"), config_.get_int("SourceWidth"));
	auto& sink = graph_.add_sink_filter<flow::exporter_filter<video_exporter>>("output.avi", shape);
	sink.input.connect(result_post.output);
	
	graph_.setup();
}


void view_synthesis::run() {
	graph_.callback_function = [](time_unit t) {
		std::cout << "frame " << t << "..." << std::endl;
	};
	graph_.run();
}


}
