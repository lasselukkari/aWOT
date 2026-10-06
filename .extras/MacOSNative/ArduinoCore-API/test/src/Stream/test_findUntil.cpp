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

TEST_CASE ("Testing findUntil(const char *target, const char *terminator)", "[Stream-findUntil-01]")
{
  StreamMock mock;

  WHEN ("'target' is contained in stream")
  {
    WHEN ("'terminator' appears before 'target'")
    {
      mock << "This is a : test string";
      REQUIRE(mock.findUntil("test", ": ") == false);
      REQUIRE(mock.readString() == arduino::String("test string"));
    }
    WHEN ("'terminator' appears after 'target'")
    {
      mock << "This is a test : string";
      REQUIRE(mock.findUntil("test", ": ") == true);
      REQUIRE(mock.readString() == arduino::String(" : string"));
    }
    WHEN ("'terminator' is not included in the string at all")
    {
      mock << "This is a test string";
      REQUIRE(mock.findUntil("test", ": ") == true);
      REQUIRE(mock.readString() == arduino::String(" string"));
    }
  }

  WHEN ("'target' is not contained in stream")
  {
    mock << "This is a test string";
    REQUIRE(mock.findUntil("abc", "def") == false);
    REQUIRE(mock.readString() == arduino::String(""));
  }
}
