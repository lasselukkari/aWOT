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

TEST_CASE ("Testing String::lastIndexOf(char ch)", "[String-lastIndexOf-01]")
{
  WHEN ("str is empty")
  {
    arduino::String str;
    REQUIRE(str.lastIndexOf('a') == -1);
  }
  WHEN ("str does not contained searched element")
  {
    arduino::String str("Hello");
    REQUIRE(str.lastIndexOf('a') == -1);
  }
  WHEN ("str does contain searched element")
  {
    arduino::String str("Hellolol");
    REQUIRE(str.lastIndexOf('l') == 7);
  }
}

TEST_CASE ("Testing String::lastIndexOf(char ch, unsigned int fromIndex)", "[String-lastIndexOf-02]")
{
  WHEN ("str is empty")
  {
    arduino::String str;
    REQUIRE(str.lastIndexOf('a', 5) == -1);
  }
  WHEN ("str does not contained searched element")
  {
    arduino::String str("Hallo");
    REQUIRE(str.lastIndexOf('o', 3) == -1);
  }
  WHEN ("str does contain searched element")
  {
    arduino::String str("Hellolol");
    REQUIRE(str.lastIndexOf('l', 3) == 3);
  }
}

TEST_CASE ("Testing String::lastIndexOf(const String &)", "[String-lastIndexOf-03]")
{
  arduino::String const search_str("Arduino");

  WHEN ("str is empty")
  {
    arduino::String str;
    REQUIRE(str.lastIndexOf(search_str) == -1);
  }
  WHEN ("str does not contained searched element")
  {
    arduino::String str("Hallo");
    REQUIRE(str.lastIndexOf(search_str) == -1);
  }
  WHEN ("str does contain searched element")
  {
    arduino::String str("Hello Arduino, Arduino!");
    REQUIRE(str.lastIndexOf(search_str) == 15);
  }
}

TEST_CASE ("Testing String::lastIndexOf(const String &, unsigned int fromIndex)", "[String-lastIndexOf-04]")
{
  arduino::String const search_str("Arduino");

  WHEN ("str is empty")
  {
    arduino::String str;
    REQUIRE(str.lastIndexOf(search_str, 3) == -1);
  }
  WHEN ("str does not contained searched element")
  {
    arduino::String str("Hallo");
    REQUIRE(str.lastIndexOf(search_str, 3) == -1);
  }
  WHEN ("str does contain searched element and fromIndex is > start of searched element")
  {
    arduino::String str("Hello Arduino, Arduino!");
    REQUIRE(str.lastIndexOf(search_str, 17) == 15);
  }
  WHEN ("str does contain searched element and fromIndex is < start of searched element")
  {
    arduino::String str("Hello Arduino, Arduino!");
    REQUIRE(str.lastIndexOf(search_str, 3) == -1);
  }
}
