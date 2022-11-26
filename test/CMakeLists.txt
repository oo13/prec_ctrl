# Build settings for Unit Test of prec_ctrl.
# Copyright © 2022 OOTA, Masato
# This is published under CC0 1.0.
# For more information, see CC0 1.0 Universal (CC0 1.0) at <https://creativecommons.org/publicdomain/zero/1.0/legalcode>.
cmake_minimum_required(VERSION 3.5)

project(test_prec_ctrl CXX)

find_package(Catch2 REQUIRED)
add_executable(test_prec_ctrl main.cpp double_limit.cpp equivalence.cpp FixedPoint.cpp fixed_numeric.cpp)
target_link_libraries(test_prec_ctrl Catch2::Catch2)
target_include_directories(test_prec_ctrl PRIVATE ../include)
target_compile_features(test_prec_ctrl PRIVATE cxx_std_17)
target_compile_definitions(test_prec_ctrl PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
set(CMAKE_CXX_FLAGS "-Wall -pedantic -O3")

include(CTest)
include(Catch)
catch_discover_tests(test_prec_ctrl)
