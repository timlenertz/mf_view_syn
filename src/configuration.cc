#include "configuration.h"
#include "rs_camera_array.h"
#include <mf/geometry/math_constants.h>
#include <string>
#include <cmath>
#include <fstream>

namespace vs {

using namespace mf;

rs_camera_array& configuration::camera_array_() const {
	if(camera_arr_) return *camera_arr_;
	
	depth_projection_parameters dparam;
	dparam.z_near = json_["input"]["format"]["depth_z_near"];
	dparam.z_far = json_["input"]["format"]["depth_z_far"];
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;
		
	camera_arr_.reset(new rs_camera_array(
		json_["input"]["camera_description_file"],
		dparam,
		input_shape()
	));
	return *camera_arr_;
}


configuration::configuration(const std::string& filename) {		
	std::ifstream fstr(filename);
	fstr >> json_;
}


configuration::~configuration() { }

ndsize<2> configuration::input_shape() const {
	return make_ndsize(
		json_["input"]["format"]["width"],
		json_["input"]["format"]["height"]
	);
}

std::size_t configuration::input_views_count() const {
	return json_["input"]["views"].size();
}


auto configuration::input_view_at(std::ptrdiff_t i) const -> input_view {
	auto vw = json_["input"]["views"][i];
	rs_camera_array& cam_arr = camera_array_();
	
	auto shape = input_shape();
		
	std::string cam_name = vw["camera_name"];
	camera_type cam(cam_arr[cam_name], shape);
	cam.flip_pixel_coordinates();
	
	return {
		std::to_string(i),
		cam,
		vw["image_sequence_file"],
		vw["depth_sequence_file"]
	};
}


///////////////


camera_type configuration::virtual_camera_functor::operator()(time_unit frame_index) const {
	camera_type left_cam = configuration_.input_view_at(0).camera;
	
	pose left = left_cam.absolute_pose();
	pose right = configuration_.input_view_at(configuration_.input_views_count() - 1).camera.absolute_pose();
	
	real n = configuration_["input"]["format"]["number_of_frames"];
	n /= 3.3;
	real t = std::sin(frame_index * pi / n) / 2.0 + 0.5;
	pose virtual_pose = interpolate(left, right, t);
	
	std::cout << t << std::endl;
	
	depth_projection_parameters dparam;
	dparam.z_near = configuration_["input"]["format"]["depth_z_near"];
	dparam.z_far = configuration_["input"]["format"]["depth_z_far"];
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;

	camera_type virtual_cam = left_cam;
	virtual_cam.set_relative_pose(virtual_pose);
	return virtual_cam;
}


}
