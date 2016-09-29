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
	Assert(input_branches_.size() >= 2);
	shape_ = input_branches_.front()->image_input.frame_shape();
	virtual_image_output.define_frame_shape(shape_);
	virtual_mask_output.define_frame_shape(shape_);
	/*
	for(auto&& br : input_branches_) {
		br->image_input.set_activated(false);
		br->depth_input.set_activated(false);
		br->mask_input.set_activated(false);
	}
	*/
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
	real threshold_depth_difference = job.param(color_blending_maximal_depth_difference);
	bool do_color_blending = job.param(color_blending);
	bool do_depth_based_blending = input_branches_.front()->depth_input.is_connected() && (threshold_depth_difference > 0);

	// select branches that are active
	// selection = list of branches, with
	// - image, depth, mask input warped from one source camera to virtual camera
	// - distance of that source camera to virtual camera
	input_branch_selection sel = select_branches_(job);
	Assert(sel.entries.size() == N);
	constexpr std::size_t n = N;

	// get input view (for each selected branch) and output view
	std::vector<ndarray_view<2, rgb_color>> image_in;
	std::vector<ndarray_view<2, tri_mask_type>> mask_in;
	std::vector<ndarray_view<2, real_depth_type>> depth_in;
	image_in.reserve(n); depth_in.reserve(n); mask_in.reserve(n);
	for(const auto& selected_entry : sel.entries) {
		image_in.push_back(job.in(selected_entry.branch.image_input));
		mask_in.push_back(job.in(selected_entry.branch.mask_input));
		if(do_depth_based_blending) depth_in.push_back(job.in(selected_entry.branch.depth_input));
	}
	auto virtual_out = job.out(virtual_image_output);
	auto virtual_out_mask = job.out(virtual_mask_output);

	// arrays will hold input color, depth and mask of current pixel, for each selected branch
	std::vector<rgb_color> in_color;
	std::vector<tri_mask_type> in_mask;
	in_color.reserve(n); in_mask.reserve(n);

	// arrays will hold branch indices (`ent`) of selected entries where pixel is stable/unstable
	std::vector<std::ptrdiff_t> stables, unstables;
	stables.reserve(n); unstables.reserve(n);
	
	// for each pixel in the image...
	for(auto coord : make_ndspan(shape_)) {
		// fill in_color, in_depth, in_mask and stables, unstables
		// collect indices of selected entries where pixel mask is stable/unstable
		// `ent` variable refers to index of selected entry
		stables.clear();
		unstables.clear();
		for(std::ptrdiff_t ent = 0; ent < n; ++ent) {			
			in_color[ent] = image_in[ent].at(coord);
			
			tri_mask_type msk = mask_in[ent].at(coord);
			if(msk == tri_mask_unstable) unstables.push_back(ent);
			else if(msk == tri_mask_stable) stables.push_back(ent);
			in_mask[ent] = msk;
		}
		
		if(stables.size() == 0 && unstables.size() == 0) {
			virtual_out_mask.at(coord) = mask_clear;
			continue;
		}

		
		// acceptables = indices of selected entries that will be used to deduce virtual color
		// normally only stables, except if there are only unstables
		std::vector<ptrdiff_t> acceptables;
		acceptables.reserve(n);
		if(stables.size() > 0) acceptables = stables;
		else acceptables = unstables;
		acceptables = stables;
	
	
		
		// if doing depth blending: filter out acceptables that are too far off
		// (where difference with maximal depth > threshold)
		if(do_depth_based_blending) {
			// get acceptable with maximal depth (largest value = closest to camera)
			// _depth_ = disparity, measures proximity to virtual camera
			std::ptrdiff_t max_d_ent = -1;
			real max_d = 0;
			for(std::ptrdiff_t ent : acceptables) {
				real d = depth_in[ent].at(coord);
				if(max_d_ent == -1 || d > max_d) {
					max_d_ent = ent;
					max_d = d;
				}
			}

			// filter out acceptables
			std::vector<ptrdiff_t> close_acceptables; close_acceptables.reserve(n);
			for(std::ptrdiff_t ent : acceptables) {
				real d = depth_in[ent].at(coord);
				if(max_d - d < threshold_depth_difference) close_acceptables.push_back(ent);
			}
			acceptables = close_acceptables;
		}
		
		if(acceptables.size() == 1) {
			virtual_out.at(coord) = in_color[acceptables.front()];
			virtual_out_mask.at(coord) = mask_set;
		
		} else if(do_color_blending) {
			// color blending being used
			
			// get maximal distance of remaining acceptables
			// _distance_ = euclidian distance of branch's source camera to virtual camera
			real max_distance = 0, distance_sum = 0;
			for(std::ptrdiff_t ent : acceptables) {
				real distance = sel.entries[ent].distance;
				if(distance > max_distance) max_distance = distance;
				distance_sum += distance;
			}
			
			// compute normalized weight for each acceptable
			// calculate from max_distance/distance[ent]
			std::vector<real> acceptable_weights; // uses indices as acceptables vector
			real acceptable_weights_sum = 0.0;
			for(std::ptrdiff_t ent : acceptables) {
				real weight = distance_sum - sel.entries[ent].distance; // TODO revise formula
				acceptable_weights.push_back(weight);
				acceptable_weights_sum += weight;
			}
	
			// now compute RGB color
			real r = 0, g = 0, b = 0;
			for(std::ptrdiff_t ent : acceptables) {
				real w = acceptable_weights[ent];
				w = 1.0;
				r += w * in_color[ent].r;
				g += w * in_color[ent].g;
				b += w * in_color[ent].b;
			}
			acceptable_weights_sum = acceptable_weights.size();
			r = clamp(r / acceptable_weights_sum, 0.0, 255.0);
			g = clamp(g / acceptable_weights_sum, 0.0, 255.0);
			b = clamp(b / acceptable_weights_sum, 0.0, 255.0);
			
			virtual_out.at(coord) = rgb_color(r, g, b);
			virtual_out_mask.at(coord) = mask_set;


		} else {
			// no color blending to be used
			// select acceptable with maximal weight
			// weight = distance_sum/its_distance --> select the one with minimal distance
			real min_distance = 0;
			std::ptrdiff_t min_distance_ent = 0;
			for(std::ptrdiff_t ent : acceptables) {
				min_distance_ent = ent;
				if(sel.entries[ent].distance < min_distance) min_distance = sel.entries[ent].distance;
			}
			
			// assign virtual color
			virtual_out.at(coord) = in_color[min_distance_ent];
			virtual_out_mask.at(coord) = mask_set;
		}
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
