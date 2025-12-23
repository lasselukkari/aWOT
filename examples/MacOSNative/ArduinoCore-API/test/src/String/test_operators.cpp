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

TEST_CASE ("Testing String::operator + (const StringSumHelper, const String)", "[String-operator+-01]")
{
  arduino::String str1("Hello ");
  arduino::String str2("Arduino");
  arduino::String str("Hello Arduino");
  REQUIRE(str == str1+str2);
}

TEST_CASE ("Testing String::operator + (const StringSumHelper, const char *)", "[String-operator+-02]")
{
  arduino::String str1("Hello ");
  arduino::String str("Hello Arduino");
  REQUIRE(str == str1+"Arduino");
}

TEST_CASE ("Testing String::operator + (const StringSumHelper, char)", "[String-operator+-03]")
{
  arduino::String str1("Hello");
  char ch='!';
  arduino::String str("Hello!");
  REQUIRE(str == str1+ch);
}

TEST_CASE ("Testing String::operator + (const StringSumHelper, unsigned char)", "[String-operator+-04]")
{
  arduino::String str1("Hello ");
  unsigned char ch='A';
  arduino::String str("Hello 65"); /* ASCII['A'] = 65 */
  REQUIRE(str == str1+ch);
}

TEST_CASE ("Testing String::operator + (const StringSumHelper, int)", "[String-operator+-05]")
{
  arduino::String str1("Hello ");
  arduino::String str("Hello 1");
  REQUIRE(str == str1+1);
}

TEST_CASE ("Testing String::operator + (unsigned int)", "[String-operator+-06]")
{
  arduino::String str1("Hello ");
  unsigned int const num = 1;
  arduino::String str("Hello 1");
  REQUIRE(str == str1+num);
}

TEST_CASE ("Testing String::operator + (long)", "[String-operator+-07]")
{
  arduino::String str1("Hello ");
  long const num = -1;
  arduino::String str("Hello -1");
  REQUIRE(str == str1+num);
}

TEST_CASE ("Testing String::operator + (unsigned long)", "[String-operator+-08]")
{
  arduino::String str1("Hello ");
  unsigned long const num = 1;
  arduino::String str("Hello 1");
  REQUIRE(str == str1+num);
}

TEST_CASE ("Testing String::operator + (float)", "[String-operator+-09]")
{
  arduino::String str1("Hello ");
  float const num = 1.234f;
  arduino::String str("Hello 1.23");
  REQUIRE(str == str1+num);
}

TEST_CASE ("Testing String::operator + (double)", "[String-operator+-10]")
{
  arduino::String str1("Hello ");
  double const num = 5.678;
  arduino::String str("Hello 5.68");
  REQUIRE(str == str1+num);
}

TEST_CASE ("Testing String::operator + (const __FlashStringHelper *)", "[String-operator+-11]")
{
#undef F
#define F(string_literal) (reinterpret_cast<const arduino::__FlashStringHelper *>(PSTR(string_literal)))
  arduino::String str1("Hello ");
  arduino::String str("Hello Arduino");
  REQUIRE(str == str1+F("Arduino"));
}

TEST_CASE ("Testing & String::operator = (StringSumHelper &&rval)", "[String-operator+-12]")
{
  arduino::String str1("Hello ");
  arduino::String str = (str1+"Arduino");
  REQUIRE(str == "Hello Arduino");
}

TEST_CASE ("Testing & String::operator = (const String &) with invalid buffer of second string", "[String-operator+-13]")
{
  arduino::String str1("Hello");

  char *buffer2 = NULL;
  arduino::String str2(buffer2);

  str1 = str2;
  REQUIRE(str1 == str2);
}

TEST_CASE ("Testing & String::operator = (const char *) with NULL does not leave string unchanged", "[String-operator+-14]")
{
  char *buffer = NULL;
  arduino::String str("Hello");
  str = buffer;
  REQUIRE(str != "Hello");
}

TEST_CASE ("Testing & String::operator = (const char *) with NULL produces invalid string", "[String-operator+-14]")
{
  char *buffer = NULL;
  arduino::String str("Hello");
  str = buffer;
  REQUIRE_FALSE(str);
}

TEST_CASE ("Testing & String::operator = (const String &) with invalid buffer of first string", "[String-operator+-15]")
{
  char *buffer1 = NULL;
  arduino::String str1(buffer1);

  arduino::String str2("Hello");

  str1 = str2;
  REQUIRE(str1 == str2);
}

TEST_CASE ("Testing & String::operator = (String &&)", "[String-operator+-16]")
{
  arduino::String str("Hello");
  arduino::String str1("Arduino");
  str1 = static_cast<arduino::String&&>(str);
  REQUIRE(str1 == "Hello");
}

TEST_CASE ("Testing & String::operator = (StringSumHelper &&)", "[String-operator+-17]")
{
  arduino::String str("Hello");
  char const ch = '!';
  arduino::String str1("Arduino");
  str1 = static_cast<arduino::StringSumHelper&&>(str+ch);
  REQUIRE(str1 == "Hello!");
}
