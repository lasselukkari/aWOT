# 
# CMake file to build and run aWOT Unit Tests natively on MacOS or Linux systems.
# 
# Arduino Unit Tests from https://github.com/Arduino-CI/arduino_ci Apache-2.0 license
# EpoxyDuino Core from https://github.com/bxparks/EpoxyDuino MIT License
# 
# MIT License

cmake_minimum_required( VERSION 3.18 )

project( aWOT-test LANGUAGES CXX )

# Set C++ standard
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED TRUE )

# Set compiler identification
if( NOT BUILD_COMPILER )
  set( BUILD_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}" )
  message( STATUS "C++ compiler: " ${BUILD_COMPILER} )
endif()

# If macOS builds a Mach-O universal binary with 2 architectures: x86_64 and arm64 for Apple M processors
if ( APPLE ) 
    set( CMAKE_OSX_ARCHITECTURES "x86_64;arm64" )
endif()

# Set build type to "Debug"
set( CMAKE_BUILD_TYPE "Debug" )

# Ensures all binaries are placed in the same output directory
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} )

# Setting build type to "debug" add only -g 
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG" )

# Set C++ compiler flags
if ( CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments" )
elseif ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4" )
endif()

# Enable testing 
enable_testing()

# Add EpoxyDuino Core source files
aux_source_directory( epoxy EPOXY )

# Compile EpoxyDuino Core as an object library
add_library( ${PROJECT_NAME}-EpoxyDuino OBJECT ${EPOXY} )

# Add include directory to use #include <file.h> or "file.h"
target_include_directories( ${PROJECT_NAME}-EpoxyDuino PUBLIC epoxy )

# Defines EPOXY_CORE_ESP8266 macro for EpoxyDuino Core
target_compile_definitions( ${PROJECT_NAME}-EpoxyDuino PRIVATE EPOXY_CORE_ESP8266 )

# Add Arduino Unit Test source files
aux_source_directory( unittest UNITTEST )

# Compile Arduino Unit Test as an object library
add_library( ${PROJECT_NAME}-unittest OBJECT ${UNITTEST} )

# Add include directory to use #include <file.h> or "file.h"
target_include_directories( ${PROJECT_NAME}-unittest PUBLIC epoxy )

# Add aWOT source files
aux_source_directory( ../src aWOT )

# Compile aWOT as an object library
add_library( ${PROJECT_NAME}-aWOT OBJECT ${aWOT} )

# Add include directory to use #include <file.h> or "file.h"
target_include_directories( ${PROJECT_NAME}-aWOT PUBLIC epoxy unittest )

# List of test source files
file( GLOB TEST_SOURCES *.cpp )

foreach( TEST_FILE ${TEST_SOURCES} )

  # Get test name
  get_filename_component( TEST_NAME ${TEST_FILE} NAME_WE )

  # Compile test target
  add_executable( ${PROJECT_NAME}-${TEST_NAME} ${TEST_FILE} )
  target_link_libraries( ${PROJECT_NAME}-${TEST_NAME} PRIVATE ${PROJECT_NAME}-aWOT ${PROJECT_NAME}-EpoxyDuino ${PROJECT_NAME}-unittest ) 

  # Add test
  add_test( NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}-${TEST_NAME}> )

  # Add test to dependency list
  list( APPEND TEST_LIST ${PROJECT_NAME}-${TEST_NAME} )

endforeach()

# Add optional target to run tests
add_custom_target( run_tests COMMAND ${CMAKE_CTEST_COMMAND} --verbose DEPENDS ${TEST_LIST} )
