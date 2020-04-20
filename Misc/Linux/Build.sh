#!/bin/bash

set -e

BUILD_DIRECTORY=$PROJECT_DIRECTORY/Build
if [ ! -e $BUILD_DIRECTORY ]; then
	mkdir $BUILD_DIRECTORY
fi

CODE_DIRECTORY=$PROJECT_DIRECTORY/Code

COMMON_COMPILER_FLAGS=$(<$CODE_DIRECTORY/Builder/CommonCompilerFlags.txt)

BUILDER_OPTIMIZED=true

BUILDER_COMPILER_FLAGS=$COMMON_COMPILER_FLAGS
BUILDER_COMPILER_FLAGS+=" -g -DDEBUG -DDEVELOPMENT"
if [ $BUILDER_OPTIMIZED = "true" ]; then
	BUILDER_COMPILER_FLAGS+=" -O3"
else
	BUILDER_COMPILER_FLAGS+=" -O0"
fi

if [ ! -f $BUILD_DIRECTORY/Builder ] || [ "$1" = "Builder" ]; then
	echo -e "Building Builder...\n"

	BASIC_BUILD_COMMAND="g++ -c $BUILDER_COMPILER_FLAGS $CODE_DIRECTORY/Basic/Basic.cpp -o $BUILD_DIRECTORY/libBasic.o"
	echo -e "BASIC_BUILD_COMMAND: $BASIC_BUILD_COMMAND\n"

	BASIC_LINK_COMMAND="ar rcs $BUILD_DIRECTORY/libBasic.a $BUILD_DIRECTORY/libBasic.o"
	echo -e "BASIC_LINK_COMMAND: $BASIC_LINK_COMMAND\n"

	BUILDER_BUILD_COMMAND="g++ $BUILDER_COMPILER_FLAGS $CODE_DIRECTORY/Builder/Builder.cpp $BUILD_DIRECTORY/libBasic.a -ldl -lpthread -o $BUILD_DIRECTORY/Builder"
	echo -e "BUILDER_BUILD_COMMAND: $BUILDER_BUILD_COMMAND\n"

	$BASIC_BUILD_COMMAND

	$BASIC_LINK_COMMAND

	$BUILDER_BUILD_COMMAND

	echo -e "Build successful.\n"
fi

if [ "$1" != "Builder" ]; then
	echo -e "Running Builder..."

	pushd . >& /dev/null
	cd $PROJECT_DIRECTORY

	Builder $@

	popd >& /dev/null
fi
