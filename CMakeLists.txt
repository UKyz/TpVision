cmake_minimum_required(VERSION 3.13)
project(TP3)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_CXX_STANDARD 14)

find_package( OpenCV REQUIRED )

FILE(COPY images/ DESTINATION ./images)

set(SOURCE_FILES main.cpp)
add_executable(OpenCVTest ${SOURCE_FILES})
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries( OpenCVTest ${OpenCV_LIBS} )