/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <float.h>

#include <catch2/catch_test_macros.hpp>

#include <api/String.h>

#include "StringPrinter.h"

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Testing String(const char *) constructor()", "[String-Ctor-01]")
{
  char const CSTR[] = "Hello Arduino String Class";
  arduino::String str(CSTR);
  REQUIRE(str == CSTR);
}

TEST_CASE ("Testing String(const String &) constructor()", "[String-Ctor-02]")
{
  arduino::String str1("Hello Arduino String class"),
                  str2(str1);
  REQUIRE(str1 == str2);
}

TEST_CASE ("Testing String(const __FlashStringHelper) constructor()", "[String-Ctor-03]")
{
#undef F
#define F(string_literal) (reinterpret_cast<const arduino::__FlashStringHelper *>(PSTR(string_literal)))
  arduino::String str1(F("Hello"));
  REQUIRE(str1 == "Hello");
}

TEST_CASE ("Testing String(char) constructor()", "[String-Ctor-04]")
{
  char const ch = 'A';
  arduino::String str(ch);
  REQUIRE(str == "A");
}

TEST_CASE ("Testing String(unsigned char, unsigned char base = 10) constructor()", "[String-Ctor-05]")
{
  unsigned char const val = 1;
  arduino::String str(val);
  REQUIRE(str == "1");
}

TEST_CASE ("Testing String(int, unsigned char base = 10) constructor()", "[String-Ctor-06]")
{
  int const val = -1;
  arduino::String str(val);
  REQUIRE(str == "-1");
}

TEST_CASE ("Testing String(unsigned int, unsigned char base = 10) constructor()", "[String-Ctor-07]")
{
  unsigned int const val = 1;
  arduino::String str(val);
  REQUIRE(str == "1");
}

TEST_CASE ("Testing String(long, unsigned char base = 10) constructor()", "[String-Ctor-08]")
{
  long const val = -1;
  arduino::String str(val);
  REQUIRE(str == "-1");
}

TEST_CASE ("Testing String(unsigned long, unsigned char base = 10) constructor()", "[String-Ctor-09]")
{
  unsigned long const val = 1;
  arduino::String str(val);
  REQUIRE(str == "1");
}

TEST_CASE ("Testing String(float, unsigned char decimalPlaces = 2) constructor()", "[String-Ctor-10]")
{
  WHEN ("String::String (some float value)")
  {
    arduino::String str(1.234f);
    REQUIRE(str == "1.23");
  }
  WHEN ("String::String (FLT_MAX)")
  {
    arduino::String str(FLT_MAX);
    REQUIRE(str == "340282346638528859811704183484516925440.00");
  }
  WHEN ("String::String (-FLT_MAX)")
  {
    arduino::String str(-FLT_MAX);
    REQUIRE(str == "-340282346638528859811704183484516925440.00");
  }
}

TEST_CASE ("Testing String(double, unsigned char decimalPlaces = 2) constructor()", "[String-Ctor-11]")
{
  WHEN ("String::String (some double value)")
  {
    arduino::String str(5.678);
    REQUIRE(str == "5.68");
  }
  WHEN ("String::String (DBL_MAX)")
  {
    arduino::String str(DBL_MAX);
    REQUIRE(str == "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.00");
  }
  WHEN ("String::String (-DBL_MAX)")
  {
    arduino::String str(-DBL_MAX);
    REQUIRE(str == "-179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.00");
  }
}

TEST_CASE ("Testing String(const __FlashStringHelper) constructor() with invalid buffer", "[String-Ctor-12]")
{
#undef F
#define F(string_literal) (reinterpret_cast<const arduino::__FlashStringHelper *>(PSTR(string_literal)))
  char *buffer = NULL;

  arduino::String str1(F(buffer));
  REQUIRE_FALSE(str1);
}

TEST_CASE ("Testing String(StringSumHelper &&) constructor()", "[String-Ctor-13]")
{
  arduino::String str("Hello");
  char const ch = '!';
  arduino::String str1(static_cast<arduino::StringSumHelper&&>(str+ch));
  REQUIRE(str1 == "Hello!");
}

TEST_CASE ("Testing String(String &&) constructor()", "[String-Ctor-14]")
{
  arduino::String str("Hello");
  arduino::String str1(static_cast<arduino::String&&>(str));
  REQUIRE(str1 == "Hello");
}

TEST_CASE ("Testing String(String &&) with move(String &rhs) from smaller to larger buffer", "[String-Ctor-15]")
{
  arduino::String str("Hello");
  arduino::String str1("Arduino");
  str1 = static_cast<arduino::String&&>(str);
  REQUIRE(str1 == "Hello");
}

TEST_CASE ("Testing String(String &&) with move(String &rhs) from larger to smaller buffer", "[String-Ctor-16]")
{
  arduino::String str("Hello");
  arduino::String str1("Arduino");
  str = static_cast<arduino::String&&>(str1);
  REQUIRE(str == "Arduino");
}
