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
#include <mf/image/image.h>
#include <mf/image/kernel.h>

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


void image_post_process_filter::process_frame
(const input_view_type& in, const output_view_type& out, job_type& job) {
	int kernel_diameter = 7;
	
	masked_image<color_type> in_img( job.in(input) );
	masked_image<color_type> out_img(out);
	
	out = in;

	cv::Mat_<uchar> bound;
	cv::bitwise_not(in_img.cv_mask_mat(), bound);
	
	if(job.param(right_side)) erode_right_bounds_(bound);
	else erode_left_bounds_(bound);
	
	auto kernel = to_opencv_mat( disk_image_kernel(kernel_diameter).view() );
	cv::dilate(bound, bound, kernel);
	
	cv::bitwise_and(in_img.cv_mask_mat(), bound, bound);

	for(auto c : make_ndspan(out.shape())) {
		if(bound(c[0], c[1])) out.at(c).set_flag(unstable_pixel_flag);
	}
}

}
