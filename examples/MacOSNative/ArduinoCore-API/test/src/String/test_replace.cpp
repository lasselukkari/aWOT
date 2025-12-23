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

TEST_CASE ("Testing String::replace(char, char) when string is empty", "[String-replace-01]")
{
  arduino::String str;
  str.replace('a', 'b');
  REQUIRE(str.length() == 0);
}

TEST_CASE ("Testing String::replace(char, char) when string contains elements != 'find'", "[String-replace-02]")
{
  arduino::String str("Hello Arduino!");
  str.replace('Z', '0');
  REQUIRE(str == "Hello Arduino!");
}

TEST_CASE ("Testing String::replace(char, char) when string contains elements = 'find'", "[String-replace-03]")
{
  arduino::String str("Hello Arduino!");
  str.replace('o', '0');
  str.replace('e', '3');
  str.replace('i', '1');
  REQUIRE(str == "H3ll0 Ardu1n0!");
}

TEST_CASE ("Testing String::replace(String, String) when string does not contain substr 'find'", "[String-replace-04]")
{
  arduino::String str("Hello Arduino!");
  str.replace(arduino::String("Zulu"), arduino::String("11"));
  REQUIRE(str == "Hello Arduino!");
}

TEST_CASE ("Testing String::replace(String, String) when string contains substr 'find'", "[String-replace-05]")
{
  arduino::String str("Hello Arduino!");
  str.replace(arduino::String("ll"), arduino::String("11"));
  REQUIRE(str == "He11o Arduino!");
}

TEST_CASE ("Testing String::replace(String, String) substr 'find' larger than 'replace'", "[String-replace-06]")
{
  arduino::String str("Hello Arduino!");
  str.replace(arduino::String("llo"), arduino::String("11"));
  REQUIRE(str == "He11 Arduino!");
}

TEST_CASE ("Testing String::replace(String, String) substr 'find' smaller than 'replace'", "[String-replace-07]")
{
  arduino::String str("Hello Arduino!");
  str.replace(arduino::String("ll"), arduino::String("111"));
  REQUIRE(str == "He111o Arduino!");
}

TEST_CASE ("Testing String::replace(String, String) substr 'find' smaller than 'replace' multiple occurencies", "[String-replace-08]")
{
  arduino::String str("Hello Arduino! Hello, Hello, Hello");
  str.replace(arduino::String("ll"), arduino::String("lll"));
  REQUIRE(str == "Helllo Arduino! Helllo, Helllo, Helllo");
}

TEST_CASE ("Testing String::replace(String, String) substr 'find' same length as 'replace' multiple occurencies", "[String-replace-09]")
{
  arduino::String str("Hello Arduino! Hello, Hello, Hello");
  str.replace(arduino::String("ll"), arduino::String("11"));
  REQUIRE(str == "He11o Arduino! He11o, He11o, He11o");
}

TEST_CASE ("Testing String::replace(String, String) substr 'find' larger than 'replace' multiple occurencies", "[String-replace-10]")
{
  arduino::String str("Helllo Arduino! Helllo, Helllo, Helllo");
  str.replace(arduino::String("lll"), arduino::String("ll"));
  REQUIRE(str == "Hello Arduino! Hello, Hello, Hello");
}
