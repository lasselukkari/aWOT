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

TEST_CASE ("Testing String::trim with space at the beginning", "[String-trim-01]")
{
  arduino::String str("  hello");
  str.trim();
  REQUIRE(str == "hello");
}

TEST_CASE ("Testing String::trim with space at the end", "[String-trim-02]")
{
  arduino::String str("hello  ");
  str.trim();
  REQUIRE(str == "hello");
}

TEST_CASE ("Testing String::trim with space at both beginning and end", "[String-trim-03]")
{
  arduino::String str("  hello  ");
  str.trim();
  REQUIRE(str == "hello");
}

TEST_CASE ("Testing String::trim with space in the middle", "[String-trim-04]")
{
  arduino::String str("Hello Arduino!");
  str.trim();
  REQUIRE(str == "Hello Arduino!");
}
