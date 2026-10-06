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

TEST_CASE ("Testing readBytesUntil(char terminator, char *buffer, size_t length)", "[Stream-readBytesUntil-01]")
{
  StreamMock mock;

  WHEN ("the stream is empty")
  {
    char buf[32] = {0};

    REQUIRE(mock.readBytesUntil(' ', buf, sizeof(buf)) == 0);
  }

  WHEN ("the stream contains the termination character")
  {
    char buf[32] = {0};
    char const str[] = "some stream content";
    char const EXPECTED_STR[] = "some";
    mock << str;

    REQUIRE(mock.readBytesUntil(' ', buf, sizeof(buf)) == strlen("some"));
    REQUIRE(strncmp(buf, EXPECTED_STR, sizeof(buf)) == 0);
    REQUIRE(mock.readString() == arduino::String("stream content"));
  }

  WHEN ("the stream does not contain the termination character")
  {
    char buf[32] = {0};
    char const STR[] = "some stream content";
    mock << STR;

    REQUIRE(mock.readBytesUntil('!', buf, sizeof(buf)) == strlen(STR));
    REQUIRE(strncmp(buf, STR, sizeof(buf)) == 0);
    REQUIRE(mock.readString() == arduino::String(""));
  }
}
