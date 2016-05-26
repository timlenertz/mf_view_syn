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
	
	masked_image<real_depth_type> in_img( job.in(input) );
	masked_image<real_depth_type> out_img(out);
	
	auto input_mask = in_img.cv_mask_mat();
	auto input_depth = in_img.cv_mat();

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
			smoothed_holes.setTo(false, added_holes[j]);
			cv::bitwise_not(smoothed_holes, non_holes);
		}
		
		for(int j = 0; j < smooth_iterations; ++j) {
			cv::medianBlur(depth, smoothed_depth, kernel_size);
			depth.copyTo(smoothed_depth, added_holes[j]);
			smoothed_depth.copyTo(depth);
		}
	}
	
	non_holes.copyTo(out_img.cv_mask_mat());
	depth.copyTo(out_img.cv_mat());
	
	out_img.commit_cv_mat();
}

}
