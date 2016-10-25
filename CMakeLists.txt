project(mf CXX)
cmake_minimum_required(VERSION 3.1)

if(UNIX)
	add_compile_options(-std=c++14)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")
	
	find_program(CCACHE_FOUND ccache)
	if(CCACHE_FOUND)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
	endif()
	
	include(FindPkgConfig)
	pkg_search_module(SDL2 REQUIRED sdl2)

elseif(WIN32)
	add_compile_options(/wd4018)

	include_directories(../sdl2/include)
	find_library(SDL2_LIBRARIES SDL2 PATHS ../sdl2/lib/x64/)
endif()

include_directories(SYSTEM ../mf/external/include)

find_package(OpenCV REQUIRED)

# link to mf
include_directories(SYSTEM ../mf/dist/include)
find_library(MF_LIB mf PATHS ../mf/dist/lib NO_DEFAULT_PATH)


# executable
file(GLOB_RECURSE EXE_SRC "src/*.cc")
add_executable(view_syn ${EXE_SRC})
target_link_libraries(view_syn ${MF_LIB} ${OpenCV_LIBS} ${SDL2_LIBRARIES})

install(TARGETS view_syn DESTINATION .)
