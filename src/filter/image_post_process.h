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

#ifndef VW_FILTER_IMAGE_POST_PROCESS_H_
#define VW_FILTER_IMAGE_POST_PROCESS_H_

#include <mf/filter/filter_handler.h>
#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include <mf/opencv.h>
#include "../common.h"

namespace vs {

class image_post_process_filter : public mf::flow::filter_handler {
private:
	void erode_left_bounds_(cv::Mat_<uchar>&);
	void erode_right_bounds_(cv::Mat_<uchar>&);

	bool should_erode_right_(job_type& job) const;

public:
	input_type<2, color_type> image_input;
	input_type<2, mask_type> image_mask_input;
	
	output_type<2, color_type> image_output;
	output_type<2, tri_mask_type> image_mask_output;

	parameter_type<int> kernel_diameter;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> virtual_camera;
	
	explicit image_post_process_filter(mf::flow::filter& filt) :
		mf::flow::filter_handler(filt),
		image_input(filt, "in"),
		image_mask_input(filt, "in mask"),
		image_output(filt, "out"),
		image_mask_output(filt, "out mask"),
		source_camera(filt, "source cam"),
		virtual_camera(filt, "virtual cam"),
		kernel_diameter(filt, "kernel diameter") { }

	void configure(const json&);

	void setup() override;
	void process(job_type& job) override;	
};
	
}

#endif
