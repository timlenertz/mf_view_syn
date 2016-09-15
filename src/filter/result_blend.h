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
#include <mf/filter/filter_parameter.h>
#include <utility>
#include "../common.h"

namespace vs {

class result_blend_filter : public mf::flow::filter {
private:
	mf::ndsize<2> shape_;
	
	std::pair<mf::real, mf::real> weights_(job_type& job) const;

public:
	input_type<2, color_type> left_image_input;
	input_type<2, real_depth_type> left_depth_input;
	input_type<2, tri_mask_type> left_mask_input;
	
	input_type<2, color_type> right_image_input;
	input_type<2, real_depth_type> right_depth_input;
	input_type<2, tri_mask_type> right_mask_input;

	output_type<2, color_type> virtual_image_output;
	output_type<2, mask_type> virtual_mask_output;
	
	parameter_type<camera_type> left_source_camera;
	parameter_type<camera_type> right_source_camera;
	parameter_type<camera_type> virtual_camera;
	
	bool depth_blending = true;
	mf::real depth_blending_minimal_difference = 5.0;

	result_blend_filter() :
		left_image_input(*this),
		left_depth_input(*this),
		left_mask_input(*this),
		right_image_input(*this),
		right_depth_input(*this),
		right_mask_input(*this),
		virtual_image_output(*this),
		virtual_mask_output(*this),
		left_source_camera(*this),
		right_source_camera(*this),
		virtual_camera(*this)
	{
		left_image_input.set_name("left im");
		right_image_input.set_name("right im");
		left_depth_input.set_name("left di");
		right_depth_input.set_name("right di");
		left_mask_input.set_name("left mask");
		right_mask_input.set_name("right mask");
		virtual_image_output.set_name("im");
		virtual_mask_output.set_name("im mask");
		left_source_camera.set_name("left cam");
		right_source_camera.set_name("right cam");
		virtual_camera.set_name("virtual cam");
	}

	void setup() override;
	void process(job_type& job) override;	
};

}

#endif
