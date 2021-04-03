cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Conway VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add custom find cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# add SDL library
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

# add the executable
add_executable(Conway conway.cxx)
target_link_libraries(Conway ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})