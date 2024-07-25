cmake_minimum_required(VERSION 3.10)

# Project name and version
project(GStreamerRTSP VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_THIRDPARTY ${PROJECT_SOURCE_DIR}/thirdparty)

#Add GStreamer
set(GSTREAMER_DIR ${PROJECT_THIRDPARTY}/GStreamer)

set(GSTREAMER_INCLUDE_DIR ${GSTREAMER_DIR}/1.0/msvc_x86_64)
set(GSTREAMER_LINK_DIR ${GSTREAMER_DIR}/1.0/msvc_x86_64/lib)
include_directories(${GSTREAMER_INCLUDE_DIR}/include
                    ${GSTREAMER_INCLUDE_DIR}/include/libxml2
                    ${GSTREAMER_INCLUDE_DIR}/include/glib-2.0
                    ${GSTREAMER_INCLUDE_DIR}/lib/glib-2.0/include
                    ${GSTREAMER_INCLUDE_DIR}/include/gstreamer-1.0)
link_directories(${GSTREAMER_LINK_DIR})


# Find the GStreamer package
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

link_directories(
   ${GSTREAMER_LINK_DIR}
)

# Add the executable
add_executable(gstreamer_rtsp gstreamer_rtsp.cpp)

# Include GStreamer headers
include_directories(${GSTREAMER_INCLUDE_DIRS})

# Link GStreamer libraries
target_link_libraries(gstreamer_rtsp 
    gobject-2.0.lib 
    glib-2.0.lib 
    gstreamer-1.0.lib 
    gstapp-1.0.lib
    ${GSTREAMER_LIBRARIES}
)
