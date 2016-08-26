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

#ifndef VS_COMMON_H_
#define VS_COMMON_H_

#include <mf/common.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/color.h>
#include <cstdint>

namespace vs {
	
using integral_depth_type = std::uint8_t;
using real_depth_type = mf::real;
using color_type = mf::rgb_color;

using camera_type = mf::projection_image_camera<integral_depth_type>;

using mask_type = std::uint8_t;
using tri_mask_type	= std::uint8_t;

constexpr mask_type mask_set = 0xff;
constexpr mask_type mask_clear = 0x00;

constexpr tri_mask_type tri_mask_stable = 0xff;
constexpr tri_mask_type tri_mask_unstable = 0xfe;
constexpr tri_mask_type tri_mask_clear = 0x00;

}


#endif
