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

#include "image_post_process.h"
#include <mf/opencv.h>
#include <mf/filter/filter_job.h>
#include <mf/image/masked_image_view.h>
#include <mf/image/kernel.h>

#include <mf/io/image_export.h>

namespace vs {

using namespace mf;

void image_post_process_filter::erode_left_bounds_(cv::Mat_<uchar>& mat) {
	for(std::ptrdiff_t y = 0; y < mat.size[0]; ++y)
	for(std::ptrdiff_t x = 0; x < mat.size[1]; ++x) {
		if(mat(y, x)) {
			std::ptrdiff_t start_x = x;
			while( mat(y, x) && (x < mat.size[1]) ) {
				mat(y, x) = false;
				++x;
			}
			mat(y, start_x) = true;
		}
	}
}


void image_post_process_filter::erode_right_bounds_(cv::Mat_<uchar>& mat) {
	for(std::ptrdiff_t y = 0; y < mat.size[0]; ++y)
	for(std::ptrdiff_t x = 0; x < mat.size[1]; ++x) {
		if(mat(y, x)) {
			while( mat(y, x) && (x < mat.size[1]) ) {
				mat(y, x) = false;
				++x;
			}
			mat(y, x - 1) = true;
		}
	}	
}


void image_post_process_filter::setup() {
	image_output.define_frame_shape(image_input.frame_shape());
	image_mask_output.define_frame_shape(image_input.frame_shape());
}


void image_post_process_filter::process(job_type& job) {
	int kernel_diameter = 12;

	auto in = job.in(image_input);
	auto in_mask = job.in(image_mask_input);
	auto out = job.out(image_output);
	auto out_mask = job.out(image_mask_output);
	
	masked_image_view<color_type, mask_type> in_img(in, in_mask);
	masked_image_view<color_type, tri_mask_type> out_img(out, out_mask);
/*
	static_assert(tri_mask_clear == mask_clear, "tri_mask_clear == mask_clear");
	static_assert(tri_mask_stable == mask_set, "tri_mask_stable == mask_set");

	cv::Mat_<uchar> bound;
	cv::bitwise_not(in_img.cv_mask_mat(), bound);

	if(job.param(right_side)) erode_right_bounds_(bound);
	else erode_left_bounds_(bound);

	auto kernel = to_opencv( disk_image_kernel(kernel_diameter).view() );
	cv::dilate(bound, bound, kernel);

	cv::bitwise_and(in_img.cv_mask_mat(), bound, bound);
*/
	out = in;
	out_mask = in_mask;
	//out_img.cv_mask_mat().setTo(tri_mask_unstable, bound);
}


}
