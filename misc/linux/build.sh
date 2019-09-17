#!/bin/bash

set -e

DEVELOPMENT=true
DEBUG=true

BUILD_DIRECTORY=$PROJECT_DIRECTORY/build
if [ ! -d $BUILD_DIRECTORY ]; then
	mkdir $BUILD_DIRECTORY
fi

pushd . >& /dev/null
cd $PROJECT_DIRECTORY/code

# Build and run preprocessor.
gcc $COMPILER_FLAGS preprocessor.c $LINKER_FLAGS -o $BUILD_DIRECTORY/preprocessor
pushd . >& /dev/null
cd $PROJECT_DIRECTORY
$BUILD_DIRECTORY/preprocessor
popd >& /dev/null

# Build the game.
# @TODO: What is -D_GNU_SOURCE used for?
# @TODO: Fix -rdynamic so stacktrace prints function names and line numbers.
# @TODO: Vendor freetype library.
# @TODO: Vendor assimp library.
COMPILER_FLAGS="-D_GNU_SOURCE -std=c99 -g -O0 -ffast-math -fno-exceptions -Idependencies/vulkan/1.1.106.0/include -Wall -Wextra -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wundef -Werror -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers"
if [ DEVELOPMENT ]; then
	COMPILER_FLAGS+=" -DDEVELOPMENT -I/usr/include/freetype2"
fi
if [ DEBUG ]; then
	COMPILER_FLAGS+=" -DDEBUG"
fi

LINKER_FLAGS="-lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread"
if [ DEVELOPMENT ]; then
	LINKER_FLAGS+=""
fi
if [ DEBUG ]; then
	LINKER_FLAGS+=""
fi

"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/pbr.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/pbr_vertex.spirv
"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/pbr.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/pbr_fragment.spirv
"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/shadow_map.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/shadow_map_vertex.spirv
"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/shadow_map.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/shadow_map_fragment.spirv
"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/flat_color.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/flat_color_vertex.spirv
"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/flat_color.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/flat_color_fragment.spirv
gcc $COMPILER_FLAGS linux.c $LINKER_FLAGS -o $BUILD_DIRECTORY/$PROJECT_NAME

popd >& /dev/null
