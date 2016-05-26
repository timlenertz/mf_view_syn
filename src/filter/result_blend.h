#ifndef VW_FILTER_RESULT_BLEND_H_
#define VW_FILTER_RESULT_BLEND_H_

#include <mf/filter/filter.h>
#include <utility>
#include "../common.h"

namespace vs {

class result_blend_filter : public mf::flow::filter {
private:
	mf::ndsize<2> shape_;
	
	std::pair<mf::real, mf::real> weights_(job_type& job) const;

public:
	input_type<2, masked_color_type> left_image_input;
	input_type<2, masked_color_type> right_image_input;
	output_type<2, masked_color_type> virtual_image_output;
	parameter_type<camera_type> left_source_camera;
	parameter_type<camera_type> right_source_camera;
	parameter_type<camera_type> virtual_camera;

	result_blend_filter(node_type& nd) :
		filter(nd),
		left_image_input(*this),
		right_image_input(*this),
		virtual_image_output(*this) { }

	void setup() override;
	void process(job_type& job) override;	
};

}

#endif
