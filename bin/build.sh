#!/bin/bash

export SCRIPT_BASE=$(dirname `readlink -f $0`)
export BASE=${SCRIPT_BASE}/..
export TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=${BASE}/etc/toolchain-${PLATFORM}.cmake"
export PLATFORM=${PLATFORM:-auto}
export BUILD_TYPE=${BUILD_TYPE:-Debug}
export PACKAGE=${PACKAGE:-TGZ}

if [ "${PLATFORM}" == "auto" ] ; then
	case `uname -m` in
		i686)   export TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=${BASE}/etc/toolchain-i686-linux.cmake" ;;
		x86_64) export TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=${BASE}/etc/toolchain-x86_64-linux.cmake" ;;
		*)      export TOOLCHAIN_FILE="" ;;
	esac
fi

function usage()
{
	echo ""
	echo "usage: $1 command"
	echo ""
	echo "Commands:"
	echo "    info           : displays info"
	echo "    clean          : cleans up the build"
	echo "    build          : builds the software, available build types: Debug, Release (default: Debug)"
	echo "    package        : packs the software."
	echo "    release        : executes clean/build/pack"
	echo "    index          : builds source index (tags, scope)"
	echo "    tags           : builds source tags"
	echo "    scope          : builds source scope"
	echo "    doc            : creates the documentation"
	echo "    unittest       : executes the unit tests"
	echo "    test           : executes the tests"
	echo "    cppcheck       : performs cppcheck"
	echo "    cccc           : calculates metrics"
	echo "    valgrind       : calls valgrind on tests to check for memory problems"
	echo "    check-coverage : checks if all files are being used for code coverage (debug build only)"
	echo "    pmccabe        : calculates the cyclomatic complexity of the relevant source (without Lua, tests)"
	echo "    gcovr          : calculates coverage using gcovr"
	echo "    lcov           : generates a HTML report on coverage using lcov"
	echo "    todo           : searches and displays all todos, fixmes and temps"
	echo ""
}

function exec_info()
{
	echo ""
	echo "INFO:"
	echo "    BUILD_TYPE     = ${BUILD_TYPE}"
	echo "    PACKAGE        = ${PACKAGE}"
	echo "    PLATFORM       = ${PLATFORM}"
	echo "    SCRIPT_BASE    = ${SCRIPT_BASE}"
	echo "    BASE           = ${BASE}"
	echo "    TOOLCHAIN_FILE = ${TOOLCHAIN_FILE}"
	echo ""
}

function exec_prepare()
{
	if [ ! -d "${BASE}/build" ] ; then
		mkdir -p ${BASE}/build
		mkdir -p ${BASE}/build/doc
	fi
}

function exec_clean()
{
	rm -fr ${BASE}/build
	rm -f ${BASE}/tags
	rm -f ${BASE}/cscope.files
	rm -f ${BASE}/cscope.out
	rm -f ${BASE}/cccc
}

function exec_filelist()
{
	cd ${BASE}
	echo "src/navd.c" > cscope.files
	for dirname in src/common src/config src/device src/navcom src/nmea ; do
		find ${dirname} -type f -name "*.c" -o -name "*.h" >> cscope.files
	done
}

