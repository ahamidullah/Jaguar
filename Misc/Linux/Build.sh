#!/bin/bash

set -e

BUILD_DIRECTORY=$PROJECT_DIRECTORY/Build
CODE_DIRECTORY=$PROJECT_DIRECTORY/Code

pushd . >& /dev/null
cd $CODE_DIRECTORY

DEVELOPMENT=true

SHARED_COMPILER_FLAGS="-std=c++17 -DUSE_VULKAN_RENDER_API -I$PROJECT_DIRECTORY/Code -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers"

if [ $(stat -c %Y $CODE_DIRECTORY/Builder/Builder.cpp) > $(stat -c %Y $BUILD_DIRECTORY/Builder) ]; then
	g++ -c $SHARED_COMPILER_FLAGS -DDEBUG -DDEVELOPMENT -g -O3 $CODE_DIRECTORY/Basic/Basic.cpp -o $BUILD_DIRECTORY/libBasic.o
	ar rcs $BUILD_DIRECTORY/libBasic.a $BUILD_DIRECTORY/libBasic.o

	g++ $SHARED_COMPILER_FLAGS -DDEBUG -DDEVELOPMENT -g -O3 $CODE_DIRECTORY/Builder/Builder.cpp $BUILD_DIRECTORY/libBasic.a -ldl -lpthread -o $BUILD_DIRECTORY/Builder
fi

Builder "$@"

#if [ "$1" = "ShaderPreprocessor" ]; then
	#g++ $SHARED_COMPILER_FLAGS ShaderPreprocessor/ShaderPreprocessor.cpp -o $BUILD_DIRECTORY/ShaderPreprocessor
#elif [ "$1" = "asset_packer" ]; then
	#echo todo: build assets
#elif [ "$1" = "" -o "$1" = "Game" ]; then
	# @TODO: What is -D_GNU_SOURCE used for?
	# @TODO: Fix -rdynamic so stacktrace prints function names and line numbers.
	# @TODO: Vendor freetype library.
	# @TODO: Vendor assimp library.
	#GAME_COMPILER_FLAGS=" -D_GNU_SOURCE -DUSE_VULKAN_RENDER_API -IDependencies/Vulkan/1.1.106.0/include"
	#GAME_LINKER_FLAGS="$PROJECT_DIRECTORY/Build/Media.a $PROJECT_DIRECTORY/Build/Basic.a -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread"
	#g++ $SHARED_COMPILER_FLAGS $GAME_COMPILER_FLAGS Basic/Basic.cpp $GAME_LINKER_FLAGS -o $BUILD_DIRECTORY/$PROJECT_NAME
	#g++ -c $SHARED_COMPILER_FLAGS Basic/Basic.cpp -ldl -lpthread -o $BUILD_DIRECTORY/Basic.o
	#ar rvs $BUILD_DIRECTORY/Basic.a $BUILD_DIRECTORY/Basic.o
	#g++ -c $SHARED_COMPILER_FLAGS -DUSE_VULKAN_RENDER_API Media/Media.cpp -lX11 -lXi -o $BUILD_DIRECTORY/Media.o
	#ar rvs $BUILD_DIRECTORY/Media.a $BUILD_DIRECTORY/Media.o
	#g++ $SHARED_COMPILER_FLAGS Builder/Builder.cpp $PROJECT_DIRECTORY/Build/Basic.a -ldl -lpthread -o $BUILD_DIRECTORY/Builder
	#g++ $SHARED_COMPILER_FLAGS $GAME_COMPILER_FLAGS Engine/Engine.cpp $GAME_LINKER_FLAGS -o $BUILD_DIRECTORY/Engine
	#g++ -c $SHARED_COMPILER_FLAGS Engine/Engine.cpp -o $BUILD_DIRECTORY/Engine.o
	#ar rvs $BUILD_DIRECTORY/Engine.a $BUILD_DIRECTORY/Engine.o
#else
	#echo Unknown build parameter
	#exit
#fi

#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/pbr.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/pbr_vertex.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/pbr.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/pbr_fragment.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/shadow_map.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/shadow_map_vertex.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/shadow_map.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/shadow_map_fragment.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/flat_color.glsl -DVERTEX_SHADER -S vert -o $BUILD_DIRECTORY/flat_color_vertex.spirv
#"$VULKAN_SDK_PATH"/bin/glslangValidator -V shaders/flat_color.glsl -DFRAGMENT_SHADER -S frag -o $BUILD_DIRECTORY/flat_color_fragment.spirv

popd >& /dev/null
