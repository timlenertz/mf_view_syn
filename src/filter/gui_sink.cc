#include "gui_sink.h"

namespace vs {

using namespace mf;

void gui_sink::setup() {	
	ndsize<2> shp = input.frame_shape();
	sdl_window_ = SDL_CreateWindow(
		"ViewSyn",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		shp[1], shp[0],
		SDL_WINDOW_SHOWN
	);
	Assert(sdl_window_ != nullptr);
	
	sdl_screen_surface_ = SDL_GetWindowSurface(sdl_window_);
	Assert(sdl_screen_surface_ != nullptr);
}


void gui_sink::process(job_type& job) {
//	auto cam = job.param(camera);
	ndarray_view<2, rgb_color> image_in = job.in(input);

	ndsize<2> shp = input.frame_shape();
	SDL_Surface* frame_surface = SDL_CreateRGBSurfaceFrom(
		static_cast<void*>(image_in.start()),
		shp[1],
		shp[0],
		8 * 3,
		shp[1] * sizeof(rgb_color),
		0xff0000,
		0x00ff00,
		0x0000ff,
		0
	);
	Assert(frame_surface);
	
	int err;
	
	err = SDL_BlitSurface(frame_surface, nullptr, sdl_screen_surface_, nullptr);
	Assert(err == 0);
	
//	SDL_FreeSurface(frame_surface);
	
	err = SDL_UpdateWindowSurface(sdl_window_);
	Assert(err == 0);
	
	//SDL_Delay(100);

	if(job.time() == 700) job.mark_end();
}

}
