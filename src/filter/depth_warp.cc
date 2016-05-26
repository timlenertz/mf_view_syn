#include "depth_warp.h"
#include <algorithm>

namespace vs {

using namespace mf;

void depth_warp_filter::process_frame
(const input_view_type& source_depth_in, const output_view_type& dest_depth_out, job_type& job) {
	auto source_cam = job.param(source_camera);
	auto dest_cam = job.param(destination_camera);

	std::fill(dest_depth_out.begin(), dest_depth_out.end(), masked_real_depth_type::null());

	Eigen_projective3 homography = homography_transformation(source_cam, dest_cam);

	for(auto source_pix_coord : make_ndspan(source_depth_in.shape())) {
		masked_real_depth_type source_pix_depth = source_depth_in.at(source_pix_coord);
		if(source_pix_depth.is_null()) continue;

		real source_depth = source_cam.to_depth(source_pix_depth);
		auto source_coord = source_cam.to_image(source_pix_coord);

		Eigen_vec3 source_3coord(source_coord[0], source_coord[1], source_depth);
		Eigen_vec3 dest_3coord = (homography * source_3coord.homogeneous()).eval().hnormalized();
	
		real dest_depth = dest_3coord[2];

		auto dest_pix_coord = dest_cam.to_pixel(dest_3coord.head(2));

		if(dest_cam.image_span().includes(dest_pix_coord)) {
			masked_real_depth_type& output_dest_depth = dest_depth_out.at(dest_pix_coord);
			if(output_dest_depth.is_null() || dest_depth > output_dest_depth.elem)
				output_dest_depth = dest_depth;
		}
	}
}

}
