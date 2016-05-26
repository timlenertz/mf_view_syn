#ifndef VS_COMMON_H_
#define VS_COMMON_H_

#include <mf/common.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/masked_elem.h>
#include <mf/color.h>
#include <cstdint>

namespace vs {

using integral_depth_type = std::uint8_t;
using real_depth_type = mf::real;
using masked_real_depth_type = mf::masked_elem<real_depth_type>;
using color_type = mf::rgb_color;
using masked_color_type = mf::masked_elem<color_type>;

using camera_type = mf::projection_image_camera<integral_depth_type>;

constexpr static mf::masked_elem_flag_index unstable_pixel_flag = 1;


}

namespace mf {

template<> inline std::uint8_t color_convert(const mf::ycbcr_color& in) {
	return in.y;
}

}


#endif
