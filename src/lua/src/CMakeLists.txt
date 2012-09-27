
include_directories(
	"../include/lua"
	)

file(GLOB SOURCE_FILES *.c)

add_library(lua
	${SOURCE_FILES}
	)

