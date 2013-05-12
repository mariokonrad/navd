
if (CMAKE_COMPILER_IS_GNUCC)
	if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -ansi -pedantic -m64")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ansi -pedantic -ggdb -m64")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -ansi -pedantic -DNDEBUG -m64")
	elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -ansi -pedantic -m32")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ansi -pedantic -ggdb -m32")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -ansi -pedantic -DNDEBUG -m32")
	else()
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -ansi -pedantic")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ansi -pedantic -ggdb")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -ansi -pedantic -DNDEBUG")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS         "")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG   "-fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
endif()

add_subdirectory(src)

