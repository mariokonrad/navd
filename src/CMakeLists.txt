
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(parser config-parser.y ${CMAKE_CURRENT_BINARY_DIR}/config-parser.tab.c)
FLEX_TARGET(lexer config-lexer.l ${CMAKE_CURRENT_BINARY_DIR}/config-lexer.yy.c)

add_subdirectory(common)
add_subdirectory(device)
add_subdirectory(nmea)
add_subdirectory(test)

add_executable(navd
	navd.c
	${BISON_parser_OUTPUTS}
	${FLEX_lexer_OUTPUTS}
	)

target_link_libraries(navd
	devices
	nmea
	common
	)

