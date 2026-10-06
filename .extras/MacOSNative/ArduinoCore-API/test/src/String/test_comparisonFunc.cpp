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

TEST_CASE ("Testing String::equals(const String &) with exit status PASS", "[String-equals-01]")
{
  arduino::String str1("Hello"), str2("Hello");
  REQUIRE(str1.equals(str2) == 1);
}

TEST_CASE ("Testing String::operator==(const String &) with exit status PASS", "[String-equals-01]")
{
  arduino::String str1("Hello"), str2("Hello");
  REQUIRE(str1 == str2);
}

TEST_CASE ("Testing String::operator!=(const String &) with exit status FAIL", "[String-equals-01]")
{
  arduino::String str1("Hello"), str2("Hello");
  REQUIRE_FALSE(str1 != str2);
}

TEST_CASE ("Testing String::equals(const String &) with exit status FAIL", "[String-equals-02]")
{
  arduino::String str1("Hello"), str2("World");
  REQUIRE(str1.equals(str2) == 0);
}

TEST_CASE ("Testing String::operator==(const String &) with exit status FAIL", "[String-equals-02]")
{
  arduino::String str1("Hello"), str2("World");
  REQUIRE_FALSE(str1 == str2);
}

TEST_CASE ("Testing String::operator !=(const String &) with exit status PASS", "[String-equals-02]")
{
  arduino::String str1("Hello"), str2("World");
  REQUIRE(str1 != str2);
}

TEST_CASE ("Testing String::equals(const char *) with exit status PASS", "[String-equals-03]")
{
  arduino::String str1("Hello");
  REQUIRE(str1.equals("Hello") == 1);
}

TEST_CASE ("Testing String::operator ==(const char *) with exit status PASS", "[String-equals-03]")
{
  arduino::String str1("Hello");
  REQUIRE(str1 == "Hello");
}

TEST_CASE ("Testing String::operator !=(const char *) with exit status FAIL", "[String-equals-03]")
{
  arduino::String str1("Hello");
  REQUIRE_FALSE(str1 != "Hello");
}

TEST_CASE ("Testing String::equals(const char *) with exit status FAIL", "[String-equals-04]")
{
  arduino::String str1("Hello");
  REQUIRE(str1.equals("World") == 0);
}

TEST_CASE ("Testing String::operator ==(const char *) with exit status FAIL", "[String-equals-04]")
{
  arduino::String str1("Hello");
  REQUIRE_FALSE(str1 == "World");
}

TEST_CASE ("Testing String::operator !=(const char *) with exit status PASS", "[String-equals-04]")
{
  arduino::String str1("Hello");
  REQUIRE(str1 != "World");
}

TEST_CASE ("Testing String::equalsIgnoreCase(const String &) PASS with NON-empty string", "[String-equalsIgnoreCase-05]")
{
  arduino::String str1("Hello"), str2("Hello");
  REQUIRE(str1.equalsIgnoreCase(str2) == 1);
}

TEST_CASE ("Testing String::equalsIgnoreCase(const String &) FAIL with NON-empty string", "[String-equalsIgnoreCase-06]")
{
  arduino::String str1("Hello"), str2("Hel");
  REQUIRE(str1.equalsIgnoreCase(str2) == 0);
}

TEST_CASE ("Testing String::equalsIgnoreCase(const String &) FAIL with different strings", "[String-equalsIgnoreCase-07]")
{
  arduino::String str1("Hello"), str2("World");
  REQUIRE(str1.equalsIgnoreCase(str2) == 0);
}

TEST_CASE ("Testing String::equalsIgnoreCase(const String &) PASS with same string", "[String-equalsIgnoreCase-08]")
{
  arduino::String str1("Hello");
  REQUIRE(str1.equalsIgnoreCase(str1) == 1);
}

TEST_CASE ("Testing String::startsWith(const String &)", "[String-startsWith-09]")
{
  WHEN ("str2 is larger than str1")
  {
    arduino::String str1("Hello");
    arduino::String str2("Hello World");
    REQUIRE(str1.startsWith(str2) == 0);
  }
  WHEN ("str1 starts with str2")
  {
    arduino::String str1("Hello World");
    arduino::String str2("Hello");
    REQUIRE(str1.startsWith(str2) == 1);
  }
  WHEN ("str1 does NOT start with str2")
  {
    arduino::String str1("Hello World");
    arduino::String str2("Helo");
    REQUIRE(str1.startsWith(str2) == 0);
  }
}

TEST_CASE ("Testing String::endsWith(const String &)", "[String-endsWith-10]")
{
  WHEN ("str2 is larger than str1")
  {
    arduino::String str1("Hello");
    arduino::String str2("Hello World");
    REQUIRE(str1.endsWith(str2) == 0);
  }
  WHEN ("str1 ends with str2")
  {
    arduino::String str1("Hello World");
    arduino::String str2("World");
    REQUIRE(str1.endsWith(str2) == 1);
  }
  WHEN ("str1 does NOT end with str2")
  {
    arduino::String str1("Hello World");
    arduino::String str2("Helo");
    REQUIRE(str1.endsWith(str2) == 0);
  }
}
