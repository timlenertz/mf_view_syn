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

#ifndef VW_FILTER_IMAGE_REVERSE_WARP_H_
#define VW_FILTER_IMAGE_REVERSE_WARP_H_

#include <mf/filter/filter.h>
#include "../common.h"

namespace vs {

class image_reverse_warp_filter : public mf::flow::filter {
private:
	mf::ndsize<2> shape_;

public:
	input_type<2, color_type> source_image_input;
	input_type<2, real_depth_type> destination_depth_input;
	input_type<2, mask_type> destination_depth_mask_input;
	
	output_type<2, color_type> destination_image_output;
	output_type<2, mask_type> destination_image_mask_output;
	
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;

	image_reverse_warp_filter() :
		source_image_input(*this),
		destination_depth_input(*this),
		destination_depth_mask_input(*this),
		destination_image_output(*this),
		destination_image_mask_output(*this)
	{
		source_image_input.set_name("im");
		destination_depth_input.set_name("di");
		destination_depth_mask_input.set_name("di mask");
		destination_image_output.set_name("im");
		destination_image_mask_output.set_name("im mask");
	}
	
	void setup() override;
	void process(job_type& job) override;	
};

}

#endif
