cmake_minimum_required( VERSION 3.18 )

project( aWOT-test LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED TRUE )

if( NOT BUILD_COMPILER )
  set( BUILD_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}" )
  message( STATUS "C++ compiler: " ${BUILD_COMPILER} )
endif()

if ( APPLE ) 
    set( CMAKE_OSX_ARCHITECTURES "x86_64;arm64" )
endif()

set( CMAKE_BUILD_TYPE "Debug" )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG" )

if ( CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments" )
elseif ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4" )
endif()

enable_testing()

aux_source_directory( epoxy EPOXY )

add_library( ${PROJECT_NAME}-EpoxyDuino OBJECT ${EPOXY} )

target_include_directories( ${PROJECT_NAME}-EpoxyDuino PUBLIC epoxy )

target_compile_definitions( ${PROJECT_NAME}-EpoxyDuino PRIVATE EPOXY_CORE_ESP8266 )

aux_source_directory( unittest UNITTEST )

add_library( ${PROJECT_NAME}-unittest OBJECT ${UNITTEST} )

target_include_directories( ${PROJECT_NAME}-unittest PUBLIC epoxy )

aux_source_directory( ../src aWOT )

add_library( ${PROJECT_NAME}-aWOT OBJECT ${aWOT} )

target_include_directories( ${PROJECT_NAME}-aWOT PUBLIC epoxy unittest )

file( GLOB TEST_SOURCES *.cpp )

foreach( TEST_FILE ${TEST_SOURCES} )
  get_filename_component( TEST_NAME ${TEST_FILE} NAME_WE )

  add_executable( ${PROJECT_NAME}-${TEST_NAME} ${TEST_FILE} )
  target_link_libraries( ${PROJECT_NAME}-${TEST_NAME} PRIVATE ${PROJECT_NAME}-aWOT ${PROJECT_NAME}-EpoxyDuino ${PROJECT_NAME}-unittest ) 

  add_test( NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}-${TEST_NAME}> )

  list( APPEND TEST_LIST ${PROJECT_NAME}-${TEST_NAME} )

endforeach()

add_custom_target( run_tests COMMAND ${CMAKE_CTEST_COMMAND} --verbose DEPENDS ${TEST_LIST} )
