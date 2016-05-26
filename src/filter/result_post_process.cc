#include "result_post_process.h"
#include <algorithm>
#include <mf/opencv.h>
#include <mf/image/image.h>

namespace vs {

using namespace mf;

void result_post_process_filter::process_frame
(const input_view_type& in, const output_view_type& out, job_type& job) {
	double inpaint_radius = 5.0;
	
	masked_image<color_type> in_image( job.in(input) );
	image<color_type> out_image(out);

	cv::Mat_<color_type> in_img = in_image.cv_mat();
	cv::Mat_<uchar> in_mask = (in_image.cv_mask_mat() != 0);
	cv::Mat_<color_type> out_img = out_image.cv_mat();
	
	cv::Mat_<uchar> holes;
	cv::bitwise_not(in_mask, holes);
	
	cv::Vec<uchar, 3> inpaint_background(0, 128, 128);
	
	in_img.copyTo(out_img);
	out_img.setTo(inpaint_background, holes);
	
	cv::inpaint(in_img, holes, out_img, inpaint_radius, cv::INPAINT_NS);
	out_image.commit_cv_mat();
}

}
