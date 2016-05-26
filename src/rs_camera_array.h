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
