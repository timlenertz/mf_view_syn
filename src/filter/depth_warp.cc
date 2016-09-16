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

#include "depth_warp.h"
#include <algorithm>

#include <mf/filter/filter_job.h>
#include <mf/image/masked_image_view.h>
#include <mf/io/image_export.h>


namespace vs {

using namespace mf;

void depth_warp_filter::setup() {
	depth_output.define_frame_shape(depth_input.frame_shape());
	depth_mask_output.define_frame_shape(depth_input.frame_shape());
}


void depth_warp_filter::process(job_type& job) {
	auto source_cam = job.param(source_camera);
	auto dest_cam = job.param(destination_camera);

	auto source_depth_in = job.in(depth_input);
	auto dest_depth_out = job.out(depth_output);
	auto dest_depth_mask_out = job.out(depth_mask_output);
	
	Eigen_projective3 homography = homography_transformation(source_cam, dest_cam);
	
	std::fill(dest_depth_mask_out.begin(), dest_depth_mask_out.end(), mask_clear);

	for(auto source_pix_coord : make_ndspan(source_depth_in.shape())) {
		integral_depth_type source_pix_depth = source_depth_in.at(source_pix_coord);

		real source_depth = source_cam.to_depth(source_pix_depth);
		auto source_coord = source_cam.to_image(source_pix_coord);

		Eigen_vec3 source_3coord(source_coord[0], source_coord[1], source_depth);
		Eigen_vec3 dest_3coord = (homography * source_3coord.homogeneous()).eval().hnormalized();
	
		real dest_depth = dest_3coord[2];

		auto dest_pix_coord = dest_cam.to_pixel(dest_3coord.head(2));

		if(dest_cam.image_span().includes(dest_pix_coord)) {
			real_depth_type& output_dest_depth = dest_depth_out.at(dest_pix_coord);
			mask_type& output_dest_depth_mask = dest_depth_mask_out.at(dest_pix_coord);
			if(output_dest_depth_mask == mask_clear || dest_depth > output_dest_depth) {
				output_dest_depth = dest_depth;
				output_dest_depth_mask = mask_set;
			}
		}
	}
}


}
