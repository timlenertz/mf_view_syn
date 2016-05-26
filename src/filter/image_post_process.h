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

#include <mf/filter/simple_filter.h>
#include <mf/masked_elem.h>
#include <mf/opencv.h>
#include "../common.h"

namespace vs {

class image_post_process_filter : public mf::flow::simple_filter<2, masked_color_type, masked_color_type> {
private:
	void erode_left_bounds_(cv::Mat_<uchar>&);
	void erode_right_bounds_(cv::Mat_<uchar>&);

public:
	parameter_type<bool> right_side;

	using simple_filter::simple_filter;

	void process_frame(const input_view_type& in, const output_view_type& out, job_type& job) override;	
};
	
}

#endif
