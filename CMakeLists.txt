cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include("target_add_sanitizers")
include("target_all_warnings_except")
include("target_use_modern_cxx")
include("default_build_type")

default_build_type(Release)
project(piecewise VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF) # Turn off gnu extensions

add_library(piecewise INTERFACE)
target_include_directories(piecewise
  INTERFACE
  "include")
# TODO - whitelist features here if necessary
target_use_modern_cxx(piecewise TYPE INTERFACE)

add_executable(tests
  "test/main.cpp"
  "test/make_from_tuple.cpp")

target_include_directories(tests
  PRIVATE
  "${CMAKE_SOURCE_DIR}/bundle/catch/include")

target_all_warnings_except(tests
  CLANG
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-exit-time-destructors # catch
  -Wno-weak-vtables # AFAIK there's not much to do about this one while remaining header-only
  -Wno-padded
  GCC
  -Wno-unknown-pragmas # catch
  )

target_add_sanitizers(tests)

target_link_libraries(tests PRIVATE piecewise)

enable_testing()
add_test(tests tests)
