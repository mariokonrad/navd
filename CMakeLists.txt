cmake_minimum_required(VERSION 2.8.9)
project(navd)

################################################################################
# version
################################################################################

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "1")

################################################################################
# options
################################################################################

option(ENABLE_SOURCE_LUA
	"Enable source LUA" ON)

option(ENABLE_SOURCE_GPSSERIAL
	"Enable source GPS serial" ON)

option(ENABLE_SOURCE_GPSSIMULATOR
	"Enable source GPS Simulator" ON)

option(ENABLE_SOURCE_SEATALKSIMULATOR
	"Enable source SeaTalk Simulator" ON)

option(ENABLE_SOURCE_SEATALKSERIAL
	"Enable source SeaTalk serial" ON)

option(ENABLE_SOURCE_GPSD
	"Enable source GPSD" OFF)

option(ENABLE_FILTER_LUA
	"Enable filter LUA" ON)

option(ENABLE_FILTER_NMEA
	"Enable filter NMEA" ON)

option(ENABLE_FILTER_SEATALK_TO_NMEA
	"Enable filter SeaTalk to NMEA" ON)

option(ENABLE_DESTINATION_LUA
	"Enable destination LUA" ON)

option(ENABLE_DESTINATION_LOGBOOK
	"Enable destination Logbook" ON)

option(ENABLE_DESTINATION_NMEASERIAL
	"Enable destination NMEA serial" ON)

if (false
		OR ENABLE_SOURCE_LUA
		OR ENABLE_FILTER_LUA
		OR ENABLE_DESTINATION_LUA
		)
	set(NEEDS_LUA true)
endif()

if (false
		OR ENABLE_SOURCE_SEATALKSERIAL
		OR ENABLE_SOURCE_SEATALKSIMULATOR
		OR ENABLE_FILTER_SEATALK_TO_NMEA
		)
	set(NEEDS_SEATALK true)
endif()

if (false
		OR ENABLE_SOURCE_GPSSERIAL
		OR ENABLE_SOURCE_GPSSIMULATOR
		OR ENABLE_SOURCE_GPSD
		OR ENABLE_FILTER_NMEA
		OR ENABLE_FILTER_SEATALK_TO_NMEA
		OR ENABLE_DESTINATION_LOGBOOK
		OR ENABLE_DESTINATION_NMEASERIAL
		)
	set(NEEDS_NMEA true)
endif()

################################################################################
# build
################################################################################

find_program(GCOV_PATH gcov)

if (CMAKE_COMPILER_IS_GNUCC)
	if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -m64")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ggdb -fprofile-arcs -ftest-coverage -m64")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG -m64")
	elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -m32")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ggdb -fprofile-arcs -ftest-coverage -m32")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG -m32")
	else()
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ggdb -fprofile-arcs -ftest-coverage")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS         "")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG   "-fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
endif()

add_subdirectory(src)

message("!  SYSTEM                         : ${CMAKE_SYSTEM_NAME}")
message("!  PROCESSOR                      : ${CMAKE_SYSTEM_PROCESSOR}")
message("!  BUILD_TYPE                     : ${CMAKE_BUILD_TYPE}")
message("!  ENABLE_SOURCE_LUA              : ${ENABLE_SOURCE_LUA}")
message("!  ENABLE_SOURCE_GPSSERIAL        : ${ENABLE_SOURCE_GPSSERIAL}")
message("!  ENABLE_SOURCE_GPSSIMULATOR     : ${ENABLE_SOURCE_GPSSIMULATOR}")
message("!  ENABLE_SOURCE_SEATALKSIMULATOR : ${ENABLE_SOURCE_SEATALKSIMULATOR}")
message("!  ENABLE_SOURCE_SEATALKSERIAL    : ${ENABLE_SOURCE_SEATALKSERIAL}")
message("!  ENABLE_SOURCE_GPSD             : ${ENABLE_SOURCE_GPSD}")
message("!  ENABLE_FILTER_LUA              : ${ENABLE_FILTER_LUA}")
message("!  ENABLE_FILTER_NMEA             : ${ENABLE_FILTER_NMEA}")
message("!  ENABLE_FILTER_SEATALK_TO_NMEA  : ${ENABLE_FILTER_SEATALK_TO_NMEA}")
message("!  ENABLE_DESTINATION_LUA         : ${ENABLE_DESTINATION_LUA}")
message("!  ENABLE_DESTINATION_LOGBOOK     : ${ENABLE_DESTINATION_LOGBOOK}")
message("!  ENABLE_DESTINATION_NMEASERIAL  : ${ENABLE_DESTINATION_NMEASERIAL}")

message("!  NEEDS_LUA                      : ${NEEDS_LUA}")
message("!  NEEDS_SEATALK                  : ${NEEDS_SEATALK}")
message("!  NEEDS_NMEA                     : ${NEEDS_NMEA}")

################################################################################
# installation
################################################################################

set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(CPACK_PACKAGE_NAME "navd")
set(CPACK_PACKAGE_VENDOR "Mario Konrad")
set(CPACK_PACKAGE_CONTACT "Mario Konrad <mario.konrad@gmx.net>")
set(CPACK_PACAKGE_DESCRIPTION "navd is a logging/multiplexing software, intended to run on low-power linux boxes.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Logger/Multiplexer")
set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_PACKAGE_EXECUTABLES "navd" "navd")
set(CPACK_STRIP_FILES true)

# debian

set(CPACK_DEBIAN_PACKAGE_NAME ${CPACK_DEBIAN_PACKAGE_NAME})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_PACAKGE_DESCRIPTION})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3)")
set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
if (CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
else()
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
endif()


include(CPack)

