cmake_minimum_required(VERSION 2.8.4)
project(navd)

if (CMAKE_COMPILER_IS_GNUCC)
# pedantic and c99 disabled because of sicaction/etc and pselect
#	set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -pedantic -std=c99")
#	set(CMAKE_C_FLAGS_DEBUG   "-ggdb -Wall -Wextra -pedantic -std=c99")
#	set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -pedantic -std=c99")
	set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG   "-ggdb -Wall -Wextra")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra")
endif()

add_subdirectory(src)

