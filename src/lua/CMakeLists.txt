
if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -ansi -pedantic")
	set(CMAKE_C_FLAGS_DEBUG   "-ggdb -Wall -Wextra -ansi -pedantic")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -ansi -pedantic")
endif()

add_subdirectory(src)

