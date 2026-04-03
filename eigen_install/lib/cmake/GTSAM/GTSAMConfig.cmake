# - Config file for GTSAM
# It defines the following variables
#  GTSAM_INCLUDE_DIR - include directories for GTSAM

if (POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD) # Don't complain about boost
endif()

# Compute paths
get_filename_component(OUR_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
if(EXISTS "${OUR_CMAKE_DIR}/CMakeCache.txt")
  # In build tree
  set(GTSAM_INCLUDE_DIR /home/wang/MID360_Project/gtsam CACHE PATH "GTSAM include directory")
else()
  # Find installed library
  set(GTSAM_INCLUDE_DIR "${OUR_CMAKE_DIR}/../../../include" CACHE PATH "GTSAM include directory")
endif()

# Find dependencies, required by cmake exported targets:
include(CMakeFindDependencyMacro)
# Allow using cmake < 3.8
if (ON OR ON)
  if(${CMAKE_VERSION} VERSION_LESS "3.8.0" OR ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.30.0")
    find_package(Boost 1.70 COMPONENTS graph;serialization;program_options;random;timer;chrono)
  else()
    find_dependency(Boost 1.70 COMPONENTS graph;serialization;program_options;random;timer;chrono)
  endif()
endif()

if()
    find_dependency(TBB 4.4 COMPONENTS tbb tbbmalloc)
endif()

if(OFF)
find_dependency(Eigen3 REQUIRED)
endif()

# Load exports
include(${OUR_CMAKE_DIR}/GTSAM-exports.cmake)

# Load project-specific flags, if present
if(EXISTS "${OUR_CMAKE_DIR}/gtsam_extra.cmake")
	include("${OUR_CMAKE_DIR}/gtsam_extra.cmake")
endif()

message(STATUS "GTSAM include directory:  ${GTSAM_INCLUDE_DIR}")
