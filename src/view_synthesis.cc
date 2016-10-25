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
#include "configuration.h"
#include "filter/depth_post_process.h"
#include "filter/depth_warp.h"
#include "filter/image_post_process.h"
#include "filter/image_reverse_warp.h"
#include "filter/image_warp.h"
#include "filter/result_blend.h"
#include "filter/result_post_process.h"
#include "filter/gui_sink.h"
#include "filter/scale.h"
#include "filter/realtime_rewarp.h"
#include <mf/filter/handler/exporter.h>
#include <mf/filter/handler/importer.h>
#include <mf/io/yuv_importer.h>
#include <mf/io/raw_video_exporter.h>

#include <mf/flow/diagnostic/processing_timeline.h>
#include <mf/flow/diagnostic/processing_timeline_json_exporter.h>

#include <iostream>

namespace vs {

using namespace mf;


view_synthesis::view_synthesis(const configuration& config) :
	configuration_(config) { }


auto view_synthesis::setup_branch_forward_warp_(const configuration::input_view& view) -> branch_end {
	auto input_shape = configuration_.input_shape();
	int yuv_sampling = 420;

	auto ycbcr2depth = [](ycbcr_color col) { return col.y; };

	// image source
	auto& image_source = graph_.add_filter<flow::importer_filter<yuv_importer>>(
		view.image_sequence_file, flip(input_shape), yuv_sampling
	);
	image_source.set_name(view.name + " image source");
	
	// depth source
	auto& depth_source = graph_.add_filter<flow::importer_filter<yuv_importer>>(
		view.depth_sequence_file, flip(input_shape), yuv_sampling
	);
	depth_source.set_name(view.name + " depth source");
	
	
	
	// image+depth scaling
	scale_filter<rgb_color>* image_scale_handler = nullptr;
	scale_filter<integral_depth_type>* depth_scale_handler = nullptr;
	if(configuration_["input"].value("scale", 1.0) != 1.0) {
		auto& image_scale = graph_.add_filter<scale_filter<rgb_color>>();
		image_scale.set_name("image scale");
		image_scale->output_size = flip(configuration_.scaled_shape());
		auto& depth_scale = graph_.add_filter<scale_filter<integral_depth_type>>();
		depth_scale.set_name("depth scale");
		depth_scale->output_size = flip(configuration_.scaled_shape());
		depth_scale->interpolation = cv::INTER_NEAREST;
		
		image_scale->input.connect(image_source->output, color_convert<rgb_color, ycbcr_color>);
		depth_scale->input.connect(depth_source->output, ycbcr2depth);
		
		image_scale_handler = &image_scale.handler();
		depth_scale_handler = &depth_scale.handler();
	}
	
	
	// image forwards warping
	auto& image_warp = graph_.add_filter<image_warp_filter>();
	image_warp.set_name(view.name + " image warp");
	image_warp->source_camera.set_constant_value(view.camera);
	image_warp->destination_camera.set_value_function(configuration_.virtual_camera());

	if(depth_scale_handler != nullptr) image_warp->depth_input.connect(depth_scale_handler->output);
	else image_warp->depth_input.connect(depth_source->output, ycbcr2depth);	

	if(image_scale_handler != nullptr) image_warp->image_input.connect(image_scale_handler->output);
	else image_warp->image_input.connect(image_source->output, color_convert<rgb_color, ycbcr_color>);		
	
	
	image_warp.set_asynchonous(true);
	image_warp.set_prefetch_duration(1);
	
	// brand end that connected to blender
	return branch_end {
		image_warp->image_output,
		image_warp->depth_output,
		image_warp->mask_output,
		image_warp->source_camera
	};
}


auto view_synthesis::setup_branch_vsrs_(const configuration::input_view& view) -> branch_end {
	auto input_shape = configuration_.input_shape();
	int yuv_sampling = 420;

	auto ycbcr2depth = [](ycbcr_color col) { return col.y; };

	// image source
	auto& image_source = graph_.add_filter<flow::importer_filter<yuv_importer>>(
		view.image_sequence_file, flip(input_shape), yuv_sampling
	);
	image_source.set_name(view.name + " image source");
	
	// depth source
	auto& depth_source = graph_.add_filter<flow::importer_filter<yuv_importer>>(
		view.depth_sequence_file, flip(input_shape), yuv_sampling
	);
	depth_source.set_name(view.name + " depth source");
	
	
	// image+depth scaling
	scale_filter<rgb_color>* image_scale_handler = nullptr;
	scale_filter<integral_depth_type>* depth_scale_handler = nullptr;
	if(configuration_.scaled_shape() != input_shape) {
		auto& image_scale = graph_.add_filter<scale_filter<rgb_color>>();
		image_scale.set_name("image scale");
		image_scale->output_size = flip(configuration_.scaled_shape());
		auto& depth_scale = graph_.add_filter<scale_filter<integral_depth_type>>();
		depth_scale.set_name("depth scale");
		depth_scale->output_size = flip(configuration_.scaled_shape());
		depth_scale->interpolation = cv::INTER_NEAREST;
		
		image_scale->input.connect(image_source->output, color_convert<rgb_color, ycbcr_color>);
		depth_scale->input.connect(depth_source->output, ycbcr2depth);
		
		image_scale_handler = &image_scale.handler();
		depth_scale_handler = &depth_scale.handler();
	}
	
	// depth forwards warping
	auto& depth_warp = graph_.add_filter<depth_warp_filter>();
	depth_warp.set_name(view.name + " depth warp");

	if(depth_scale_handler != nullptr) depth_warp->depth_input.connect(depth_scale_handler->output);
	else depth_warp->depth_input.connect(depth_source->output, ycbcr2depth);	

	depth_warp->source_camera.set_constant_value(view.camera);
//	depth_warp->destination_camera.set_value_function(configuration_.virtual_camera());
	depth_warp->destination_camera.set_dynamic( configuration_.virtual_camera()(0) );
	vcam = &depth_warp->destination_camera;
	//depth_warp->destination_camera.set_constant_value(configuration_.output_camera(Eigen_vec3(0, 0, 0)));

	
	// depth refinement
	auto& depth_post = graph_.add_filter<depth_post_process_filter>();
	depth_post.set_name(view.name + " depth refine");
	depth_post->depth_input.connect(depth_warp->depth_output);
	depth_post->depth_mask_input.connect(depth_warp->depth_mask_output);
	depth_post->configure(configuration_["synthesis"]["depth_refine"]);
	
	// image reverse warping (using refined depth map)
	auto& image_warp = graph_.add_filter<image_reverse_warp_filter>();
	image_warp.set_name(view.name + " image reverse warp");	

	if(image_scale_handler != nullptr) image_warp->source_image_input.connect(image_scale_handler->output);
	else image_warp->source_image_input.connect(image_source->output, color_convert<rgb_color, ycbcr_color>);	

	image_warp->destination_depth_input.connect(depth_post->depth_output);
	image_warp->destination_depth_mask_input.connect(depth_post->depth_mask_output);
	image_warp->source_camera.set_reference(depth_warp->source_camera);
	image_warp->destination_camera.set_reference(depth_warp->destination_camera);



	// image refinement
	auto& image_post = graph_.add_filter<image_post_process_filter>();
	image_post.set_name(view.name + " image refine");

	image_post->image_input.connect(image_warp->destination_image_output);
	image_post->image_mask_input.connect(image_warp->destination_image_mask_output);
	image_post->source_camera.set_reference(depth_warp->source_camera);
	image_post->virtual_camera.set_reference(depth_warp->destination_camera);
	
	
	image_post->configure(configuration_["synthesis"]["image_refine"]);
	
	image_post.set_asynchonous(true);
	image_post.set_prefetch_duration(1);
	
	// brand end that connected to blender
	
	return branch_end {
		image_post->image_output,
		depth_post->depth_output,
		image_post->image_mask_output,
		depth_warp->source_camera
	};
}



auto view_synthesis::setup_branch_(const configuration::input_view& view) -> branch_end {
	if(mode_ == mode::forward_warp) return setup_branch_forward_warp_(view);
	else if(mode_ == mode::vsrs) return setup_branch_vsrs_(view);
	else throw std::logic_error("invalid mode");
}


void view_synthesis::setup() {
	std::string md = configuration_["synthesis"]["mode"];
	if(md == "forward_warp") mode_ = mode::forward_warp;
	else if(md == "vsrs") mode_ = mode::vsrs;
	else throw configuration_error("invalid mode");
		
	// blender
	auto& blend = graph_.add_filter<result_blend_filter>();
	blend.set_name("blend");
	
	int i=1;
	for(std::ptrdiff_t i = 0; i < configuration_.input_views_count(); ++i) {		
		configuration::input_view view = configuration_.input_view_at(i);
		branch_end b_end = setup_branch_(view);
		result_blend_filter::input_branch& b_in = blend->add_input_branch(view.name);
				
		b_in.image_input.connect(b_end.image_output);
		b_in.mask_input.connect(b_end.mask_output);

		//if(configuration_["synthesis"]["blend"].value("depth_based_blending", false))
			b_in.depth_input.connect(b_end.depth_output);

		b_in.source_camera.set_reference(b_end.source_camera);
	}
	blend->virtual_camera.set_reference(*vcam); /////
		
	blend->configure(configuration_["synthesis"]["blend"]);
		
	blend.set_asynchonous(false);
	
	// result refinement
	auto& result_post = graph_.add_filter<result_post_process_filter>();
	result_post.set_name("refine");
	result_post->image_input.connect(blend->virtual_image_output);
	result_post->image_mask_input.connect(blend->virtual_mask_output);
	result_post->image_depth_input.connect(blend->virtual_depth_output);
	result_post->configure(configuration_["synthesis"]["result_refine"]);


	// rewarp
	auto& rewarp = graph_.add_filter<realtime_rewarp>();
	rewarp.set_name("live rewarp");
	rewarp->image_input.connect(result_post->image_output);
	rewarp->depth_input.connect(result_post->image_depth_output);
	//rewarp->source_camera.set_reference(blend->virtual_camera);
	rewarp->source_camera.set_reference(*vcam, true, true);
	//rewarp->destination_camera.set_constant_value(configuration_.output_camera(Eigen_vec3(0, 0, 1.0)));
	rewarp->destination_camera.set_value_function(configuration_.virtual_camera());
	rewarp.set_own_timing(flow::stream_timing::real_time());

	// output
	/*
	auto& sink = graph_.add_filter<gui_sink>();
	sink->input.connect(rewarp->image_output);
	*/
	
	auto& vsink = graph_.add_filter<flow::exporter_filter<raw_video_exporter<rgb_color>>>(
		configuration_["output"]["image_sequence_file"],
		configuration_.output_rgb_raw_format()
	);
	vsink.set_name("video export");
	vsink->input.connect(rewarp->image_output);
	


/*
	// output
	if(configuration_.output_rgb()) {
		auto& sink = graph_.add_filter<flow::exporter_filter<raw_video_exporter<rgb_color>>>(
			configuration_["output"]["image_sequence_file"],
			configuration_.output_rgb_raw_format()
		);
		sink.set_name("video export");
		sink->input.connect(result_post->image_output);

	} else {
		auto& sink = graph_.add_filter<flow::exporter_filter<raw_video_exporter<ycbcr_color>>>(
			configuration_["output"]["image_sequence_file"],
			configuration_.output_ycbcr_raw_format()
		);
		sink.set_name("video export");
		sink->input.connect(result_post->image_output, color_convert<ycbcr_color, rgb_color>);
	}
*/
		
	graph_.setup();
}


void view_synthesis::run() {
	/*graph_.callback_function = [](time_unit t) {
		std::cout << "frame " << t << "..." << std::endl;
	};*/
	
	mf::flow::processing_timeline timeline(*graph_.node_graph_);
	
	graph_.node_graph_->set_diagnostic(timeline);
	
	try {
	graph_.run_for(800);
//	graph_.node_graph_->stop();
	} catch(...) { }
	
	std::cerr << "stopped" << std::endl;
	
	mf::flow::processing_timeline_json_exporter exp(timeline);
	std::ofstream exp_f("timeline.json");
	exp.generate(exp_f);
}


}
