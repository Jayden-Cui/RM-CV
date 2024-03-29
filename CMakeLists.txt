# cmake needs this line
cmake_minimum_required(VERSION 3.1)

# Define project name
project(rm_cv)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(GLOB_RECURSE src main.cpp armor/*/*.cpp energy/*/*.cpp utils/*.cpp)

include_directories(armor/include)
include_directories(energy/include)
include_directories(utils)

# Declare the executable target built from your sources
add_executable(armor ${src})

# Link your application with OpenCV libraries
target_link_libraries(armor PRIVATE ${OpenCV_LIBS})

