
find_package(BISON)
find_package(FLEX)

BISON_TARGET(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c)
FLEX_TARGET(lexer lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.c)

add_library(config
	config.c
	${BISON_parser_OUTPUTS}
	${FLEX_lexer_OUTPUTS}
	)

