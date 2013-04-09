SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER   gcc)
SET(CMAKE_CXX_COMPILER g++)

ADD_DEFINITIONS(-D__LINUX__=1 -D__LINUX=1)

if (CMAKE_COMPILER_IS_GNUCC)
	# pedantic and c99 disabled because of sigaction/etc and pselect
	set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG -m64")
	set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -m64")
	set(CMAKE_C_FLAGS_DEBUG   "-O0 -ggdb -Wall -Wextra -fprofile-arcs -ftest-coverage -m64")

	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
	set(CMAKE_EXE_LINKER_FLAGS         "")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG   "-fprofile-arcs -ftest-coverage")
endif()

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
