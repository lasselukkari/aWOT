/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/String.h>

#include "StringPrinter.h"

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Testing String::compareTo(const String &)", "[String-compareTo-01]")
{
  WHEN ("Strings are equal")
  {
    arduino::String str1("Hello"), str2("Hello");
    REQUIRE(str1.compareTo(str2) == 0);
  }

  WHEN ("str2 is empty")
  {
    arduino::String str1("Hello"), str2;
    REQUIRE(str1.compareTo(str2) > 0);
  }

  WHEN ("str1 is empty")
  {
    arduino::String str1, str2("Hello");
    REQUIRE(str1.compareTo(str2) < 0);
  }
}

TEST_CASE ("Testing String::compareTo(const char *)", "[String-compareTo-02]")
{
  WHEN ("Strings are equal")
  {
    arduino::String str("Hello");
    REQUIRE(str.compareTo("Hello") == 0);
  }

  WHEN ("Passed string is empty")
  {
    arduino::String str("Hello");
    REQUIRE(str.compareTo("") > 0);
  }

  WHEN ("Passed string is compared with empty string")
  {
    arduino::String str;
    REQUIRE(str.compareTo("") == 0);
  }
}

TEST_CASE ("Testing String::compareTo(const char *) with empty buffer", "[String-compareTo-03]")
{
  WHEN ("First string has a valid buffer")
  {
    char *buffer = NULL;

    arduino::String str("Hello");
    REQUIRE(str.compareTo(buffer) != 0);
  }

  WHEN ("First string does NOT have a valid buffer")
  {
    char *buffer1 = NULL;
    char *buffer2 = NULL;

    arduino::String str(buffer1);
    REQUIRE(str.compareTo(buffer2) == 0);
  }
}


TEST_CASE ("Testing String::compareTo(const String &) with empty buffer", "[String-compareTo-04]")
{
  WHEN ("First string has a valid buffer")
  {
    char *buffer = NULL;

    arduino::String str1("Hello");
    arduino::String str2(buffer);
    REQUIRE(str1.compareTo(str2) != 0);
  }

  WHEN ("First string does NOT have a valid buffer")
  {
    char *buffer1 = NULL;
    char *buffer2 = NULL;

    arduino::String str1(buffer1);
    arduino::String str2(buffer2);
    REQUIRE(str1.compareTo(str2) == 0);
  }
}
