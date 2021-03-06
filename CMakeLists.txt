cmake_minimum_required(VERSION 3.21)

set(OSX_ARCHITECTURE arm64 #[[x86_64]])
set(CMAKE_OSX_ARCHITECTURES ${OSX_ARCHITECTURE})
set(CMAKE_HOST_SYSTEM_PROCESSOR  ${OSX_ARCHITECTURE})
set(CMAKE_SYSTEM_PROCESSOR  ${OSX_ARCHITECTURE})

message("CMAKE_OSX_ARCHITECTURES: ${CMAKE_OSX_ARCHITECTURES}")
message("CMAKE_HOST_SYSTEM_PROCESSOR: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message("CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("CMAKE_CROSSCOMPILING: ${CMAKE_CROSSCOMPILING}")

project(ome_container)

set(CMAKE_CXX_STANDARD 23)

add_executable(ome_container main.cpp)

find_package(rxcpp CONFIG)

find_package(fmt)


set(BOOST_ROOT "/usr/local/include/boost")
set(BOOSTROOT "/usr/local/include/boost")
find_package(Boost 1.78.0)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

include_directories(${Boost_INCLUDE_DIRS} "/usr/local/include" "/usr/local/include/zmqpp/4.2.0/include" "/usr/local/include/zeromq/4.3.4/include" "/usr/local/include/cppzmq/4.8.1/include" "~/Documents/sc/c++/matchingengine" "~/Documents/sc/c++/matching_engine_test_utils")


SET(CMAKE_BUILD_TYPE_LOWER_CASE, ${CMAKE_BUILD_TYPE})
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER_CASE)

find_library(ZEROMQ_LIB
        NAMES libzmq.a
        PATHS /opt/homebrew/lib)

find_library(SODIUM_LIB
        NAMES libsodium.a
        PATHS /opt/homebrew/lib)

find_library(ZMQPP_LIB
        NAMES libzmqpp.a
        PATHS /opt/homebrew/lib)

find_library(ORDER_MATCHING_ENGINE_LIB
            NAMES libmatching_engine.a
            PATHS ~/lib/matching_engine/${CMAKE_BUILD_TYPE_LOWER_CASE})

find_library(BOOST_PROGRAM_OPTIONS_LIB
            NAMES libboost_program_options.a
            PATHS ~/Documents/sc/c++/boost_1_78_0/bin.v2/libs/program_options/build/clang-darwin-13/release/link-static/threading-multi/visibility-hidden)

if (${CMAKE_BUILD_TYPE_LOWER_CASE} STREQUAL "debug")
    message(STATUS "debug build")
    target_link_libraries(ome_container PRIVATE fmt::fmt debug ${SODIUM_LIB}  ${ZEROMQ_LIB} ${ZMQPP_LIB} ${ORDER_MATCHING_ENGINE_LIB} ${BOOST_PROGRAM_OPTIONS_LIB})
elseif(${CMAKE_BUILD_TYPE_LOWER_CASE} STREQUAL "release")
    message(STATUS "release build")
    target_link_libraries(ome_container PRIVATE fmt::fmt optimized ${SODIUM_LIB} ${ZEROMQ_LIB} ${ZMQPP_LIB} ${ORDER_MATCHING_ENGINE_LIB} ${BOOST_PROGRAM_OPTIONS_LIB})
endif()
