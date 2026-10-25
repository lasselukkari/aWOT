cmake_minimum_required(VERSION 3.18)
project(aWOT-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(AWOT_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(AWOT_ENABLE_COVERAGE "Enable LLVM source coverage and the coverage-http target" OFF)
option(AWOT_NO_STD_FUNCTION_MIDDLEWARE "Use function pointer middleware" OFF)
option(AWOT_SMALL_BUFFERS "Test the Uno-sized URL, output and pushback buffers" OFF)

if(AWOT_ENABLE_COVERAGE)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR NOT UNIX OR CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR "HTTP coverage requires a native Clang build on macOS or Linux")
  endif()
  find_package(Python3 3.6 REQUIRED COMPONENTS Interpreter)

  # Prefer the LLVM tools from the compiler's own installation. Apple's compiler
  # launcher lives in /usr/bin, so ask xcrun for the selected toolchain as well.
  get_filename_component(AWOT_CLANG_PATH "${CMAKE_CXX_COMPILER}" REALPATH)
  get_filename_component(AWOT_LLVM_DIR "${AWOT_CLANG_PATH}" DIRECTORY)
  if(APPLE)
    execute_process(COMMAND xcrun --find llvm-cov
      OUTPUT_VARIABLE AWOT_XCRUN_LLVM_COV OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(AWOT_XCRUN_LLVM_COV)
      get_filename_component(AWOT_XCRUN_LLVM_DIR "${AWOT_XCRUN_LLVM_COV}" DIRECTORY)
      list(APPEND AWOT_LLVM_DIR "${AWOT_XCRUN_LLVM_DIR}")
    endif()
  endif()
  string(REGEX MATCH "^[0-9]+" AWOT_CLANG_MAJOR "${CMAKE_CXX_COMPILER_VERSION}")
  find_program(AWOT_LLVM_COV NAMES llvm-cov llvm-cov-${AWOT_CLANG_MAJOR}
    HINTS ${AWOT_LLVM_DIR} REQUIRED)
  find_program(AWOT_LLVM_PROFDATA NAMES llvm-profdata llvm-profdata-${AWOT_CLANG_MAJOR}
    HINTS ${AWOT_LLVM_DIR} REQUIRED)

  # Instrument library definitions in every translation unit, including inline
  # and template code in tests. Skipping test helpers avoids profile collisions
  # between their differing definitions in separate executables.
  add_compile_options(-O0 -g -fprofile-instr-generate -fcoverage-mapping)
  add_compile_options("-fprofile-list=${CMAKE_CURRENT_SOURCE_DIR}/scripts/coverage-files.txt")
  add_link_options(-fprofile-instr-generate)
endif()

# Every translation unit must use the same configuration and class layouts.
add_compile_definitions(DEBUG)
if(AWOT_NO_STD_FUNCTION_MIDDLEWARE)
  add_compile_definitions(AWOT_NO_STD_FUNCTION_MIDDLEWARE)
endif()
if(AWOT_SMALL_BUFFERS)
  add_compile_definitions(SERVER_URL_BUFFER_SIZE=64 SERVER_OUTPUT_BUFFER_SIZE=32 SERVER_PUSHBACK_BUFFER_SIZE=32)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)
  endif()
  if(AWOT_ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
    add_link_options(-fsanitize=address,undefined -fno-sanitize-recover=all)
  endif()
elseif(MSVC)
  add_compile_options(/W4)
  if(AWOT_ENABLE_SANITIZERS)
    message(FATAL_ERROR "The sanitizer configuration requires GCC or Clang")
  endif()
endif()

enable_testing()
file(GLOB EPOXY CONFIGURE_DEPENDS epoxy/*.cpp)
add_library(${PROJECT_NAME}-EpoxyDuino OBJECT ${EPOXY})
target_include_directories(${PROJECT_NAME}-EpoxyDuino PUBLIC epoxy)
target_compile_definitions(${PROJECT_NAME}-EpoxyDuino PRIVATE EPOXY_CORE_ESP8266)

file(GLOB UNITTEST CONFIGURE_DEPENDS unittest/*.cpp)
add_library(${PROJECT_NAME}-unittest OBJECT ${UNITTEST})
target_include_directories(${PROJECT_NAME}-unittest PUBLIC epoxy unittest)

file(GLOB AWOT_SOURCES CONFIGURE_DEPENDS ../../src/*.cpp)
add_library(${PROJECT_NAME}-aWOT OBJECT ${AWOT_SOURCES})
target_include_directories(${PROJECT_NAME}-aWOT PUBLIC ../../src epoxy)

foreach(TEST_GROUP http regression)
  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${TEST_GROUP}/*.cpp)
  foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    if(AWOT_NO_STD_FUNCTION_MIDDLEWARE AND TEST_NAME STREQUAL "member-functions")
      continue() # Member-function binding is intentionally unavailable in this mode.
    endif()
    add_executable(${PROJECT_NAME}-${TEST_NAME} ${TEST_FILE})
    target_include_directories(${PROJECT_NAME}-${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(${PROJECT_NAME}-${TEST_NAME} PRIVATE ${PROJECT_NAME}-aWOT ${PROJECT_NAME}-EpoxyDuino ${PROJECT_NAME}-unittest)
    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}-${TEST_NAME}>)
    set_tests_properties(${TEST_NAME} PROPERTIES LABELS ${TEST_GROUP})
    list(APPEND TEST_LIST ${PROJECT_NAME}-${TEST_NAME})
    if(TEST_GROUP STREQUAL "http")
      list(APPEND HTTP_TEST_LIST ${PROJECT_NAME}-${TEST_NAME})
      string(APPEND HTTP_COVERAGE_EXECUTABLES "$<TARGET_FILE:${PROJECT_NAME}-${TEST_NAME}>\n")
    endif()
  endforeach()
endforeach()

add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure DEPENDS ${TEST_LIST})

if(AWOT_ENABLE_COVERAGE)
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/http-coverage-$<CONFIG>.txt"
    CONTENT "${HTTP_COVERAGE_EXECUTABLES}")
  add_custom_target(coverage-http
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/coverage.py"
      --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
      --source-dir "${CMAKE_CURRENT_SOURCE_DIR}/../../src"
      --manifest "${CMAKE_CURRENT_BINARY_DIR}/http-coverage-$<CONFIG>.txt"
      --llvm-cov "${AWOT_LLVM_COV}" --llvm-profdata "${AWOT_LLVM_PROFDATA}"
      --ctest "${CMAKE_CTEST_COMMAND}" --config "$<CONFIG>"
    DEPENDS ${HTTP_TEST_LIST}
    USES_TERMINAL VERBATIM)
endif()
