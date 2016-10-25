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


bool result_blend_filter::input_branch_selection::selected(const input_branch& br) const {
	return std::any_of(entries.begin(), entries.end(), [&br](const entry& ent) {
		return (&ent.branch == &br);
	});
}

void result_blend_filter::input_branch_selection::add(input_branch& branch, mf::real distance) {
	entries.push_back({ branch, distance });
}


auto result_blend_filter::select_branches_(job_type& job) const -> input_branch_selection {
	input_branch_selection selection;
	selection.add(*input_branches_[0], 1.0);
	return selection;

	/*
	std::ptrdiff_t n = job.param(selected_inputs_count);
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
	
	// Set n branches whose source camera is closest to the virtual camera
	// (distances of camera centers are compared)
	std::vector<input_branch*> branches;
	for(auto&& br : input_branches_) branches.push_back(br.get());
	std::nth_element(branches.begin(), branches.begin() + n, branches.end(), cmp);
	
	// Assign weights and return selection
	input_branch_selection selection;
	for(std::ptrdiff_t i = 0; i < n; ++i)
		selection.add(*branches[i], distance(*branches[i]));

	return selection;
	*/
}


void result_blend_filter::configure(const json& j) {
	color_blending.set_constant_value(j.value("color_blending", true));
	color_blending_maximal_depth_difference.set_constant_value(j.value("color_blending_maximal_depth_difference", 0.0));
	selected_inputs_count.set_constant_value(j.value("selected_inputs_count", 2));
}


auto result_blend_filter::add_input_branch(const std::string& name) -> input_branch& {
	input_branch* br = new input_branch(*this, name);
	input_branches_.emplace_back(br);
	return *br;
}


void result_blend_filter::setup() {
	//Assert(input_branches_.size() >= 2);
	shape_ = input_branches_.front()->image_input.frame_shape();
	virtual_image_output.define_frame_shape(shape_);
	virtual_mask_output.define_frame_shape(shape_);
	virtual_depth_output.define_frame_shape(shape_);
}


void result_blend_filter::pre_process(job_type& job) {
	input_branch_selection sel = select_branches_(job);
	for(auto&& br : input_branches_) {
		bool act = sel.selected(*br);
		job.set_activated(br->image_input, act);
		if(input_branches_.front()->depth_input.is_connected()) job.set_activated(br->depth_input, act);
		job.set_activated(br->mask_input, act);
	}
}


template<std::size_t N>
void result_blend_filter::process_(job_type& job) {
	input_branch_selection sel = select_branches_(job);
	Assert(sel.entries.size() == N);
	constexpr std::size_t n = N;

	std::vector<ndarray_view<2, rgb_color>> image_in;
	std::vector<ndarray_view<2, real_depth_type>> depth_in;
	std::vector<ndarray_view<2, tri_mask_type>> mask_in;
	image_in.reserve(n); mask_in.reserve(n);
	for(const auto& selected_entry : sel.entries) {
		image_in.push_back(job.in(selected_entry.branch.image_input));
		depth_in.push_back(job.in(selected_entry.branch.depth_input));
		mask_in.push_back(job.in(selected_entry.branch.mask_input));
	}
	auto virtual_out = job.out(virtual_image_output);
	auto virtual_out_depth = job.out(virtual_depth_output);
	auto virtual_out_mask = job.out(virtual_mask_output);

	std::vector<rgb_color> in_color;
	std::vector<real> in_distance;
	in_color.reserve(n); in_distance.reserve(n);
	
	// for each pixel in the image...
	for(auto coord : make_ndspan(shape_)) {
		real r = 0, g = 0, b = 0;
		
		in_color.clear();
		in_distance.clear();
		
		real depth = 0;
		
		bool has_stables = false;
		for(std::ptrdiff_t ent = 0; ent < n; ++ent) {	
			tri_mask_type msk = mask_in[ent].at(coord);
			if(msk == tri_mask_stable) { has_stables = true; break; }
		}
		
		real distance_sum = 0, distance_max = 0;
		std::ptrdiff_t distance_max_ent = -1;
		for(std::ptrdiff_t ent = 0; ent < n; ++ent) {		
			tri_mask_type msk = mask_in[ent].at(coord);
			if(msk == tri_mask_clear) continue;
			else if(msk == tri_mask_unstable && has_stables) continue;
			
			in_color.push_back(image_in[ent].at(coord));
			
			depth = depth_in[ent].at(coord);
			
			real dist = sel.entries[ent].distance;
			in_distance.push_back(dist);
			distance_sum += dist;
			if(dist > distance_max) {
				distance_max = dist;
				distance_max_ent = ent;
			}
		}
		
		if(in_color.size() == 0) {
			virtual_out_mask.at(coord) = mask_clear;
			continue;
		}
				
		real weights_sum = 0;
		for(std::ptrdiff_t i = 0; i < in_color.size(); ++i) {
			real w;
			if(in_color.size() == 1) w = 1.0;
			else w = (distance_sum - in_distance[i])/distance_sum;
			
			r += w * in_color[i].r;
			g += w * in_color[i].g;
			b += w * in_color[i].b;
			weights_sum += w;
		}
					
		r = clamp(r / weights_sum, 0.0, 255.0);
		g = clamp(g / weights_sum, 0.0, 255.0);
		b = clamp(b / weights_sum, 0.0, 255.0);
		
		virtual_out.at(coord) = rgb_color(r, g, b);
		virtual_out_mask.at(coord) = mask_set;
		virtual_out_depth.at(coord) = depth;
	}
	
	//image_export(make_masked_image_view<rgb_color, mask_type>(virtual_out, virtual_out_mask), "img/" + name()+".png");

}


void result_blend_filter::process(job_type& job) {
	std::ptrdiff_t n = job.param(selected_inputs_count);
	switch(n) {
		case 1: process_<1>(job); break;
		case 2: process_<2>(job); break;
		case 3: process_<3>(job); break;
		case 4: process_<4>(job); break;
		default: throw std::runtime_error("unsupported selected view count");
	}
}


// input mask values:
// set = stable color value
// unstable = unstable color, prefer stable, take if no stable
// clear = no color value


}
