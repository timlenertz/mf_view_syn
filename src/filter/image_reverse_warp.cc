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

#include "image_reverse_warp.h"

#include <mf/filter/filter_job.h>
#include <mf/io/image_export.h>


namespace vs {

using namespace mf;

void image_reverse_warp_filter::setup() {
	Assert(source_image_input.frame_shape() == destination_depth_input.frame_shape());
	shape_ = source_image_input.frame_shape();
	destination_image_output.define_frame_shape(shape_);
	destination_image_mask_output.define_frame_shape(shape_);
}


void image_reverse_warp_filter::process(job_type& job) {
	auto source_image_in = job.in(source_image_input);
	auto dest_depth_in = job.in(destination_depth_input);
	auto dest_depth_mask_in = job.in(destination_depth_mask_input);
		
	auto dest_image_out = job.out(destination_image_output);
	auto dest_image_mask_out = job.out(destination_image_mask_output);
	
	auto source_cam = job.param(source_camera);
	auto dest_cam = job.param(destination_camera);


	Eigen_projective3 reverse_homography = homography_transformation(dest_cam, source_cam);
	
	for(auto dest_pix_coord : make_ndspan(shape_)) {
		real_depth_type dest_depth = dest_depth_in.at(dest_pix_coord);
		mask_type dest_depth_mask = dest_depth_mask_in.at(dest_pix_coord);
		if(dest_depth_mask == mask_clear) {
			dest_image_mask_out.at(dest_pix_coord) = mask_clear;
			continue;
		}

		auto dest_coord = dest_cam.to_image(dest_pix_coord);

		Eigen_vec3 dest_3coord(dest_coord[0], dest_coord[1], dest_depth);
		Eigen_vec3 source_3coord = (reverse_homography * dest_3coord.homogeneous()).eval().hnormalized();

		auto source_pix_coord = source_cam.to_pixel(source_3coord.head(2));

		if(source_cam.image_span().includes(source_pix_coord)) {
			color_type source_color = source_image_in.at(source_pix_coord);
			dest_image_out.at(dest_pix_coord) = source_color;
			dest_image_mask_out.at(dest_pix_coord) = mask_set;
		} else {
			dest_image_mask_out.at(dest_pix_coord) = mask_clear;
		}
	}
}


}
