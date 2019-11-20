#!/bin/bash

set -e

BUILD_DIRECTORY=$PROJECT_DIRECTORY/build
if [ ! -d $BUILD_DIRECTORY ]; then
	mkdir $BUILD_DIRECTORY
fi

pushd . >& /dev/null
cd $PROJECT_DIRECTORY/code

DEVELOPMENT=true

SHARED_COMPILER_FLAGS=" -std=gnu99 -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers"
if [ DEVELOPMENT ]; then
	SHARED_COMPILER_FLAGS+=" -DDEBUG -DDEVELOPMENT -g -O0"
else
	SHARED_COMPILER_FLAGS+=" -O3"
fi

if [ "$1" = "material_compiler" ]; then
	gcc $SHARED_COMPILER_FLAGS material_compiler.c -o $BUILD_DIRECTORY/material_compiler
elif [ "$1" = "asset_packer" ]; then
	echo todo: build assets
elif [ "$1" = "" -o "$1" = "game" ]; then
	# @TODO: What is -D_GNU_SOURCE used for?
	# @TODO: Fix -rdynamic so stacktrace prints function names and line numbers.
	# @TODO: Vendor freetype library.
	# @TODO: Vendor assimp library.
	GAME_COMPILER_FLAGS=" -D_GNU_SOURCE -DUSE_VULKAN_RENDER_API -Idependencies/vulkan/1.1.106.0/include"
	GAME_LINKER_FLAGS=" -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread"
	gcc $SHARED_COMPILER_FLAGS $GAME_COMPILER_FLAGS linux.c $GAME_LINKER_FLAGS -o $BUILD_DIRECTORY/$PROJECT_NAME
else
	echo Unknown build parameter
	exit
fi

#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/pbr.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/pbr_vertex.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/pbr.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/pbr_fragment.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/shadow_map.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/shadow_map_vertex.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/shadow_map.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/shadow_map_fragment.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/flat_color.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/flat_color_vertex.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/flat_color.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/flat_color_fragment.spirv

popd >& /dev/null
