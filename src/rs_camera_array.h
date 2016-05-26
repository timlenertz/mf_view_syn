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

#ifndef VS_RS_CAMERA_ARRAY_H_
#define VS_RS_CAMERA_ARRAY_H_

#include <mf/common.h>
#include <mf/geometry/depth_projection_parameters.h>
#include <mf/camera/projection_camera.h>
#include <mf/ndarray/ndcoord.h>
#include <string>
#include <map>
#include <cstdint>

namespace vs {

/// Reader of camera array format used by DERS/VSRS.
/** File contains array of names projection cameras, defined using their extrinsic and intrinsic matrices, in plain
 ** text format. Some camera array files contain additional fourth `0 0 0 1` row of extrinsic matrix, while some omit
 ** it. */
class rs_camera_array {
private:
	std::map<std::string, mf::projection_camera> cameras_;
	
	mf::projection_camera read_camera_(std::istream& str, const mf::depth_projection_parameters&, const mf::ndsize<2>&);

public:
	rs_camera_array(const std::string& filename, const mf::depth_projection_parameters&, const mf::ndsize<2>&);
		
	bool has(const std::string&) const;
	const mf::projection_camera& operator[](const std::string&) const;
};

}

#endif
