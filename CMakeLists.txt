cmake_minimum_required(VERSION 2.8.4)
project(navd)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")

option(ENABLE_SOURCE_GPSSIMULATOR
	"Enable source GPS Simulator" ON)

option(ENABLE_SOURCE_GPSD
	"Enable source GPSD" OFF)

option(ENABLE_FILTER_LUA
	"Enable filter LUA" ON)

find_program(GCOV_PATH gcov)

add_subdirectory(src)

message("!  BUILD_TYPE                 : ${CMAKE_BUILD_TYPE}")
message("!  ENABLE_SOURCE_GPSSIMULATOR : ${ENABLE_SOURCE_GPSSIMULATOR}")
message("!  ENABLE_SOURCE_GPSD         : ${ENABLE_SOURCE_GPSD}")
message("!  ENABLE_FILTER_LUA          : ${ENABLE_FILTER_LUA}")

