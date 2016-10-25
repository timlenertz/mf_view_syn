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
#include <mf/filter/filter_job.h>
#include <mf/opencv.h>
#include <mf/image/masked_image_view.h>

#include <mf/io/image_export.h>

#include <thread>
#include <chrono>

namespace vs {

using namespace mf;

void result_post_process_filter::configure(const json& j) {
	inpaint_radius.set_constant_value(j.value("inpaint_radius", 3.0));
	
	rgb_color bg(0, 128, 128);
	if(j.count("inpaint_background") != 0) {
		auto j_bg = j["inpaint_background"];
		bg.r = j_bg[0];
		bg.g = j_bg[1];
		bg.b = j_bg[2];
	}
	inpaint_background.set_constant_value(bg);
}


void result_post_process_filter::setup() {
	image_output.define_frame_shape(image_input.frame_shape());
	image_depth_output.define_frame_shape(image_input.frame_shape());
}


void result_post_process_filter::process(job_type& job) {
	job.out(image_depth_output)=job.in(image_depth_input);
	
	auto in = job.in(image_input);
	auto in_mask = job.in(image_mask_input);
	auto out = job.out(image_output);	
	
	double radius = job.param(inpaint_radius);
	cv::Vec<uchar, 3> background;
	background[0] = job.param(inpaint_background).r;
	background[1] = job.param(inpaint_background).g;
	background[2] = job.param(inpaint_background).b;

	masked_image_view<color_type, mask_type> in_img(in, in_mask);
	image_view<color_type> out_img(out);

	cv::Mat_<color_type> in_img_mat = in_img.cv_mat();
	cv::Mat_<uchar> in_mask_mat = in_img.cv_mask_mat();
	cv::Mat_<color_type> out_img_mat = out_img.cv_mat();
	
	cv::Mat_<uchar> holes;
	cv::bitwise_not(in_mask_mat, holes);
	
	in_img_mat.setTo(background, holes);
	out = in;
	//cv::inpaint(in_img_mat, holes, out_img_mat, radius, cv::INPAINT_NS);	
}

}
