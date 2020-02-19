CODE_DIRECTORY = $(PROJECT_DIRECTORY)/Code
BUILD_DIRECTORY = $(PROJECT_DIRECTORY)/Build

SOURCE_FILES = $(CODE_DIRECTORY)/Basic/Basic.cpp $(CODE_DIRECTORY)/Media/Media.cpp $(CODE_DIRECTORY)/Engine/Engine.cpp
DEP_FILES := $(patsubst %,$(BUILD_DIRECTORY)/%.d,$(basename $(SOURCE_FILES)))

LIB_BASIC_FILEPATH = $(BUILD_DIRECTORY)/libBasic.so
LIB_MEDIA_FILEPATH = $(BUILD_DIRECTORY)/libMedia.so

#obj = $(src:.c=.o)
#dep = $(obj:.o=.d)  # one dependency file for each source

COMPILER = g++ # @TODO: Should this be gcc?

COMMON_COMPILER_FLAGS = -std=c++17 -DUSE_VULKAN_RENDER_API -I$(CODE_DIRECTORY) -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers
COMMON_LINKER_FLAGS = 

ifeq ($(DEBUG), true)
	COMMON_COMPILER_FLAGS += -DDEBUG -g -O0
else
	COMMON_COMPILER_FLAGS += -O3

GAME_COMPILER_FLAGS = $(COMMON_COMPILER_FLAGS) -D_GNU_SOURCE -I$(PROJECT_DIRECTORY)/Dependencies/Vulkan/1.1.106.0/include
GAME_LINKER_FLAGS = $(COMMON_LINKER_FLAGS) $(LIB_MEDIA_FILEPATH) $(LIB_BASIC_FILEPATH) -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread

Engine: $(BUILD_DIRECTORY)/Engine.o libMedia
    $(COMPILER) -o $(BUILD_DIRECTORY)/Engine $(BUILD_DIRECTORY)/Engine.o $(GAME_LINKER_FLAGS)

libMedia: $(BUILD_DIRECTORY)/libMedia.o
    $(COMPILER) -o $(LIB_MEDIA_FILEPATH) $(BUILD_DIRECTORY)/Engine.o $(GAME_LINKER_FLAGS)

libBasic: $(BUILD_DIRECTORY)/libBasic.o
    $(COMPILER) -o $(BUILD_DIRECTORY)/$@ $(BUILD_DIRECTORY)/Engine.o $(GAME_LINKER_FLAGS)

-include $(DEP_FILES)

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
Engine.d: $(CODE_DIRECTORY)/Engine/Engine.cpp
    @$(CPP) $(GAME_COMPILER_FLAGS) $< -MM -MT $(@:.d=.o) >$@

Basic.d: $(CODE_DIRECTORY)/Basic/Basic.cpp
    @$(CPP) $(COMMON_COMPILER_FLAGS) $< -MM -MT $(@:.d=.o) >$@

Media.d: $(CODE_DIRECTORY)/Media/Media.cpp
    @$(CPP) $(COMMON_COMPILER_FLAGS) $< -MM -MT $(@:.d=.o) >$@
