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

#include "depth_post_process.h"
#include <mf/image/masked_image_view.h>
#include <mf/filter/filter_job.h>
#include <mf/opencv.h>
#include "../common.h"

#include <mf/io/image_export.h>


namespace vs {

using namespace mf;

void depth_post_process_filter::configure(const json& j) {
	kernel_diameter.set_constant_value(j.value("kernel_diameter", 3));
	outer_iterations.set_constant_value(j.value("outer_iterations", 2));
	inner_smooth_iterations.set_constant_value(j.value("inner_smooth_iterations", 2));
}


void depth_post_process_filter::setup() {
	depth_output.define_frame_shape(depth_input.frame_shape());
	depth_mask_output.define_frame_shape(depth_input.frame_shape());
}


void depth_post_process_filter::process(job_type& job) {
	const int kernel_sz = job.param(kernel_diameter);
	const int iterations = job.param(outer_iterations);
	const int smooth_iterations = job.param(inner_smooth_iterations);

	auto in = job.in(depth_input);
	auto in_mask = job.in(depth_mask_input);
	auto out = job.out(depth_output);
	auto out_mask = job.out(depth_mask_output);
	
	masked_image_view<real_depth_type, mask_type> in_img(in, in_mask);
	masked_image_view<real_depth_type, mask_type> out_img(out, out_mask);
	
	cv::Mat_<uchar> holes;
	cv::Mat_<float> depth; // double?

	in_img.cv_mat().copyTo(depth);
	cv::bitwise_not(in_img.cv_mask_mat(), holes);
	depth.setTo(0.0, holes);
			
	for(int i = 0; i < iterations; ++i) {
		std::vector<cv::Mat_<uchar>> added_holes(smooth_iterations);
		cv::Mat_<float> smoothed_depth;

		for(int j = 0; j < smooth_iterations; ++j) {
			cv::Mat_<uchar> non_holes, smoothed_holes;
			cv::bitwise_not(holes, non_holes);
			cv::medianBlur(holes, smoothed_holes, kernel_sz);
			cv::bitwise_and(non_holes, smoothed_holes, added_holes[j]);
			smoothed_holes.setTo(0, added_holes[j]);
			smoothed_holes.copyTo(holes);
		}
		
		for(int j = 0; j < smooth_iterations; ++j) {
			cv::medianBlur(depth, smoothed_depth, kernel_sz);

			depth.copyTo(smoothed_depth, added_holes[j]);
			smoothed_depth.copyTo(depth);
		}
	}
	
	depth.copyTo(out_img.cv_mat());
	cv::bitwise_not(holes, out_img.cv_mask_mat());
	
	//image_export(make_masked_image_view<real_depth_type, mask_type>(out, out_mask), "img/" + this_filter().name()+".png");
}

}
