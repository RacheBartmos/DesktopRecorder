cmake_minimum_required(VERSION 3.14)

project(DesktopRecorder LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(FFMpegDepends avcodec avdevice avfilter avformat avutil swscale)
include_directories(./deps/include)
link_directories(./deps/lib)
FILE(GLOB allHeads  
	"./DesktopRecorder/*.h"
)

FILE(GLOB allSources  
	"./DesktopRecorder/*.cpp"
)

add_executable(${PROJECT_NAME} ${allHeads} ${allSources} "./DesktopRecorder/main.cpp")
foreach(ffmpegmodule ${FFMpegDepends})
	target_link_libraries(${PROJECT_NAME} ${ffmpegmodule})
endforeach()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/deps/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
