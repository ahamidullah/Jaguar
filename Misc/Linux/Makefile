SourceDirectory = $(PROJECT_DIRECTORY)/Code
ObjectDirectory = $(PROJECT_DIRECTORY)/Build/Linux/Object
DependencyDirectory = $(PROJECT_DIRECTORY)/Build/Linux/Dependency

Sources = $(shell find $(SourceDirectory)/Basic -type f -name "*.cpp")
Objects = $(Sources:$(SourceDirectory)/%.cpp=$(ObjectDirectory)/%.o)
Dependencies = $(Sources:$(SourceDirectory)/%.cpp=$(DependencyDirectory)/%.d)

CompilerFlags = -std=c++17 -DUSE_VULKAN_RENDER_API -ICode -ffast-math -fno-exceptions -Wall -Wextra -Wfatal-errors -Wcast-align -Wdisabled-optimization -Wformat=2 -Winit-self -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wundef -Wno-unused -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

$(shell mkdir -p $(dir $(Dependencies)) >/dev/null)
$(shell mkdir -p $(dir $(Objects)) >/dev/null)

# C flags
CFLAGS := -std=c11
# C++ flags
CXXFLAGS := -std=c++11
# C/C++ flags
CPPFLAGS := -g -Wall -Wextra -pedantic
# linker flags
LDFLAGS :=
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DependencyDirectory)/$*.Td

# compile C++ source files
COMPILE.cc = g++ $(DEPFLAGS) $(CompilerFlags) -c -o $@
# link object files to binary
LINK.o = $(LD) $(LDFLAGS) $(LDLIBS) -o $@
# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = mv -f $(DependencyDirectory)/$*.Td $(DependencyDirectory)/$*.d

Binary : $(Objects)
	$(LINK.o) $^

$(ObjectDirectory)/%.o: $(SourceDirectory)/%.cpp
$(ObjectDirectory)/%.o: $(SourceDirectory)/%.cpp $(DependencyDirectory)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS: $(DependencyDirectory)/%.d
$(DependencyDirectory)/%.d: ;

-include $(Dependencies)

clean:
	rm -rf $(ObjectDirectory)/*
	rm -rf $(DependencyDirectory)/*
