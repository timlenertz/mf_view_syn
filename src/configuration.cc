#include "configuration.h"
#include "rs_camera_array.h"
#include <mf/geometry/math_constants.h>
#include <mf/geometry/depth_projection_parameters.h>
#include <string>
#include <cmath>
#include <fstream>

namespace vs {

using namespace mf;
using namespace std::literals;

rs_camera_array& configuration::camera_array_() const {
	if(camera_arr_) return *camera_arr_;
			
	camera_arr_.reset(new rs_camera_array(
		json_["input"]["camera_description_file"],
		depth_projection_(),
		input_shape()
	));
	return *camera_arr_;
}


depth_projection_parameters configuration::depth_projection_() const {
	depth_projection_parameters dparam;
	dparam.z_near = json_["input"]["depth"]["z_near"];
	dparam.z_far = json_["input"]["depth"]["z_far"];
	dparam.flip_z = json_["input"]["depth"]["z_flipped"];
	
	if(json_["input"]["depth"]["type"].get<std::string>() == "disparity")
		dparam.range = depth_projection_parameters::unsigned_normalized_disparity;
	else
		throw configuration_error("invalid input depth type: " + json_["input"]["depth"]["type"].get<std::string>());

	return dparam;
}


configuration::configuration(const std::string& filename) {		
	std::ifstream fstr(filename);
	fstr >> json_;
}


configuration::~configuration() { }

ndsize<2> configuration::input_shape() const {
	return make_ndsize(
		json_["input"]["width"],
		json_["input"]["height"]
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


bool configuration::output_rgb() const {
	if(json_["output"]["raw_color"].get<std::string>() == "rgb") return true;
	else if(json_["output"]["raw_color"].get<std::string>() == "ycbcr") return false;
	else throw configuration_error("invalid output raw color: " + json_["output"]["raw_color"].get<std::string>());
}


raw_video_frame_format<rgb_color> configuration::output_rgb_raw_format() const {
	if(json_["output"]["raw_sampling"].get<std::string>() == "planar_444")
		return raw_video_frame_formats::planar_rgb();
	else if(json_["output"]["raw_sampling"].get<std::string>() == "interleaved")
		return raw_video_frame_formats::interleaved_rgb();
	else
		throw configuration_error("invalid output raw sampling: " + json_["output"]["raw_sampling"].get<std::string>());
}


raw_video_frame_format<ycbcr_color> configuration::output_ycbcr_raw_format() const {
	if(json_["output"]["raw_sampling"].get<std::string>() == "planar_444")
		return raw_video_frame_formats::planar_ycbcr_4_4_4();
	else if(json_["output"]["raw_sampling"].get<std::string>() == "planar_420")
		return raw_video_frame_formats::planar_ycbcr_4_2_0();
	else if(json_["output"]["raw_sampling"].get<std::string>() == "interleaved")
		return raw_video_frame_formats::interleaved_ycbcr();
	else
		throw configuration_error("invalid output raw sampling: " + json_["output"]["raw_sampling"].get<std::string>());
}


///////////////


camera_type configuration::virtual_camera_functor::operator()(time_unit frame_index) const {
	camera_type left_cam = configuration_.input_view_at(0).camera;
	
	pose left = left_cam.absolute_pose();
	pose right = configuration_.input_view_at(configuration_.input_views_count() - 1).camera.absolute_pose();
	
	real n = 100;
	n /= 1.3;
	real t = -std::cos(frame_index * pi / n) / 2.0 + 0.5;
	pose virtual_pose = interpolate(left, right, t);
	virtual_pose.orientation = left.orientation.slerp(t, right.orientation);

	camera_type virtual_cam = left_cam;
	virtual_cam.set_relative_pose(virtual_pose);
	return virtual_cam;
}


}
