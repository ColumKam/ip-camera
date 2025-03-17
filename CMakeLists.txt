cmake_minimum_required(VERSION 3.10)
project(ip-camera C)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 查找必要的包
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)
pkg_check_modules(GSTREAMER_RTSP REQUIRED gstreamer-rtsp-server-1.0)
pkg_check_modules(QRENCODE REQUIRED libqrencode)
pkg_check_modules(CAIRO REQUIRED cairo)

# 添加源文件
set(SOURCES
    src/main.c
    src/gst-streamer-common.c
)

# 添加头文件
set(HEADERS
    src/gst-streamer-common.h
)

# 创建可执行文件
add_executable(streamer ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(streamer PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${GSTREAMER_VIDEO_INCLUDE_DIRS}
    ${GSTREAMER_RTSP_INCLUDE_DIRS}
    ${QRENCODE_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

# 链接库
target_link_libraries(streamer
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${GSTREAMER_VIDEO_LIBRARIES}
    ${GSTREAMER_RTSP_LIBRARIES}
    ${QRENCODE_LIBRARIES}
    ${CAIRO_LIBRARIES}
)

# 设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(streamer PRIVATE -g -Wall -Wextra)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(streamer PRIVATE -O2 -Wall -Wextra)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(streamer PRIVATE -O2 -Wall -Wextra -g)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(streamer PRIVATE -O2 -Wall -Wextra)
endif()

# 安装目标
install(TARGETS streamer
    RUNTIME DESTINATION bin
) 