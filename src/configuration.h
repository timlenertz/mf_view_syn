#ifndef VS_CONFIGURATION_H_
#define VS_CONFIGURATION_H_

#include <string>
#include <json.hpp>
#include <mf/geometry/depth_projection_parameters.h>
#include <mf/io/raw_video_frame_format.h>
#include <memory>
#include <string>
#include "common.h"

namespace vs {
	
class rs_camera_array;

class configuration {
public:
	struct input_view {
		std::string name;
		camera_type camera;
		std::string image_sequence_file;
		std::string depth_sequence_file;
	};
	
	class virtual_camera_functor {
	private:
		const configuration& configuration_;
	public:
		camera_type operator()(mf::time_unit) const;
		explicit virtual_camera_functor(const configuration& config) :
			configuration_(config) { }
	};
	
private:
	nlohmann::json json_;
	mutable std::unique_ptr<rs_camera_array> camera_arr_;

	rs_camera_array& camera_array_() const;
	mf::depth_projection_parameters depth_projection_() const;

public:
	explicit configuration(const std::string& filename);
	~configuration();
	
	mf::ndsize<2> input_shape() const;
	
	std::size_t input_views_count() const;
	input_view input_view_at(std::ptrdiff_t i) const;
	
	virtual_camera_functor virtual_camera() const {
		return virtual_camera_functor(*this);
	}
	
	bool output_rgb() const;
	mf::raw_video_frame_format<mf::rgb_color> output_rgb_raw_format() const;
	mf::raw_video_frame_format<mf::ycbcr_color> output_ycbcr_raw_format() const;
	
	auto operator[](const std::string& key) const {
		return json_[key];
	}
};


MF_DEFINE_EXCEPTION(configuration_error, std::runtime_error);
	
}

#endif
