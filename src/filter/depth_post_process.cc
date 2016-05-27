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
#include <mf/image/image.h>
#include <mf/opencv.h>
#include "../common.h"

namespace vs {

using namespace mf;

void depth_post_process_filter::process_frame
(const input_view_type& in, const output_view_type& out, job_type& job) {	
	int kernel_size = 3;
	constexpr int iterations = 2, smooth_iterations = 2;
	
	masked_image<real_depth_type> img(in);
	
	auto input_mask = img.cv_mask_mat();
	auto input_depth = img.cv_mat();

	cv::Mat_<uchar> non_holes = input_mask;
	cv::Mat_<float> depth = input_depth;

			
	for(int i = 0; i < iterations; ++i) {
		cv::Mat added_holes[smooth_iterations];
		cv::Mat smoothed_holes, smoothed_depth;

		for(int j = 0; j < smooth_iterations; ++j) {
			cv::Mat holes;
			cv::bitwise_not(non_holes, holes);
			cv::medianBlur(holes, smoothed_holes, kernel_size);
			cv::bitwise_and(non_holes, smoothed_holes, added_holes[j]);
			smoothed_holes.setTo(0, added_holes[j]);
			cv::bitwise_not(smoothed_holes, non_holes);
		}
		
		for(int j = 0; j < smooth_iterations; ++j) {
			cv::medianBlur(depth, smoothed_depth, kernel_size);
			depth.copyTo(smoothed_depth, added_holes[j]);
			smoothed_depth.copyTo(depth);
		}
	}
	
	img.write(out);
}

}
