#!/bin/bash

# select platform
case $1 in
	1)
		PLATFORM="debug_x64"
		PLATFORM_PRINT="64-bit (Debug)"
		EXECUTABLE_EXTENSION="_debug_x64"
		;;
	2)
		PLATFORM="release_x64"
		PLATFORM_PRINT="64-bit (Release)"
		EXECUTABLE_EXTENSION="_x64"
		;;
	3)
		PLATFORM="debug_x32"
		PLATFORM_PRINT="32-bit (Debug)"
		EXECUTABLE_EXTENSION="_debug"
		;;
	4)
		PLATFORM="release_x32"
		PLATFORM_PRINT="32-bit (Release)"
		EXECUTABLE_EXTENSION=""
		;;
	*)
		echo
		echo "  Usage:  mz_build PLATFORM TARGET [BUILD_OPTIONS]"
		echo
		echo
		echo "  Platform:  ..."
		echo
		echo "  1: 64-bit (Debug)"
		echo "  2: 64-bit (Release)"
		echo
		echo "  3: 32-bit (Debug)"
		echo "  4: 32-bit (Release)"
		echo
		exit
esac

echo
echo "  Platform:  $PLATFORM_PRINT"

# get next command line option
shift 1

# select target
case $1 in
	a)
		MAKEFILE="all"
		MAKEFILE_PRINT="all targets"
		;;
	c)
		MAKEFILE="clean"
		MAKEFILE_PRINT="clean targets"
		;;
	u)
		MAKEFILE="unittest"
		MAKEFILE_PRINT="Unit tests"
		EXECUTABLE="unittest"
		;;
	1)
		MAKEFILE="kmeter_standalone_stereo"
		MAKEFILE_PRINT="Standalone (Stereo)"
		EXECUTABLE="kmeter_stereo"
		;;
	2)
		MAKEFILE="kmeter_standalone_surround"
		MAKEFILE_PRINT="Standalone (Surround)"
		EXECUTABLE="kmeter_surround"
		;;
	3)
		MAKEFILE="kmeter_vst2_stereo"
		MAKEFILE_PRINT="VST2 plug-in (Stereo)"
		;;
	4)
		MAKEFILE="kmeter_vst2_surround"
		MAKEFILE_PRINT="VST2 plug-in (Surround)"
		;;
	*)
		echo "  Target:    ..."
		echo
		echo "  a: All targets"
		echo "  c: Clean targets"
		echo "  u: Unit tests"
		echo
		echo "  1: Standalone (Stereo)"
		echo "  2: Standalone (Surround)"
		echo
		echo "  3: VST2 plug-in (Stereo)"
		echo "  4: VST2 plug-in (Surround)"
		echo
		exit
esac

echo "  Target:    $MAKEFILE_PRINT"

# get next command line option
shift 1

# display additional options
if [ -z "$1" ]; then
	echo "  Options:   --no-print-directory"
	echo
else
	echo "  Options:  --no-print-directory $*"
	echo
fi

# indent and format code
if [ "$MAKEFILE" != "clean" ]; then
	echo "==== Indenting and formatting code ===="
	cd "../Source" || exit
	"./format_code.sh"
	cd "../Builds" || exit
	echo
fi

# compile target
make --directory=linux/gmake/ --no-print-directory config=$PLATFORM $MAKEFILE $* 2>&1

# compilation was successful
if [ $? -eq 0 ]; then
	echo $MAKEFILE | grep "_standalone_" > /dev/null

	# target is standalone, so execute file
	if [ $? -eq 0 ]; then
        echo $EXECUTABLE$EXECUTABLE_EXTENSION
		echo
		../bin/standalone/$EXECUTABLE$EXECUTABLE_EXTENSION
	fi

	echo $MAKEFILE | grep "unittest" > /dev/null

	# target is unit test, so execute file
	if [ $? -eq 0 ]; then
        echo $EXECUTABLE$EXECUTABLE_EXTENSION
		echo
		../bin/unittest/$EXECUTABLE$EXECUTABLE_EXTENSION
	fi
fi
