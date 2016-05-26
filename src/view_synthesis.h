#ifndef VS_VIEW_SYNTHESIS_H_
#define VS_VIEW_SYNTHESIS_H_

#include <string>
#include <mf/color.h>
#include <mf/flow/graph.h>
#include <mf/filter/filter_parameter.h>
#include "common.h"
#include "rs_config_reader.h"

namespace vs {

class image_post_process_filter;

class view_synthesis {
private:
	rs_config_reader config_;
	mf::flow::graph graph_;
	
	mf::flow::filter_parameter<camera_type>* left_camera_parameter_ = nullptr;
	mf::flow::filter_parameter<camera_type>* right_camera_parameter_ = nullptr;
	mf::flow::filter_parameter<camera_type>* virtual_camera_parameter_ = nullptr;
	
	template<typename Input, typename Output>
	void connect_with_color_converter_(Input& input, Output& output);
	
	image_post_process_filter& setup_branch_(bool right_side);
	
public:
	explicit view_synthesis(const std::string& configuration_file);

	void setup();
	void run();
};


template<typename Input, typename Output>
void view_synthesis::connect_with_color_converter_(Input& input, Output& output) {
	/*
	Node
	 output      : output_elem_type
	  |
	 conv.input  : output_elem_type
	Converter
	 conv.output : input_elem_type
	  |
	 input       : input_elem_type
	Node
	*/
		
	MF_STATIC_ASSERT(Input::dimension == Output::dimension);
	constexpr std::size_t dimension = Input::dimension;
	using input_elem_type = typename Input::elem_type;
	using output_elem_type = typename Output::elem_type;
			
	auto& conv = graph_.add_convert_filter<dimension, output_elem_type, input_elem_type>(
		mf::color_convert<input_elem_type, output_elem_type>
	);
	conv.input.connect(output);
	input.connect(conv.output);
};



}

#endif

