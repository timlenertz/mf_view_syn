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

#ifndef VW_FILTER_RESULT_BLEND_H_
#define VW_FILTER_RESULT_BLEND_H_

#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include <utility>
#include <vector>
#include <memory>
#include "../common.h"

namespace vs {

class result_blend_filter : public mf::flow::filter {
public:
	struct input_branch {
		input_type<2, color_type> image_input;
		input_type<2, real_depth_type> depth_input;
		input_type<2, tri_mask_type> mask_input;
		parameter_type<camera_type> source_camera;
		
		input_branch(result_blend_filter& filt, const std::string& name) :
			image_input(filt),
			depth_input(filt),
			mask_input(filt),
			source_camera(filt)
		{
			image_input.set_name(name + " im");
			depth_input.set_name(name + " di");
			mask_input.set_name(name + " mask");
			source_camera.set_name(name + " cam");
		}
	};
	
private:
	struct input_branch_selection {
		input_branch& left;
		mf::real left_weight;
		input_branch& right;
		mf::real right_weight;
		
		bool selected(const input_branch& br) const {
			return (&br == &left) || (&br == &right);
		}
	};
	
	std::vector<std::unique_ptr<input_branch>> input_branches_;
	
	input_branch_selection select_branches_(job_type& job) const;

public:
	output_type<2, color_type> virtual_image_output;
	output_type<2, mask_type> virtual_mask_output;
	parameter_type<camera_type> virtual_camera;
	
	mf::ndsize<2> shape_;
	bool depth_blending = true;
	mf::real depth_blending_minimal_difference = 5.0;

	result_blend_filter() :
		virtual_image_output(*this),
		virtual_mask_output(*this),
		virtual_camera(*this)
	{
		virtual_image_output.set_name("im");
		virtual_mask_output.set_name("im mask");
		virtual_camera.set_name("virtual cam");
	}
	
	input_branch& add_input_branch(const std::string& name);

	void setup() override;
	void pre_process(job_type& job) override;	
	void process(job_type& job) override;	
};

}

#endif
