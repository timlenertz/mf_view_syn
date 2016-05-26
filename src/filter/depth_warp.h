#ifndef VW_FILTER_DEPTH_WARP_H_
#define VW_FILTER_DEPTH_WARP_H_

#include <mf/filter/simple_filter.h>
#include <mf/masked_elem.h>
#include "../common.h"

namespace vs {

class depth_warp_filter : public mf::flow::simple_filter<2, integral_depth_type, masked_real_depth_type> {
public:
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;

	using simple_filter::simple_filter;

	void process_frame(const input_view_type& in, const output_view_type& out, job_type& job) override;	
};
	
}

#endif