function exec_tags()
{
	cd ${BASE}
	ctags --recurse -f tags src/*
}

function exec_scope()
{
	cd ${BASE}
	rm -f cscope.out
	cscope -b -i cscope.files
}

function exec_build()
{
	exec_prepare
	cd ${BASE}/build
	if [ ! -r Makefile ] ; then
		cmake \
			${TOOLCHAIN_FILE} \
			-DCMAKE_VERBOSE_MAKEFILE=FALSE \
			-DCMAKE_COLOR_MAKEFILE=TRUE \
			-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
			..
	fi
	#cmake --build .
	make
}

function exec_package()
{
	cd ${BASE}/build
	cpack -G ${PACKAGE}
}

function exec_doc()
{
	exec_prepare
	doxygen ${BASE}/etc/doxygen.conf
}

function exec_unittest_gcov()
{
	find build/src -name "*.gcno" | while read fn
	do
		(cd $(dirname $fn) ; gcov -o . $(echo $(basename $fn) | sed 's/\.gcno/\.o/'))
	done > build/gcov-summary.log 2>&1
	awk -f ${SCRIPT_BASE}/cov-summary.awk build/gcov-summary.log
}

function exec_unittest()
{
	if [ -r "${BASE}/build/src/test/testrunner" ] ; then
		${BASE}/build/src/test/testrunner
	else
		echo "error: unit tests not present"
	fi
}

function exec_gcovr()
{
	exec_prepare
	gcovr \
		-p \
		--object-directory=build/src \
		--gcov-exclude=".*lexer.yy.c|.*parser.tab.c|.*lexer.l|.*parser.y" \
		-o build/doc/gcovr.txt

	# file list with:
	# cat build/doc/gcovr.txt | grep -E "^/home" | sed 's/\(\w\)\ .*$/\1/' | sort
}

function exec_lcov()
{
	exec_prepare
	lcov --zerocounters --directory build/src --output-file build/doc/coverage.info
	${BASE}/build/src/navd --help
	${BASE}/build/src/navd --list
	${BASE}/build/src/navd --version
	${BASE}/build/src/navd foobar
	${BASE}/build/src/navd --config ${BASE}/src/test/testconfig --dump-config --log 7 --max-msg 5
	${BASE}/build/src/test/testrunner
	lcov --capture --directory build/src --output-file build/doc/coverage.info
	lcov --remove build/doc/coverage.info "/usr/*" --output-file build/doc/coverage.info
	lcov --remove build/doc/coverage.info "lexer.yy.*" --output-file build/doc/coverage.info
	lcov --remove build/doc/coverage.info "parser.tab.*" --output-file build/doc/coverage.info
	lcov --remove build/doc/coverage.info "lexer.l" --output-file build/doc/coverage.info
	lcov --remove build/doc/coverage.info "parser.y" --output-file build/doc/coverage.info
	genhtml build/doc/coverage.info --output-directory build/doc/coverage
}

function exec_test()
{
	case $1 in
		config)
			if [ -r "${BASE}/build/src/test/config_test" ] ; then
				${BASE}/build/src/test/config_test ${BASE}/src/test/testconfig-small
			else
				echo "error: config test not present"
			fi
			;;

		*)
			echo "no test specified"
			;;
	esac
}

function exec_valgrind()
{
	if [ -r "${BASE}/build/src/test/config_test" ] ; then
		valgrind --leak-check=full ${BASE}/build/src/test/config_test ${BASE}/src/test/testconfig-small
	else
		echo "error: config test not present"
	fi

	echo ""
	echo "----------------------------------------"
	echo ""

	if [ -r "${BASE}/build/src/test/config_test" ] ; then
		valgrind --leak-check=full ${BASE}/build/src/test/config_test ${BASE}/src/test/testconfig-small-1
	else
		echo "error: config test not present"
	fi

	echo ""
	echo "----------------------------------------"
	echo ""

	if [ -r "${BASE}/build/src/test/config_test_1" ] ; then
		valgrind --leak-check=full --show-reachable=yes ${BASE}/build/src/test/config_test_1
	else
		echo "error: config test 1 not present"
	fi

	echo ""
	echo "----------------------------------------"
	echo ""

	if [ -r "${BASE}/build/src/test/config_test_1" ] ; then
		valgrind -v --leak-check=full --show-reachable=yes ${BASE}/build/src/test/testrunner
	else
		echo "error: testrunner not present"
	fi
}

function exec_cppcheck()
{
	# unfortunately cppcheck cannot handle paths like '${BASE}/src' therefore
	# cppchecks must be executed from directory ${BASE}

	cd ${BASE}
	cppcheck -v -f --enable=all -i src/test/cunit -i src/lua/src -I src/lua/include/lua -I src src
}

function exec_cccc()
{
	if [ ! -r cscope.files ] ; then
		exec_filelist
	fi

	exec_prepare
	cat cscope.files | xargs cccc --lang="c" --outdir=${BASE}/build/doc/cccc
}

function exec_pmccabe()
{
	cd ${BASE}
	pmccabe $(cat cscope.files)
}

function exec_todo()
{
	grep --color -Erni "\<todo\>|\<fixme\>|\<temp\>" --exclude-dir=${BASE}/src/lua ${BASE}/src/*
}

if [ $# == 0 ] ; then
	usage $0
	exit
fi

case $1 in
	info)
		exec_info
		;;
	clean)
		exec_clean
		;;
	build)
		exec_build
		;;
	package)
		exec_package
		;;
	release)
		export BUILD_TYPE=Release
		exec_clean
		exec_info
		exec_build
		exec_package
		;;
	index)
		exec_filelist
		exec_tags
		exec_scope
		;;
	tags)
		exec_filelist
		exec_tags
		;;
	scope)
		exec_filelist
		exec_scope
		;;
	doc)
		exec_doc
		;;
	unittest)
		exec_unittest
		;;
	test)
		exec_test $2
		;;
	valgrind)
		exec_valgrind
		;;
	cppcheck)
		exec_cppcheck
		;;
	cccc)
		exec_cccc
		;;
	check-coverage)
		exec_unittest_gcov
		;;
	gcovr)
		exec_gcovr
		;;
	lcov)
		exec_lcov
		;;
	pmccabe)
		exec_filelist
		exec_pmccabe
		;;
	todo)
		exec_todo
		;;
	*)
		echo ""
		echo "error: unknown command: $1"
		usage $0
		exit -1
		;;
esac

exit 0

