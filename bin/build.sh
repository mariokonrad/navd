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
	echo "    index    : creates a tags file, i.e. indexes the source"
	echo "    test     : executes the tests"
	echo "    valgrind : calls valgrind on tests to check for memory problems"
	echo ""
}

function exec_clean()
{
	rm -fr ${BASE}/build
	rm -f ${BASE}/tags
}

function exec_build()
{
	if [ ! -d "${BASE}/build" ] ; then
		mkdir -p ${BASE}/build
	fi
	cd ${BASE}/build
	if [ ! -r Makefile ] ; then
		cmake ..
	fi
	make
}

function exec_index()
{
	cd ${BASE}
	ctags --recurse -f tags src/*
}

function exec_test()
{
	if [ -r "${BASE}/build/src/nmea_test" ] ; then
		${BASE}/build/src/nmea_test
	else
		echo "error: test not present"
	fi
}

function exec_valgrind()
{
	if [ -r "${BASE}/build/src/nmea_test" ] ; then
		valgrind ${BASE}/build/src/nmea_test
	else
		echo "error: test not present"
	fi
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
	index)
		exec_index
		;;
	test)
		exec_test
		;;
	valgrind)
		exec_valgrind
		;;
	*)
		echo ""
		echo "error: unknown command: $1"
		usage $0
		exit -1
		;;
esac

exit 0

