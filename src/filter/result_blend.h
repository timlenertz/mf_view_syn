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

#ifndef VW_FILTER_RESULT_BLEND_H_
#define VW_FILTER_RESULT_BLEND_H_

#include <mf/filter/filter.h>
#include <utility>
#include "../common.h"

namespace vs {

class result_blend_filter : public mf::flow::filter {
private:
	mf::ndsize<2> shape_;
	
	std::pair<mf::real, mf::real> weights_(job_type& job) const;

public:
	input_type<2, masked_color_type> left_image_input;
	input_type<2, masked_color_type> right_image_input;
	input_type<2, masked_real_depth_type> left_depth_input;
	input_type<2, masked_real_depth_type> right_depth_input;
	output_type<2, masked_color_type> virtual_image_output;
	parameter_type<camera_type> left_source_camera;
	parameter_type<camera_type> right_source_camera;
	parameter_type<camera_type> virtual_camera;

	result_blend_filter() :
		left_image_input(*this),
		left_depth_input(*this),
		right_image_input(*this),
		right_depth_input(*this),
		virtual_image_output(*this) { }

	void setup() override;
	void process(job_type& job) override;	
};

}

#endif
