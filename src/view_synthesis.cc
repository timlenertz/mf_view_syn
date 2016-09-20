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
#include "filter/result_blend.h"
#include "filter/result_post_process.h"
#include <mf/filter/exporter.h>
#include <mf/filter/importer.h>
#include <mf/io/yuv_importer.h>
#include <mf/io/raw_video_exporter.h>

#include <mf/flow/diagnostic/processing_timeline.h>
#include <mf/flow/diagnostic/processing_timeline_json_exporter.h>

#include <iostream>

namespace vs {

using namespace mf;


view_synthesis::view_synthesis(const std::string& configuration_file) :
	configuration_(configuration_file) { }


auto view_synthesis::setup_branch_(const configuration::input_view& view) -> branch_end {
	auto shape = configuration_.input_shape();
	int yuv_sampling = 420;

	auto& image_source = graph_.add_filter<flow::importer_filter<yuv_importer>>(
		view.image_sequence_file, flip(shape), yuv_sampling
	);
	image_source.set_name(view.name + "image source");
	auto& depth_source = graph_.add_filter<flow::importer_filter<yuv_importer>>(
		view.depth_sequence_file, flip(shape), yuv_sampling
	);
	depth_source.set_name(view.name + "depth source");	
	
	auto& depth_warp = graph_.add_filter<depth_warp_filter>();
	depth_warp.set_name(view.name + "depth warp");
	depth_warp.source_camera.set_constant_value(view.camera);
	depth_warp.destination_camera.set_value_function(configuration_.virtual_camera());
	depth_warp.depth_input.connect(depth_source.output, [](ycbcr_color col) { return col.y; });	
	
	auto& depth_post = graph_.add_filter<depth_post_process_filter>();
	depth_post.set_name(view.name + "depth refine");
	depth_post.depth_input.connect(depth_warp.depth_output);
	depth_post.depth_mask_input.connect(depth_warp.depth_mask_output);
	
	auto& image_warp = graph_.add_filter<image_reverse_warp_filter>();
	image_warp.set_name(view.name + "image reverse warp");	
	image_warp.source_image_input.connect(image_source.output, color_convert<rgb_color, ycbcr_color>);	
	image_warp.destination_depth_input.connect(depth_post.depth_output);
	image_warp.destination_depth_mask_input.connect(depth_post.depth_mask_output);
	image_warp.source_camera.set_reference(depth_warp.source_camera);
	image_warp.destination_camera.set_reference(depth_warp.destination_camera);

	
	auto& image_post = graph_.add_filter<image_post_process_filter>();
	image_post.set_name(view.name + "image refine");
	image_post.right_side.set_dynamic(false);
	image_post.image_input.connect(image_warp.destination_image_output);
	image_post.image_mask_input.connect(image_warp.destination_image_mask_output);
	
	image_post.set_asynchonous(true);
	image_post.set_prefetch_duration(5);
	
	return branch_end {
		image_post.image_output,
		depth_post.depth_output,
		image_post.image_mask_output,
		depth_warp.source_camera,
		image_post.right_side
	};
}


void view_synthesis::setup() {	
	auto& blend = graph_.add_filter<result_blend_filter>();
	blend.set_name("blend");
	blend.virtual_camera.set_value_function(configuration_.virtual_camera());
	
	for(std::ptrdiff_t i = 0; i < configuration_.input_views_count(); ++i) {		
		configuration::input_view view = configuration_.input_view_at(i);
		branch_end b_end = setup_branch_(view);
		result_blend_filter::input_branch& b_in = blend.add_input_branch(view.name);
		
		b_in.image_input.connect(b_end.image_output);
		b_in.depth_input.connect(b_end.depth_output);
		b_in.mask_input.connect(b_end.mask_output);
		b_in.source_camera.set_reference(b_end.source_camera);
		//b_in.source_camera.set_constant_value(b_end.source_camera.deterministic_value(0));
		b_in.right_side_sent.set_sent_reference(b_end.right_side);
	}	
		
//	blend.set_asynchonous(true);
//	blend.set_prefetch_duration(5);
	
	auto& result_post = graph_.add_filter<result_post_process_filter>();
	result_post.set_name("refine");
	result_post.image_input.connect(blend.virtual_image_output);
	result_post.image_mask_input.connect(blend.virtual_mask_output);

	auto shape = configuration_.input_shape();
	if(configuration_.output_rgb()) {
		auto& sink = graph_.add_filter<flow::exporter_filter<raw_video_exporter<rgb_color>>>(
			configuration_["output"]["image_sequence_file"],
			configuration_.output_rgb_raw_format()
		);
		sink.set_name("video export");
		sink.input.connect(result_post.image_output);

	} else {
		auto& sink = graph_.add_filter<flow::exporter_filter<raw_video_exporter<ycbcr_color>>>(
			configuration_["output"]["image_sequence_file"],
			configuration_.output_ycbcr_raw_format()
		);
		sink.set_name("video export");
		sink.input.connect(result_post.image_output, color_convert<ycbcr_color, rgb_color>);
	}
		
	graph_.setup();
}


void view_synthesis::run() {
	/*graph_.callback_function = [](time_unit t) {
		std::cout << "frame " << t << "..." << std::endl;
	};*/
	
	mf::flow::processing_timeline timeline(*graph_.node_graph_);
	
	graph_.node_graph_->set_diagnostic(timeline);
	
	graph_.run_for(100);
	graph_.node_graph_->stop();
	
	std::cerr << "stopped" << std::endl;
	
	mf::flow::processing_timeline_json_exporter exp(timeline);
	std::ofstream exp_f("timeline.json");
	exp.generate(exp_f);
}


}
