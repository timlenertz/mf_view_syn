#ifndef VW_FILTER_RESULT_POST_PROCESS_H_
#define VW_FILTER_RESULT_POST_PROCESS_H_

#include <mf/filter/simple_filter.h>
#include <mf/masked_elem.h>
#include "../common.h"

namespace vs {

class result_post_process_filter : public mf::flow::simple_filter<2, masked_color_type, color_type> {
public:
	using simple_filter::simple_filter;

	void process_frame(const input_view_type& in, const output_view_type& out, job_type& job) override;	
};
	
}

#endif
