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
#include <mf/image/masked_image_view.h>

#include <mf/io/image_export.h>


namespace vs {

using namespace mf;

void result_post_process_filter::setup() {
	image_output.define_frame_shape(image_input.frame_shape());
}


void result_post_process_filter::process(job_type& job) {
	auto in = job.in(image_input);
	auto in_mask = job.in(image_mask_input);
	auto out = job.out(image_output);	

	double inpaint_radius = 10.0;
	cv::Vec<uchar, 3> inpaint_background(0, 128, 128);

	masked_image_view<color_type, mask_type> in_img(in, in_mask);
	image_view<color_type> out_img(out);

	cv::Mat_<color_type> in_img_mat = in_img.cv_mat();
	cv::Mat_<uchar> in_mask_mat = in_img.cv_mask_mat();
	cv::Mat_<color_type> out_img_mat = out_img.cv_mat();
	
	cv::Mat_<uchar> holes;
	cv::bitwise_not(in_mask_mat, holes);
	
	in_img_mat.setTo(inpaint_background, holes);
	cv::inpaint(in_img_mat, holes, out_img_mat, inpaint_radius, cv::INPAINT_NS);	
}

}
