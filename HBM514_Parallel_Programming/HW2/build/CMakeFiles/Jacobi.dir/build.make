# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/build

# Include any dependencies generated for this target.
include CMakeFiles/Jacobi.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Jacobi.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Jacobi.dir/flags.make

CMakeFiles/Jacobi.dir/LinearEquation.cpp.o: CMakeFiles/Jacobi.dir/flags.make
CMakeFiles/Jacobi.dir/LinearEquation.cpp.o: ../LinearEquation.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Jacobi.dir/LinearEquation.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Jacobi.dir/LinearEquation.cpp.o -c /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/LinearEquation.cpp

CMakeFiles/Jacobi.dir/LinearEquation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Jacobi.dir/LinearEquation.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/LinearEquation.cpp > CMakeFiles/Jacobi.dir/LinearEquation.cpp.i

CMakeFiles/Jacobi.dir/LinearEquation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Jacobi.dir/LinearEquation.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/LinearEquation.cpp -o CMakeFiles/Jacobi.dir/LinearEquation.cpp.s

# Object files for target Jacobi
Jacobi_OBJECTS = \
"CMakeFiles/Jacobi.dir/LinearEquation.cpp.o"

# External object files for target Jacobi
Jacobi_EXTERNAL_OBJECTS =

Jacobi: CMakeFiles/Jacobi.dir/LinearEquation.cpp.o
Jacobi: CMakeFiles/Jacobi.dir/build.make
Jacobi: /usr/lib64/openmpi/lib/libmpi_cxx.so
Jacobi: /usr/lib64/openmpi/lib/libmpi.so
Jacobi: CMakeFiles/Jacobi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Jacobi"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Jacobi.dir/link.txt --verbose=$(VERBOSE)
	/usr/bin/cmake -E copy_if_different /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/build/Jacobi /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2

# Rule to build all files generated by this target.
CMakeFiles/Jacobi.dir/build: Jacobi

.PHONY : CMakeFiles/Jacobi.dir/build

CMakeFiles/Jacobi.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Jacobi.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Jacobi.dir/clean

CMakeFiles/Jacobi.dir/depend:
	cd /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2 /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2 /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/build /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/build /run/media/pival/E8BA3B54BA3B1E8E/ITU/HBM514_Parallel_Programming/HW2/build/CMakeFiles/Jacobi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Jacobi.dir/depend

