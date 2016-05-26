#ifndef VW_FILTER_IMAGE_REVERSE_WARP_H_
#define VW_FILTER_IMAGE_REVERSE_WARP_H_

#include <mf/filter/filter.h>
#include "../common.h"

namespace vs {

class image_reverse_warp_filter : public mf::flow::filter {
private:
	mf::ndsize<2> shape_;

public:
	input_type<2, color_type> source_image_input;
	input_type<2, masked_real_depth_type> destination_depth_input;
	output_type<2, masked_color_type> destination_image_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;

	image_reverse_warp_filter(node_type& nd) :
		filter(nd),
		source_image_input(*this),
		destination_depth_input(*this),
		destination_image_output(*this) { }
	
	void setup() override;
	void process(job_type& job) override;	
};

}

#endif
