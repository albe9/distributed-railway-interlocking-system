# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build

# Include any dependencies generated for this target.
include src/CMakeFiles/init.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/init.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/init.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/init.dir/flags.make

src/CMakeFiles/init.dir/initTask.c.o: src/CMakeFiles/init.dir/flags.make
src/CMakeFiles/init.dir/initTask.c.o: ../src/initTask.c
src/CMakeFiles/init.dir/initTask.c.o: src/CMakeFiles/init.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/CMakeFiles/init.dir/initTask.c.o"
	cd /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/src && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/init.dir/initTask.c.o -MF CMakeFiles/init.dir/initTask.c.o.d -o CMakeFiles/init.dir/initTask.c.o -c /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/src/initTask.c

src/CMakeFiles/init.dir/initTask.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/init.dir/initTask.c.i"
	cd /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/src && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/src/initTask.c > CMakeFiles/init.dir/initTask.c.i

src/CMakeFiles/init.dir/initTask.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/init.dir/initTask.c.s"
	cd /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/src && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/src/initTask.c -o CMakeFiles/init.dir/initTask.c.s

# Object files for target init
init_OBJECTS = \
"CMakeFiles/init.dir/initTask.c.o"

# External object files for target init
init_EXTERNAL_OBJECTS =

src/libinit.a: src/CMakeFiles/init.dir/initTask.c.o
src/libinit.a: src/CMakeFiles/init.dir/build.make
src/libinit.a: src/CMakeFiles/init.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libinit.a"
	cd /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/src && $(CMAKE_COMMAND) -P CMakeFiles/init.dir/cmake_clean_target.cmake
	cd /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/init.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/init.dir/build: src/libinit.a
.PHONY : src/CMakeFiles/init.dir/build

src/CMakeFiles/init.dir/clean:
	cd /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/src && $(CMAKE_COMMAND) -P CMakeFiles/init.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/init.dir/clean

src/CMakeFiles/init.dir/depend:
	cd /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/src /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/src /home/albe/distributed-railway-interlocking-system/test_script/virtualized_raspberry/build/src/CMakeFiles/init.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/init.dir/depend

