# SPDX-License-Identifier: LGPL-2.1-or-later

##########################################################################

cmake_minimum_required(VERSION 2.8)

##########################################################################

project(test-ArduinoCore-API)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

##########################################################################

include_directories(..)
include_directories(include)

##########################################################################

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##########################################################################

set(TEST_TARGET ${CMAKE_PROJECT_NAME})

##########################################################################

set(TEST_SRCS
  src/CanMsg/test_CanMsg.cpp
  src/CanMsg/test_CanMsg_CopyCtor.cpp
  src/CanMsg/test_CanExtendedId.cpp
  src/CanMsg/test_CanStandardId.cpp
  src/CanMsg/test_isExtendedId.cpp
  src/CanMsg/test_isStandardId.cpp
  src/CanMsg/test_operator_assignment.cpp
  src/CanMsg/test_printTo.cpp
  src/CanMsgRingbuffer/test_available.cpp
  src/Common/test_makeWord.cpp
  src/Common/test_map.cpp
  src/Common/test_max.cpp
  src/Common/test_min.cpp
  src/IPAddress/test_toString.cpp
  src/IPAddress/test_fromString.cpp
  src/IPAddress/test_fromString6.cpp
  src/IPAddress/test_IPAddress.cpp
  src/IPAddress/test_IPAddress6.cpp
  src/IPAddress/test_operator_assignment.cpp
  src/IPAddress/test_operator_comparison.cpp
  src/IPAddress/test_operator_comparison6.cpp
  src/IPAddress/test_operator_parentheses.cpp
  src/IPAddress/test_operator_parentheses6.cpp
  src/IPAddress/test_printTo.cpp
  src/IPAddress/test_printTo6.cpp
  src/Print/test_clearWriteError.cpp
  src/Print/test_getWriteError.cpp
  src/Print/test_print.cpp
  src/Print/test_println.cpp
  src/Print/test_availableForWrite.cpp
  src/Ringbuffer/test_available.cpp
  src/Ringbuffer/test_availableForStore.cpp
  src/Ringbuffer/test_clear.cpp
  src/Ringbuffer/test_isFull.cpp
  src/Ringbuffer/test_peek.cpp
  src/Ringbuffer/test_read_char.cpp
  src/Ringbuffer/test_store_char.cpp
  src/Stream/test_find.cpp
  src/Stream/test_findUntil.cpp
  src/Stream/test_getTimeout.cpp
  src/Stream/test_parseFloat.cpp
  src/Stream/test_parseInt.cpp
  src/Stream/test_readBytes.cpp
  src/Stream/test_readBytesUntil.cpp
  src/Stream/test_readString.cpp
  src/Stream/test_readStringUntil.cpp
  src/Stream/test_setTimeout.cpp
  src/String/test_concat.cpp
  src/String/test_operators.cpp
  src/String/test_compareTo.cpp
  src/String/test_comparisonFunc.cpp
  src/String/test_characterAccessFunc.cpp
  src/String/test_substring.cpp
  src/String/test_indexOf.cpp
  src/String/test_lastIndexOf.cpp
  src/String/test_length.cpp
  src/String/test_move.cpp
  src/String/test_remove.cpp
  src/String/test_replace.cpp
  src/String/test_String.cpp
  src/String/test_toDouble.cpp
  src/String/test_toFloat.cpp
  src/String/test_toInt.cpp
  src/String/test_toLowerCase.cpp
  src/String/test_toUpperCase.cpp
  src/String/test_trim.cpp
  src/WCharacter/test_isAscii.cpp
  src/WCharacter/test_isControl.cpp
  src/WCharacter/test_isDigit.cpp
  src/WCharacter/test_isHexadecimalDigit.cpp
  src/WCharacter/test_isLowerCase.cpp
  src/WCharacter/test_isPunct.cpp
  src/WCharacter/test_isSpace.cpp
  src/WCharacter/test_isUpperCase.cpp
  src/WCharacter/test_isWhitespace.cpp
  src/WCharacter/test_toAscii.cpp
)

set(TEST_DUT_SRCS
  ../api/CanMsg.cpp
  ../api/CanMsgRingbuffer.cpp
  ../api/Common.cpp
  ../api/IPAddress.cpp
  ../api/String.cpp
  ../api/Stream.cpp
  ../api/Print.cpp
)

##########################################################################

set(TEST_TARGET_SRCS
  src/dtostrf.cpp
  src/itoa.cpp
  src/MillisFake.cpp
  src/PrintMock.cpp
  src/StreamMock.cpp
  ${TEST_SRCS}
  ${TEST_DUT_SRCS}
)

##########################################################################

add_compile_definitions(HOST)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
add_compile_options(-Wno-cast-function-type)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} --coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -Wno-deprecated-copy")

##########################################################################

add_executable(
  ${TEST_TARGET}
  ${TEST_TARGET_SRCS}
)

target_link_libraries( ${TEST_TARGET} Catch2WithMain )

##########################################################################

