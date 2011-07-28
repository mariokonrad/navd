cmake_minimum_required(VERSION 2.8.3)
project(navd)

if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -pedantic -std=c99")
	set(CMAKE_C_FLAGS_DEBUG   "-ggdb -Wall -Wextra -pedantic -std=c99")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -pedantic -std=c99")
endif()

add_subdirectory(src)

