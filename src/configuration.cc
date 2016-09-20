#include "configuration.h"
#include "rs_camera_array.h"
#include <mf/geometry/math_constants.h>
#include <mf/geometry/depth_projection_parameters.h>
#include <mf/utility/string.h>
#include <mf/utility/misc.h>
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
		scaled_shape(),
		json_["input"].value("scale", 1.0)
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

ndsize<2> configuration::scaled_shape() const {
	ndsize<2> in_shape = input_shape();
	if(json_["input"].count("scale") != 0) {
		double factor = json_["input"]["scale"];
		if(factor != 1.0) return make_ndsize(factor * in_shape[0], factor * in_shape[1]);
	}
	return in_shape;
}


std::size_t configuration::input_views_count() const {
	auto views = json_["input"]["views"];
	if(views.is_array()) {
		return views.size();
	} else if(views.is_object()) {
		int count = views["to"].get<int>() - views["from"].get<int>();
		if(views.count("step") != 0) count /= views["step"].get<int>();
		return count;
	} else {
		throw configuration_error("input views invalid");
	}
}


auto configuration::input_view_at(std::ptrdiff_t i) const -> input_view {
	std::string image_sequence_file, depth_sequence_file, camera_name;
	std::string name = std::to_string(i);

	auto views = json_["input"]["views"];
	if(views.is_array()) {
		auto view = json_["input"]["views"][i];
		image_sequence_file = view["image_sequence_file"];
		depth_sequence_file = view["depth_sequence_file"];
		camera_name = view["camera_name"];
		if(view.count("name") != 0) name = view["name"];
		
	} else if(views.is_object()) {
		int from = views["from"], to = views["to"];
		int step = 1;
		if(views.count("step") != 0) step = views["step"];
		
		int index = from + (i * step);
		if(index < from || index > to) throw std::out_of_range("input view out of range");
		
		std::string placeholder = views["placeholder"];
		std::string index_str = std::to_string(index);
		if(views.count("placeholder_length") != 0) {
			int placeholder_length = views["placeholder_length"];
			if(index_str.length() < placeholder_length)
				index_str = std::string(placeholder_length - index_str.length(), '0') + index_str;
		}
		
		image_sequence_file = replace_all(views["image_sequence_file"], placeholder, index_str);
		depth_sequence_file = replace_all(views["depth_sequence_file"], placeholder, index_str);
		camera_name = replace_all(views["camera_name"], placeholder, index_str);
		image_sequence_file = replace_all(views["image_sequence_file"], placeholder, index_str);
		if(views.count("name") != 0) name = replace_all(views["name"], placeholder, index_str);

	} else {
		throw configuration_error("input views invalid");
	
	} 

	if(! file_exists(image_sequence_file)) throw configuration_error("input view image sequence file does not exist");
	if(! file_exists(depth_sequence_file)) throw configuration_error("input view depth sequence file does not exist");

	rs_camera_array& cam_arr = camera_array_();	
		
	camera_type cam(cam_arr[camera_name], scaled_shape());
	cam.flip_pixel_coordinates();
	
	return {
		name,
		cam,
		image_sequence_file,
		depth_sequence_file,
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
	std::ptrdiff_t first = 0;
	std::ptrdiff_t last = configuration_.input_views_count() - 1 - first;
	
	camera_type left_cam = configuration_.input_view_at(first).camera;
	
	pose left = left_cam.absolute_pose();
	pose right = configuration_.input_view_at(last).camera.absolute_pose();
	
	real n = 100;
	real t = -std::cos(frame_index * pi / n) / 2.0 + 0.5;
	real lin_t = (frame_index / n);
	pose virtual_pose = interpolate(left, right, t);
	virtual_pose.orientation = left.orientation.slerp(lin_t, right.orientation);

	camera_type virtual_cam = left_cam;
	virtual_cam.set_relative_pose(virtual_pose);
	return virtual_cam;
}


}
