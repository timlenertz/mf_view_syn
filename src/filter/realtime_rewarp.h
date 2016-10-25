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

#ifndef VW_REALTIME_REWARP_H_
#define VW_REALTIME_REWARP_H_

#include <mf/filter/filter_handler.h>
#include <mf/filter/filter.h>
#include <mf/filter/filter_job.h>
#include <mf/filter/filter_parameter.h>
#include <mf/image/image_view.h>
#include <mf/opencv.h>
#include <mf/nd/ndcoord.h>
#include "../common.h"

namespace vs {

class realtime_rewarp : public mf::flow::filter_handler {
public:
	input_type<2, mf::rgb_color> image_input;
	input_type<2, real_depth_type> depth_input;
	output_type<2, mf::rgb_color> image_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;
		
	explicit realtime_rewarp(mf::flow::filter& filt) :
		mf::flow::filter_handler(filt),
		image_input(filt, "im in"),
		depth_input(filt, "di in"),
		image_output(filt, "out"),
		source_camera(filt, "in pose"),
		destination_camera(filt, "out pose") { }
	
	void setup() override;
	void process(job_type& job) override;
};
	
}

#endif
