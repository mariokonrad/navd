cmake_minimum_required(VERSION 2.8.4)
project(navd)

find_program(GCOV_PATH gcov)

if (CMAKE_COMPILER_IS_GNUCC)
# pedantic and c99 disabled because of sigaction/etc and pselect
#	set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -pedantic -std=c99")
#	set(CMAKE_C_FLAGS_DEBUG   "-ggdb -Wall -Wextra -pedantic -std=c99")
#	set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -pedantic -std=c99")
	set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -fprofile-arcs -ftest-coverage")
	set(CMAKE_C_FLAGS_DEBUG   "-O0 -ggdb -Wall -Wextra -fprofile-arcs -ftest-coverage")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra")

	set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
endif()

add_subdirectory(src)

