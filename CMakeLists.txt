cmake_minimum_required(VERSION 3.20)
project(no_one_cares)

# Checking for MSVC toolchain with C++20 features

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT MSVC)
    message(FATAL_ERROR "MSVC toolchain is required!")
endif ()

# Loading useful cmake functions

include(cmake/embedded.cmake)
include(cmake/utility.cmake)

# Loading all kinds of projects

add_subdirectory(projects)
