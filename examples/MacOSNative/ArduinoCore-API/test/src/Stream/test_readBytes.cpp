/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <StreamMock.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Testing readBytes(char *buffer, size_t length)", "[Stream-readBytes-01]")
{
  StreamMock mock;

  WHEN ("the stream is empty")
  {
    char buf[32] = {0};

    REQUIRE(mock.readBytes(buf, sizeof(buf)) == 0);
  }

  WHEN ("the stream contains less data than we want to read")
  {
    char buf[32] = {0};
    char const str[] = "some stream content";
    mock << str;

    REQUIRE(mock.readBytes(buf, sizeof(buf)) == strlen(str));
    REQUIRE(strncmp(buf, str, sizeof(buf)) == 0);
    REQUIRE(mock.readString() == arduino::String(""));
  }

  WHEN ("the stream contains more data than we want to read")
  {
    char buf[5] = {0};
    mock << "some stream content";
    char const EXPECTED_STR[] = "some ";

    REQUIRE(mock.readBytes(buf, sizeof(buf)) == 5);
    REQUIRE(strncmp(buf, EXPECTED_STR, sizeof(buf)) == 0);
    REQUIRE(mock.readString() == arduino::String("stream content"));
  }
}
