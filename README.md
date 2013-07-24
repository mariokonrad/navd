navd
====

This software handles navigational data (currently only a subset of NMEA 0183
is supported) from one or more sources through filters to some destinations.
For example: read NMEA data from a serial port (to which a GPS is connected),
filters all sentences that are not GPRMC and writes them to another serial port,
while (at the same time) logs the data in a file).

It is also possible to use this software as a NMEA multiplexer, for example
to provide positional data to a navigation computer and the VHF, read from
two GPS devices (redundancy).

The software is also scriptable with Lua.

At this point in time, the documentation of the software is virtually non-existant.
However the goal is to test is rigurously (unit test, code analysis) using
the following tools:
- CUnit
- valgrind
- gcov / lcov
- cppcheck
- pmccabe
- cccc

The documentation from source is done with doxygen.

The software is built using cmake, using toolchain files for Linux (32bit, 64bit)
as well as ARM (gnueabi, generic). Tools needed to build the software:
- GCC (only the C compiler)
- flex
- bison

and the usual unix tools (bash, grep, etc.) as well as all tools used for
testing, analysis and documentation.

LICENSE
-------

This software is distributed under the terms of GPLv3. See ```LICENSE.txt```


TODO
----

- Implement support for SeaTalk
- Extend 'filters' to be full grown processes, no routing through the parent process

Consider:
- SQlite: https://sqlite.org
- JSON
- XML
  - mini-xml: http://svn.msweet.org/mxml/
  - https://github.com/lxfontes/ezxml
  - http://sourceforge.net/projects/sxmlc/

