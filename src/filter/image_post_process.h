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
