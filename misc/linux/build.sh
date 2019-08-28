#!/bin/bash

set -e

pushd . >& /dev/null

cd $PROJECT_DIRECTORY/code

BUILD_DIRECTORY=$PROJECT_DIRECTORY/build

if [ ! -d $BUILD_DIRECTORY ]; then
	mkdir $BUILD_DIRECTORY
fi

DEVELOPMENT=true
DEBUG=true

#COMPILER_FLAGS="-DDEBUG -std=c++17 -g -O0 -ffast-math -fno-exceptions -fno-threadsafe-statics -Idependencies/vulkan/1.1.106.0/include"

COMPILER_FLAGS="-DDEBUG -std=gnu99 -g -O0 -ffast-math -fno-exceptions -Idependencies/vulkan/1.1.106.0/include -Wall -Wextra -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wundef -Werror -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers"

if [ DEVELOPMENT ]; then
	COMPILER_FLAGS+=" -DDEVELOPMENT -I/usr/include/freetype2"
fi
if [ DEBUG ]; then
	COMPILER_FLAGS+=" -DDEBUG"
fi

LINKER_FLAGS="-lX11 -ldl -lm -lfreetype -lXi -lassimp"
if [ DEVELOPMENT ]; then
	LINKER_FLAGS+=""
fi

"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/pbr.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/pbr_vertex.spirv
"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/pbr.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/pbr_fragment.spirv

"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/shadow_map.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/shadow_map_vertex.spirv
"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/shadow_map.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/shadow_map_fragment.spirv

"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/flat_color.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/flat_color_vertex.spirv
"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/flat_color.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/flat_color_fragment.spirv

gcc $COMPILER_FLAGS preprocessor/preprocessor.c $LINKER_FLAGS -o $BUILD_DIRECTORY/preprocessor
$BUILD_DIRECTORY/preprocessor

gcc $COMPILER_FLAGS $PROJECT_NAME.c $LINKER_FLAGS -o $BUILD_DIRECTORY/$PROJECT_NAME

popd >& /dev/null
