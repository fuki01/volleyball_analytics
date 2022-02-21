cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 11)
project(j-base-clustring)

find_package(OpenCV 4.0.1 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
file(GLOB SOURCES src/*.cpp src/*.hpp src/*.h src/*/*.cpp src/*/*.hpp src/*/*.h)

add_executable (j-base-clustring ${SOURCES})
target_link_libraries (j-base-clustring
	PRIVATE ${OpenCV_LIBS}
)