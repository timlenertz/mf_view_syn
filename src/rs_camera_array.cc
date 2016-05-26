#include "rs_camera_array.h"
#include <fstream>
#include <string>
#include <mf/eigen.h>
#include <mf/geometry/pose.h>

#include <iostream>

namespace vs {

using namespace mf;

projection_camera rs_camera_array::read_camera_
(std::istream& str, const depth_projection_parameters& dparam, const ndsize<2>& img_sz) {
	str.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	
	Eigen_mat3 intrinsic = Eigen_mat3::Zero();
	str >> intrinsic(0, 0) >> intrinsic(0, 1) >> intrinsic(0, 2);
	str >> intrinsic(1, 0) >> intrinsic(1, 1) >> intrinsic(1, 2);
	str >> intrinsic(2, 0) >> intrinsic(2, 1) >> intrinsic(2, 2);
	
	Eigen_scalar gomi[2]; // ?
	str >> gomi[0] >> gomi[1];

	Eigen_mat3 rotation;
	Eigen_vec3 translation;
	str >> rotation(0, 0) >> rotation(0, 1) >> rotation(0, 2); str >> translation(0);
	str >> rotation(1, 0) >> rotation(1, 1) >> rotation(1, 2); str >> translation(1);
	str >> rotation(2, 0) >> rotation(2, 1) >> rotation(2, 2); str >> translation(2);
	
	Eigen_affine3 extrinsic_affine;
	extrinsic_affine = Eigen_translation3(translation) * Eigen_affine3(rotation).inverse();
		
	return projection_camera(extrinsic_affine, intrinsic, dparam, img_sz);
}


rs_camera_array::rs_camera_array
(const std::string& filename, const depth_projection_parameters& dparam, const ndsize<2>& img_sz) {
	std::ifstream file(filename);
	
	for(;;) {
		file.exceptions(std::ios_base::badbit);	
		
		std::string name;
		file >> name;
		if(file.eof() || file.fail()) break;
				
		if(name == "0") {
			int unused;
			file >> unused >> unused >> unused;
			continue;
		}
		
		cameras_.insert(std::make_pair(
			name,
			read_camera_(file, dparam, img_sz)
		));
	}
}


bool rs_camera_array::has(const std::string& name) const {
	return (cameras_.find(name) != cameras_.end());
}


const projection_camera& rs_camera_array::operator[](const std::string& name) const {
	return cameras_.at(name);
}


}

