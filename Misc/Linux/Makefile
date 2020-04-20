CODE_DIRECTORY = $(PROJECT_DIRECTORY)/Code
BUILD_DIRECTORY = $(PROJECT_DIRECTORY)/Build

MODULE_NAMES := Basic Media Engine Game
SOURCE_FILES := $(MODULE_NAMES:%=$(CODE_DIRECTORY)/%.cpp)
DEP_FILES := $(MODULE_NAMES:%=$(BUILD_DIRECTORY)/%.d)

$(info SOURCE_FILES is $(SOURCE_FILES))
$(info PROJECT_DIRECTORY is $(PROJECT_DIRECTORY))
$(info DEP_FILES is $(DEP_FILES))

COMPILER = g++

COMPILER_FLAGS = -std=c++17 -rdynamic -DUSE_VULKAN_RENDER_API -I$PROJECT_DIRECTORY/Code -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers

ifeq ($(DEBUG), true)
	COMPILER_FLAGS += -DDEBUG -g -O0
else
	COMPILER_FLAGS += -O3
endif

All: Game

Game: Game.o Engine
	$(COMPILER) $(COMPILER_FLAGS) -I$(PROJECT_DIRECTORY)/Dependencies/Vulkan/1.1.106.0/include -o $(BUILD_DIRECTORY)/Game.o $(BUILD_DIRECTORY)/libEngine.so $(BUILD_DIRECTORY)/libMedia.so $(BUILD_DIRECTORY)/libBasic.so -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread

Engine: Engine.o Media
	$(COMPILER) -o $(BUILD_DIRECTORY)/libEngine.so $(BUILD_DIRECTORY)/Engine.o "-fPIC"

Media: Media.o Basic
	$(COMPILER) -o $(BUILD_DIRECTORY)/libMedia.so $(BUILD_DIRECTORY)/Media.o "-fPIC"

Basic: Basic.o
	$(COMPILER) -o $(BUILD_DIRECTORY)/libBasic.so $(BUILD_DIRECTORY)/Basic.o "-fPIC"

-include $(DEP_FILES)

Game.d: $(CODE_DIRECTORY)/Game/Game.cpp
	@$(COMPILER) $(COMPILER_FLAGS) -I$(PROJECT_DIRECTORY)/Dependencies/Vulkan/1.1.106.0/include $< -MM -MT $(@:.d=.o) >$@

Engine.d: $(CODE_DIRECTORY)/Engine/Engine.cpp
	@$(COMPILER) $(COMPILER_FLAGS) $< -MM -MT $(@:.d=.o) >$@

Basic.d: $(CODE_DIRECTORY)/Basic/Basic.cpp
	@$(COMPILER) $(COMPILER_FLAGS) $< -MM -MT $(@:.d=.o) >$@

Media.d: $(CODE_DIRECTORY)/Media/Media.cpp
	@$(COMPILER) $(COMPILER_FLAGS) $< -MM -MT $(@:.d=.o) >$@
