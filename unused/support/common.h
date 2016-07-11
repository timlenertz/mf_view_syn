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

#ifndef PROG_COMMON_H_
#define PROG_COMMON_H_

#include <cstdint>
#include <mf/masked_elem.h>
#include <mf/color.h>

namespace mf {

template<> inline std::uint8_t color_convert(const mf::ycbcr_color& in) {
	return in.y;
}

template<> inline mf::masked_elem<std::uint8_t> color_convert(const mf::ycbcr_color& in) {
	if(in.y == 0.0) return mf::masked_elem<std::uint8_t>::null();
	else return in.y;
}

template<> inline mf::rgb_color color_convert(const std::uint8_t& in) {
	return mf::rgb_color(in, in, in);
}

template<> inline mf::rgb_color color_convert(const mf::masked_elem<std::uint8_t>& in) {
	if(in.is_null()) return mf::rgb_color(0, 0, 0); // background
	else return mf::rgb_color(in, in, in);
}

}

using depth_type = std::uint8_t;

#endif
