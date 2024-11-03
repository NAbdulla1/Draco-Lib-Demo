cmake_minimum_required(VERSION 3.10)
project(DracoDemo)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify paths to Draco headers and library
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Add the main executable
add_executable(DracoDemo src/main.cpp)

# Link Draco library
target_link_libraries(DracoDemo draco)
