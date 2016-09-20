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

#ifndef VW_FILTER_DEPTH_POST_PROCESS_H_
#define VW_FILTER_DEPTH_POST_PROCESS_H_

#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include "../common.h"

namespace vs {

class depth_post_process_filter : public mf::flow::filter {
public:
	input_type<2, real_depth_type> depth_input;
	input_type<2, mask_type> depth_mask_input;
	output_type<2, real_depth_type> depth_output;
	output_type<2, mask_type> depth_mask_output;
	
	parameter_type<int> kernel_diameter;
	parameter_type<int> outer_iterations;
	parameter_type<int> inner_smooth_iterations;
	
	depth_post_process_filter() :
		depth_input(*this),
		depth_mask_input(*this),
		depth_output(*this),
		depth_mask_output(*this),
		kernel_diameter(*this),
		outer_iterations(*this),
		inner_smooth_iterations(*this)
	{
		depth_input.set_name("di");
		depth_mask_input.set_name("di mask");
		depth_output.set_name("di");
		depth_mask_output.set_name("di mask");
		kernel_diameter.set_name("kernel diameter");
		outer_iterations.set_name("outer iterations");
		inner_smooth_iterations.set_name("inner smooth iterations");
	}
	
	void configure(const json&);

	void setup() override;
	void process(job_type& job) override;
};
	
}

#endif
