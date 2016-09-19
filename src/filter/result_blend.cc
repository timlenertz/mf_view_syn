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

#include "result_blend.h"
#include <tuple>
#include <algorithm>

#include <mf/filter/filter_job.h>
#include <mf/io/image_export.h>


namespace vs {

using namespace mf;


auto result_blend_filter::select_branches_(job_type& job) const -> input_branch_selection {
	Eigen_vec3 virtual_position = job.param(virtual_camera).absolute_pose().position;

	auto position = [&job](const input_branch& br) {
		return job.param(br.source_camera).absolute_pose().position;
	};
	auto distance = [&virtual_position, &position](const input_branch& br) {
		return (position(br) - virtual_position).norm();
	};
	auto cmp = [&distance](const input_branch* a, const input_branch* b) {
		return (distance(*a) < distance(*b));
	};
	
	// Set 2 branches whose source camera is closest to the virtual camera
	// (distances of camera centers are compared)
	std::vector<input_branch*> branches;
	for(auto&& br : input_branches_) branches.push_back(br.get());
	std::nth_element(branches.begin(), branches.begin() + 1, branches.end(), cmp);
	input_branch* left_branch = branches[0];
	input_branch* right_branch = branches[1];
	
	// Determine left/right (X coordinate)
	if(position(*left_branch)[0] > position(*right_branch)[0]) std::swap(left_branch, right_branch);
	
	// Calculate weights from distance
	real left_distance = distance(*left_branch);
	real right_distance = distance(*right_branch);
	real sum_distance = left_distance + right_distance;
	real left_weight = left_distance / sum_distance;
	real right_weight = right_distance / sum_distance;
	
	return input_branch_selection {
		*left_branch,
		left_weight,
		*right_branch,
		right_weight
	};
}


auto result_blend_filter::add_input_branch(const std::string& name) -> input_branch& {
	input_branch* br = new input_branch(*this, name);
	input_branches_.emplace_back(br);
	return *br;
}


void result_blend_filter::setup() {
	Assert(input_branches_.size() >= 2);
	shape_ = input_branches_.front()->image_input.frame_shape();
	virtual_image_output.define_frame_shape(shape_);
	virtual_mask_output.define_frame_shape(shape_);
	
	for(auto&& br : input_branches_) {
		br->image_input.set_activated(false);
		br->depth_input.set_activated(false);
		br->mask_input.set_activated(false);
	}
}


void result_blend_filter::pre_process(job_type& job) {
	input_branch_selection sel = select_branches_(job);
	for(auto&& br : input_branches_) {
		bool act = sel.selected(*br);
		br->image_input.set_activated(act);
		br->depth_input.set_activated(act);
		br->mask_input.set_activated(act);
	}
	job.send_param(sel.right.right_side_sent, true);
	job.send_param(sel.left.right_side_sent, false);
}


void result_blend_filter::process(job_type& job) {	
	input_branch_selection sel = select_branches_(job);

	auto virtual_out = job.out(virtual_image_output);
	auto virtual_out_mask = job.out(virtual_mask_output);
	auto left_image_in = job.in(sel.left.image_input);
	auto left_depth_in = job.in(sel.left.depth_input);
	auto left_mask_in = job.in(sel.left.mask_input);
	auto right_image_in = job.in(sel.right.image_input);
	auto right_depth_in = job.in(sel.right.depth_input);
	auto right_mask_in = job.in(sel.right.mask_input);
	
	real left_weight = sel.left_weight;
	real right_weight = sel.right_weight;
	bool prefer_left = (left_weight >= right_weight);
	bool prefer_right = (right_weight >= left_weight);
	
	for(auto coord : make_ndspan(shape_)) {
		color_type left_color = left_image_in.at(coord);
		real_depth_type left_depth = left_depth_in.at(coord);
		tri_mask_type left_mask = left_mask_in.at(coord);

		color_type right_color = right_image_in.at(coord);
		real_depth_type right_depth = right_depth_in.at(coord);
		tri_mask_type right_mask = right_mask_in.at(coord);
	
		color_type virtual_color;
		mask_type virtual_mask = mask_set;

		if(left_mask == tri_mask_clear && right_mask == tri_mask_clear) {
			virtual_mask = mask_clear;
		} else if(left_mask == tri_mask_clear || (left_mask == tri_mask_unstable && prefer_right)) {
			virtual_color = right_color;
		} else if(right_mask == tri_mask_clear || (right_mask == tri_mask_unstable && prefer_left)) {
			virtual_color = left_color;
		} else {
			bool has_depth = (left_mask == tri_mask_stable && right_mask == tri_mask_stable);
			real depth_difference = std::abs(left_depth - right_depth);
			if(has_depth && depth_blending && depth_difference > depth_blending_minimal_difference) {
				if(left_depth < right_depth) virtual_color = left_color;
				else virtual_color = right_color;
			} else {
				virtual_color = color_blend(left_color, left_weight, right_color, right_weight);
			}
		}
		
		virtual_out.at(coord) = virtual_color;
		virtual_out_mask.at(coord) = virtual_mask;
	}
}


}
