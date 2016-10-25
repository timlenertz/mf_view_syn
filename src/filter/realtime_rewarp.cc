#include "realtime_rewarp.h"

namespace vs {

using namespace mf;

void realtime_rewarp::setup() {
	image_output.define_frame_shape(image_input.frame_shape());
}


void realtime_rewarp::process(job_type& job) {	
	auto source_cam = job.param(source_camera);
	auto dest_cam = job.param(destination_camera);

	auto source_image_in = job.in(image_input);
	auto source_depth_in = job.in(depth_input);
	auto dest_image_out = job.out(image_output);
	
	Eigen_projective3 homography = homography_transformation(source_cam, dest_cam);
		
	dest_image_out = source_image_in;

	ndarray<2, real_depth_type> depth_buffer(image_input.frame_shape());
	std::fill(depth_buffer.begin(), depth_buffer.end(), 0);

	for(auto source_pix_coord : make_ndspan(source_image_in.shape())) {
		integral_depth_type source_pix_depth = source_depth_in.at(source_pix_coord);

		real source_depth = source_cam.to_depth(source_pix_depth);
		if(source_depth == 0.0) continue;
		auto source_coord = source_cam.to_image(source_pix_coord);

		Eigen_vec3 source_3coord(source_coord[0], source_coord[1], source_depth);
		Eigen_vec3 dest_3coord = (homography * source_3coord.homogeneous()).eval().hnormalized();
	
		real dest_depth = dest_3coord[2];

		auto dest_pix_coord = dest_cam.to_pixel(dest_3coord.head(2));
	
		if(dest_cam.image_span().includes(dest_pix_coord)) {
			const rgb_color& source_pix_color = source_image_in.at(source_pix_coord);
			
			real_depth_type& buffered_dest_depth = depth_buffer.at(dest_pix_coord);
			rgb_color& output_dest_color = dest_image_out.at(dest_pix_coord);
			
			if(dest_depth > buffered_dest_depth) {
				output_dest_color = source_pix_color;
				buffered_dest_depth = dest_depth;
			}
		}
	}
	
	job.send_param(source_camera, dest_cam);
}

}
