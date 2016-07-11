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

#ifndef VS_VIEW_SYNTHESIS_H_
#define VS_VIEW_SYNTHESIS_H_

#include <string>
#include <tuple>
#include <mf/color.h>
#include <mf/filter/filter_graph.h>
#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include "common.h"
#include "rs_config_reader.h"

namespace vs {

class image_post_process_filter;

class view_synthesis {
private:
	struct branch_outputs {
		mf::flow::filter_output<2, masked_color_type>* image_output;
		mf::flow::filter_output<2, masked_real_depth_type>* depth_output;
	};
	
	rs_config_reader config_;
	mf::flow::filter_graph graph_;
	
	mf::flow::filter_parameter<camera_type>* left_camera_parameter_ = nullptr;
	mf::flow::filter_parameter<camera_type>* right_camera_parameter_ = nullptr;
	mf::flow::filter_parameter<camera_type>* virtual_camera_parameter_ = nullptr;
	
	
		
	branch_outputs setup_branch_(bool right_side);
	
public:
	explicit view_synthesis(const std::string& configuration_file);

	void setup();
	void run();
};


}

#endif

