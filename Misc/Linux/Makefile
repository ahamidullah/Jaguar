Sources = $(shell find $(PROJECT_DIRECTORY)/Code/Basic -type f -name "*.cpp") # $(wildcard $(PROJECT_DIRECTORY)/Code/Basic/*.cpp)
Objects = $(Sources:$(PROJECT_DIRECTORY)/Code/%.cpp=$(PROJECT_DIRECTORY)/Build/Linux/GCC/Object/%.o)
Dependencies = $(Sources:$(PROJECT_DIRECTORY)/Code/%.cpp=$(PROJECT_DIRECTORY)/Build/Linux/GCC/Dependency/%.d)

Binary: $(Objects)
	$(CC) -o $@ $^ $(LDFLAGS)

%o: %.cpp

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.cpp
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

-include $(Dependencies)
