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

#include "result_post_process.h"
#include <algorithm>
#include <mf/opencv.h>
#include <mf/image/image.h>

namespace vs {

using namespace mf;

void result_post_process_filter::process_frame
(const input_view_type& in, const output_view_type& out, job_type& job) {
	double inpaint_radius = 10.0;
	
	masked_image<color_type> img(in);

	cv::Mat_<color_type> in_img = img.cv_mat();
	cv::Mat_<uchar> in_mask = img.cv_mask_mat();
	
	cv::Mat_<uchar> holes;
	cv::bitwise_not(in_mask, holes);
	
	cv::Vec<uchar, 3> inpaint_background(0, 128, 128);
	
	cv::Mat_<color_type> out_img;
	in_img.setTo(inpaint_background, holes);
	
	cv::inpaint(in_img, holes, out_img, inpaint_radius, cv::INPAINT_NS);
	
	copy_to_ndarray_view(out_img, out);
}

}
