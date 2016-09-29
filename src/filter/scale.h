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

#ifndef VW_FILTER_SCALE_H_
#define VW_FILTER_SCALE_H_

#include <mf/filter/filter_handler.h>
#include <mf/filter/filter.h>
#include <mf/filter/filter_job.h>
#include <mf/filter/filter_parameter.h>
#include <mf/image/image_view.h>
#include <mf/opencv.h>
#include <mf/nd/ndcoord.h>
#include "../common.h"

namespace vs {

template<typename Elem>
class scale_filter : public mf::flow::filter_handler {
public:
	input_type<2, Elem> input;
	output_type<2, Elem> output;
	
	mf::ndsize<2> output_size;
	int interpolation = cv::INTER_AREA;
	
	explicit scale_filter(mf::flow::filter& filt) :
		mf::flow::filter_handler(filt),
		input(filt, "in"),
		output(filt, "out") { }
	
	void setup() override {
		output.define_frame_shape(output_size);
	}
	
	void process(job_type& job) override {					
		mf::image_view<Elem> in_img(job.in(input));
		
		cv::Mat_<Elem> scaled_img;
		cv::resize(
			in_img.cv_mat(),
			scaled_img,
			cv::Size(output_size[1], output_size[0]),
			0, 0,
			interpolation
		);
		
		job.out(output).assign( mf::to_ndarray_view(scaled_img) );
	}
};
	
}

#endif
