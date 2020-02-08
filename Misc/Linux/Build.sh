#!/bin/bash

set -e

BUILD_DIRECTORY=$PROJECT_DIRECTORY/Build
if [ ! -d $BUILD_DIRECTORY ]; then
	mkdir $BUILD_DIRECTORY
fi
if [ ! -d $BUILD_DIRECTORY/ShaderPreprocessor ]; then
	mkdir $BUILD_DIRECTORY/ShaderPreprocessor
fi

pushd . >& /dev/null
cd $PROJECT_DIRECTORY/Code

DEVELOPMENT=true

SHARED_COMPILER_FLAGS=" -std=c++17 -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers"
if [ DEVELOPMENT ]; then
	SHARED_COMPILER_FLAGS+=" -DDEBUG -DDEVELOPMENT -g -O0"
else
	SHARED_COMPILER_FLAGS+=" -O3"
fi

if [ "$1" = "ShaderPreprocessor" ]; then
	g++ $SHARED_COMPILER_FLAGS ShaderPreprocessor/ShaderPreprocessor.cpp -o $BUILD_DIRECTORY/ShaderPreprocessor
elif [ "$1" = "asset_packer" ]; then
	echo todo: build assets
elif [ "$1" = "" -o "$1" = "Game" ]; then
	# @TODO: What is -D_GNU_SOURCE used for?
	# @TODO: Fix -rdynamic so stacktrace prints function names and line numbers.
	# @TODO: Vendor freetype library.
	# @TODO: Vendor assimp library.
	GAME_COMPILER_FLAGS=" -D_GNU_SOURCE -DUSE_VULKAN_RENDER_API -IDependencies/Vulkan/1.1.106.0/include"
	GAME_LINKER_FLAGS=" -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread"
	g++ $SHARED_COMPILER_FLAGS $GAME_COMPILER_FLAGS Game/Jaguar.cpp $GAME_LINKER_FLAGS -o $BUILD_DIRECTORY/$PROJECT_NAME
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
