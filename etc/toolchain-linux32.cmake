SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

ADD_DEFINITIONS(-D__LINUX__=1 -D__LINUX=1)

# Set Compiler options for Debug Builds
set(CMAKE_CXX_FLAGS_DEBUG   "-g -Wall -Wno-long-long -pedantic -g3 -ggdb -Wconversion -Wextra -Wfloat-equal -Woverloaded-virtual")
set(CMAKE_C_FLAGS_DEBUG   "-g -Wall -Wno-long-long -pedantic -g3 -ggdb -Wconversion -Wextra -Wfloat-equal -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS_PROFILE "-g -Wall -Wno-long-long -pedantic -g3 -ggdb -Wconversion -Wextra -Wfloat-equal -Woverloaded-virtual -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS_PROFILE "-g -Wall -Wno-long-long -pedantic -g3 -ggdb -Wconversion -Wextra -Wfloat-equal -Woverloaded-virtual -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -g")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG -g")
set(CMAKE_SHARED_LINKER_FLAGS "--hash-style=gnu -z now")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

