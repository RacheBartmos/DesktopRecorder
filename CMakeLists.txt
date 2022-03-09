cmake_minimum_required(VERSION 3.14)

project(DesktopRecorder LANGUAGES CXX VERSION 0.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QtMoudles Core Gui)
set(FFMpegDepends avcodec avdevice avfilter avformat avutil swscale)

find_package(QT NAMES Qt6 Qt5 COMPONENTS ${QtMoudles} REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QtMoudles} REQUIRED)
include_directories(./DesktopRecorder/)
include_directories(./deps/include)
include_directories(./DesktopRecorder/encode)
include_directories(./DesktopRecorder/source)
link_directories(./deps/lib)
FILE(GLOB allHeads
    "./DesktopRecorder/*.h"
    "./DesktopRecorder/encode/*.h"
    "./DesktopRecorder/decode/*.h"
    "./DesktopRecorder/source/*.h"
)

FILE(GLOB allSources
    "./DesktopRecorder/*.cpp"
    "./DesktopRecorder/encode/*.cpp"
    "./DesktopRecorder/decode/*.cpp"
    "./DesktopRecorder/source/*.cpp"
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${allHeads} ${allSources} "./DesktopRecorder/main.cpp" )
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${allHeads} ${allSources} "./DesktopRecorder/main.cpp"
        )
    else()
        add_executable(${PROJECT_NAME} ${allHeads} ${allSources} "./DesktopRecorder/main.cpp"
        )
    endif()
endif()

#add_executable(${PROJECT_NAME} ${allHeads} ${allSources} "./DesktopRecorder/main.cpp")
foreach(QtMoudle ${QtMoudles})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::${QtMoudle})
endforeach()
target_link_libraries(${PROJECT_NAME} PRIVATE d3d9.lib d3d11.lib dxgi.lib)

foreach(ffmpegmodule ${FFMpegDepends})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${ffmpegmodule})
endforeach()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/deps/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
