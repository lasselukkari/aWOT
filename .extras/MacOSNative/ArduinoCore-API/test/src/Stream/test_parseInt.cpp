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

using namespace arduino;

TEST_CASE ("Testing parseInt(LookaheadMode lookahead = SKIP_ALL, char ignore = NO_IGNORE_CHAR)", "[Stream-parseInt-01]")
{
  StreamMock mock;

  WHEN ("A positive integer is contained in stream")
  {
    mock << "1234";
    REQUIRE(mock.parseInt() == 1234);
  }
  WHEN ("A negative integer is contained in stream")
  {
    mock << "-1234";
    REQUIRE(mock.parseInt() == -1234);
  }
  WHEN ("A integer is prepended by digits")
  {
    mock << "abcdef1234";
    REQUIRE(mock.parseInt() == 1234);
  }
  WHEN ("A integer is prepended by whitespace chars")
  {
    mock << "\r\n\t 1234";
    REQUIRE(mock.parseInt() == 1234);
  }
}

TEST_CASE ("Testing parseInt(LookaheadMode lookahead = SKIP_NONE, char ignore = NO_IGNORE_CHAR)", "[Stream-parseInt-02]")
{
  StreamMock mock;

  WHEN ("A positive integer is contained in stream")
  {
    mock << "1234";
    REQUIRE(mock.parseInt(SKIP_NONE) == 1234);
    REQUIRE(mock.readString() == arduino::String(""));
  }
  WHEN ("A integer is prepended by digits")
  {
    mock << "abcdef1234";
    REQUIRE(mock.parseInt(SKIP_NONE) == 0);
    REQUIRE(mock.readString() == arduino::String("abcdef1234"));
  }
  WHEN ("A integer is prepended by whitespace chars")
  {
    mock << "\r\n\t 1234";
    REQUIRE(mock.parseInt(SKIP_NONE) == 0);
    REQUIRE(mock.readString() == arduino::String("\r\n\t 1234"));
  }
}

TEST_CASE ("Testing parseInt(LookaheadMode lookahead = SKIP_WHITESPACE, char ignore = NO_IGNORE_CHAR)", "[Stream-parseInt-03]")
{
  StreamMock mock;

  WHEN ("A integer is prepended by whitespace chars")
  {
    mock << "\r\n\t 1234";
    REQUIRE(mock.parseInt(SKIP_WHITESPACE) == 1234);
    REQUIRE(mock.readString() == arduino::String(""));
  }
}

TEST_CASE ("Testing parseInt(LookaheadMode lookahead = SKIP_ALL, char ignore = 'a')", "[Stream-parseInt-04]")
{
  StreamMock mock;

  WHEN ("A positive integer is contained in stream")
  {
    mock << "1234";
    REQUIRE(mock.parseInt(SKIP_ALL, 'a') == 1234);
    REQUIRE(mock.readString() == arduino::String(""));
  }
  WHEN ("A integer contains only ignore char values")
  {
    mock << "12a3a4a";
    REQUIRE(mock.parseInt(SKIP_ALL, 'a') == 1234);
    REQUIRE(mock.readString() == arduino::String(""));
  }
  WHEN ("A integer contains other than ignore chars")
  {
    mock << "1bed234";
    REQUIRE(mock.parseInt(SKIP_ALL, 'a') == 1);
    REQUIRE(mock.readString() == arduino::String("bed234"));
  }
}
