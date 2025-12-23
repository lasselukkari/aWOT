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

TEST_CASE ("Testing String::remove(index) when string is empty", "[String-remove-01]")
{
  arduino::String str;
  str.remove(0);
  REQUIRE(str.length() == 0);
}

TEST_CASE ("Testing String::remove(index) when index is > string length", "[String-remove-02]")
{
  arduino::String str("Hello Arduino!");
  str.remove(100);
  REQUIRE(str == "Hello Arduino!");
}

TEST_CASE ("Testing String::remove(index) when index is < string length", "[String-remove-03]")
{
  arduino::String str("Hello Arduino!");
  str.remove(5);
  REQUIRE(str == "Hello");
}

TEST_CASE ("Testing String::remove(index,count) when string is empty", "[String-remove-04]")
{
  arduino::String str;
  str.remove(0, 10);
  REQUIRE(str.length() == 0);
}

TEST_CASE ("Testing String::remove(index,count) when index is > string length", "[String-remove-05]")
{
  arduino::String str("Hello Arduino!");
  str.remove(100, 10);
  REQUIRE(str == "Hello Arduino!");
}

TEST_CASE ("Testing String::remove(index,count) when index is < string length && count is > remaining length", "[String-remove-06]")
{
  arduino::String str("Hello Arduino!");
  str.remove(5, 100);
  REQUIRE(str == "Hello");
}

TEST_CASE ("Testing String::remove(index,count) when index is < string length && count is < remaining length", "[String-remove-07]")
{
  arduino::String str("Hello Arduino!");
  str.remove(5, 1);
  REQUIRE(str == "HelloArduino!");
}
