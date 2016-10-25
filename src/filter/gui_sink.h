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

#ifndef VS_GUI_SINK_H_
#define VS_GUI_SINK_H_

#include <mf/filter/filter_handler.h>
#include <mf/filter/filter.h>
#include <mf/filter/filter_job.h>
#include <mf/filter/filter_parameter.h>
#include "../common.h"
#include <SDL2/SDL.h>

namespace vs {

class gui_sink : public mf::flow::filter_handler {
public:
	input_type<2, mf::rgb_color> input;
	//parameter_type<camera_type> camera;
	
private:
	SDL_Window* sdl_window_ = nullptr;
	SDL_Surface* sdl_screen_surface_ = nullptr;

public:
	explicit gui_sink(mf::flow::filter& filt) :
		mf::flow::filter_handler(filt),
		input(filt, "in")
		//camera(filt, "cam")
	{
		this_filter().set_own_timing(mf::flow::stream_timing::real_time());
		this_filter().set_name("gui sink");
	}
	
	void setup() override;
	void process(job_type& job) override;
};
	
}

#endif
