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
#include "configuration.h"

namespace vs {

class view_synthesis {
private:
	enum class mode {
		forward_warp,
		vsrs
	};

	struct branch_end {
		mf::flow::filter_output<2, color_type>& image_output;
		mf::flow::filter_output<2, real_depth_type>& depth_output;
		mf::flow::filter_output<2, tri_mask_type>& mask_output;
		mf::flow::filter_parameter<camera_type>& source_camera;
	};
	
	mf::flow::filter_parameter<camera_type>* vcam;
	
	const configuration& configuration_;
	mode mode_;
	mf::flow::filter_graph graph_;
		
	branch_end setup_branch_forward_warp_(const configuration::input_view&);	
	branch_end setup_branch_vsrs_(const configuration::input_view&);	
	branch_end setup_branch_(const configuration::input_view&);	
		
public:
	explicit view_synthesis(const configuration&);

	void setup();
	void run();
};


}

#endif

