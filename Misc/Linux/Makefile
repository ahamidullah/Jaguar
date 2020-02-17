DEBUG := true

BUILD_DIRECTORY := $(PROJECT_DIRECTORY)/Build
CODE_DIRECTORY := $(PROJECT_DIRECTORY)/Code

BASIC_SOURCE_FILE := $(CODE_DIRECTORY)/Basic/Basic.cpp
MEDIA_SOURCE_FILE := $(CODE_DIRECTORY)/Media/Media.cpp
ENGINE_SOURCE_FILE := $(CODE_DIRECTORY)/Engine/Engine.cpp

COMPILER := g++ # @TODO: Should this be gcc?

COMMON_COMPILER_FLAGS := -std=c++17 -DUSE_VULKAN_RENDER_API -I$(CODE_DIRECTORY) -ffast-math -fno-exceptions -Wall -Wextra -Werror -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-sign-compare -Wno-missing-field-initializers

GAME_COMPILER_FLAGS := -D_GNU_SOURCE -I$(PROJECT_DIRECTORY)/Dependencies/Vulkan/1.1.106.0/include

GAME_LINKER_FLAGS := $(BUILD_DIRECTORY)/libMedia.a $(BUILD_DIRECTORY)/libBasic.a -lX11 -ldl -lm -lfreetype -lXi -lassimp -lpthread

ifeq ($(DEBUG), true)
	COMMON_COMPILER_FLAGS += -DDEBUG -g -O0
else
	COMMON_COMPILER_FLAGS += -O3

Game: $(ENGINE_SOURCE_FILE:.cpp=.o) $(MEDIA_SOURCE_FILE:.cpp=.o) $(BASIC_SOURCE_FILE:.cpp=.o)
	$(COMPILER) $(COMMON_COMPILER_FLAGS) $(GAME_COMPILER_FLAGS) $^ -o $(BUILD_DIRECTORY)/Game

# object files, auto generated from source files
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS)))
# dependency files, auto generated from source files
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

# compilers (at least gcc and clang) don't create the subdirectories automatically
$(shell mkdir -p $(dir $(OBJS)) >/dev/null)
$(shell mkdir -p $(dir $(DEPS)) >/dev/null)

# C compiler
CC := clang
# C++ compiler
CXX := clang++
# linker
LD := clang++
# tar
TAR := tar

# C flags
CFLAGS := -std=c11
# C++ flags
CXXFLAGS := -std=c++11
# C/C++ flags
CPPFLAGS := -g -Wall -Wextra -pedantic
# linker flags
LDFLAGS :=
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

# compile C source files
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@
# compile C++ source files
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@
# link object files to binary
LINK.o = $(LD) $(LDFLAGS) $(LDLIBS) -o $@
# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all: $(BIN)

dist: $(DISTFILES)
	$(TAR) -cvzf $(DISTOUTPUT) $^

.PHONY: clean
clean:
	$(RM) -r $(OBJDIR) $(DEPDIR)

.PHONY: distclean
distclean: clean
	$(RM) $(BIN) $(DISTOUTPUT)

.PHONY: install
install:
	@echo no install tasks configured

.PHONY: uninstall
uninstall:
	@echo no uninstall tasks configured

.PHONY: check
check:
	@echo no tests configured

.PHONY: help
help:
	@echo available targets: all dist clean distclean install uninstall check

$(BIN): $(OBJS)
	$(LINK.o) $^

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cc
$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cxx
$(OBJDIR)/%.o: %.cxx $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(DEPS)
