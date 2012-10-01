#!/bin/bash

export SCRIPT_BASE=$(dirname `readlink -f $0`)
export BASE=${SCRIPT_BASE}/..

function usage()
{
	echo ""
	echo "usage: $1 command"
	echo ""
	echo "Commands:"
	echo "    clean    : cleans up the build"
	echo "    build    : builds the software"
	echo "    tags     : builds the tags file"
	echo "    doc      : creates the documentation"
	echo "    unittest : executes the unit tests"
	echo "    test     : executes the tests"
	echo "    cppcheck : performs cppcheck"
	echo "    valgrind : calls valgrind on tests to check for memory problems"
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
}

function exec_tags()
{
	exec_prepare
	cd ${BASE}
	ctags --recurse -f tags src/*
}

function exec_build()
{
	exec_prepare
	exec_tags
	cd ${BASE}/build
	if [ ! -r Makefile ] ; then
		CMAKE_BUILD_TYPE=Debug cmake ..
	fi
	make
}

function exec_doc()
{
	exec_prepare
	exec_tags
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
		exec_unittest_gcov
	else
		echo "error: unit tests not present"
	fi
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
	# TODO: valgrind over cunit tests

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
}

function exec_cppcheck()
{
	# unfortunately cppcheck cannot handle paths like '${BASE}/src' therefore
	# cppchecks must be executed from directory ${BASE}

	cppcheck -v -f --enable=all -i src/test/cunit -I src src
}

if [ $# == 0 ] ; then
	usage $0
	exit
fi

case $1 in
	clean)
		exec_clean
		;;
	build)
		exec_build
		;;
	tags)
		exec_tags
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
	check-coverage)
		rm -f buildfiles.txt sourcefiles.txt
		find build/src -name "*.gcno" |  while read fn ; do echo $(basename $fn) ; done | sed 's/\.gcno//g' | grep -Ev "parser\.tab\.c|lexer\.yy\.c" | sort > buildfiles.txt
		find src -name "*.c"  | grep -Ev "/lua/|/cunit/" | while read fn ; do echo $(basename $fn) ; done | sort > sourcefiles.txt
		echo ""
		echo "files not covered:"
		diff sourcefiles.txt buildfiles.txt
		echo ""
		rm -f buildfiles.txt sourcefiles.txt
		;;
	*)
		echo ""
		echo "error: unknown command: $1"
		usage $0
		exit -1
		;;
esac

exit 0

