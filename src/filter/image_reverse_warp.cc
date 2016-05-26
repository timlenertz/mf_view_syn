#include "image_reverse_warp.h"

namespace vs {

using namespace mf;

void image_reverse_warp_filter::setup() {
	MF_ASSERT(source_image_input.frame_shape() == destination_depth_input.frame_shape());
	shape_ = source_image_input.frame_shape();
	destination_image_output.define_frame_shape(shape_);
}


void image_reverse_warp_filter::process(job_type& job) {
	auto dest_image_out = job.out(destination_image_output);
	auto dest_depth_in = job.in(destination_depth_input);
	auto source_image_in = job.in(source_image_input);
	auto source_cam = job.param(source_camera);
	auto dest_cam = job.param(destination_camera);

	Eigen_projective3 reverse_homography = homography_transformation(dest_cam, source_cam);
	
	for(auto dest_pix_coord : make_ndspan(shape_)) {
		masked_real_depth_type dest_depth = dest_depth_in.at(dest_pix_coord);
		if(dest_depth.is_null()) {
			dest_image_out.at(dest_pix_coord) = masked_color_type::null();
			continue;
		}

		auto dest_coord = dest_cam.to_image(dest_pix_coord);

		Eigen_vec3 dest_3coord(dest_coord[0], dest_coord[1], dest_depth);
		Eigen_vec3 source_3coord = (reverse_homography * dest_3coord.homogeneous()).eval().hnormalized();

		auto source_pix_coord = source_cam.to_pixel(source_3coord.head(2));

		if(source_cam.image_span().includes(source_pix_coord)) {
			color_type source_color = source_image_in.at(source_pix_coord);
			dest_image_out.at(dest_pix_coord) = source_color;
		} else {
			dest_image_out.at(dest_pix_coord) = masked_color_type::null();
		}
	}
}


}
