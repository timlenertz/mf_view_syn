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

#ifndef VW_FILTER_RESULT_POST_PROCESS_H_
#define VW_FILTER_RESULT_POST_PROCESS_H_

#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include "../common.h"

namespace vs {
	
class result_post_process_filter : public mf::flow::filter {
public:
	input_type<2, color_type> image_input;
	input_type<2, mask_type> image_mask_input;
	output_type<2, color_type> image_output;
	
	parameter_type<double> inpaint_radius;
	parameter_type<color_type> inpaint_background;
	
	result_post_process_filter() :
		image_input(*this),
		image_mask_input(*this),
		image_output(*this),
		inpaint_radius(*this),
		inpaint_background(*this)
	{
		image_input.set_name("im");
		image_mask_input.set_name("im mask");
		image_output.set_name("im");
		inpaint_radius.set_name("inpaint radius");
		inpaint_background.set_name("inpaint background");
	}
	
	void configure(const json&);
	
	void setup() override;
	void process(job_type& job) override;
};
	
}

#endif
