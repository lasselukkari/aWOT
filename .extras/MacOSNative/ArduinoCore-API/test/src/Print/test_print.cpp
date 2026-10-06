/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include <api/Print.h>

#include <PrintMock.h>
#include <PrintableMock.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Print::print(char)", "[Print-print-01]")
{
  PrintMock mock;

  mock.print('A');

  REQUIRE(mock._str == "A");
}

TEST_CASE ("Print::print(const String &)", "[Print-print-02]")
{
  PrintMock mock;
  arduino::String const str("Test String");

  mock.print(str);

  REQUIRE(mock._str == "Test String");
}

TEST_CASE ("Print::print(const char str[])", "[Print-print-03]")
{
  PrintMock mock;
  const char str[] = "Test String";

  mock.print(str);

  REQUIRE(mock._str == "Test String");
}

TEST_CASE ("Print::print(int, int = DEC|HEX|OCT|BIN)", "[Print-print-04]")
{
  PrintMock mock;

  int const val = -1;

  WHEN("DEC") { mock.print(val, DEC); REQUIRE(mock._str  == "-1"); }
  WHEN("HEX") { mock.print(val, HEX); REQUIRE(mock._str  == "FFFFFFFFFFFFFFFF"); }
  WHEN("OCT") { mock.print(val, OCT); REQUIRE(mock._str  == "1777777777777777777777"); }
  WHEN("BIN") { mock.print(val, BIN); REQUIRE(mock._str  == "1111111111111111111111111111111111111111111111111111111111111111"); }
}

TEST_CASE ("Print::print(unsigned int, int = DEC|HEX|OCT|BIN)", "[Print-print-05]")
{
  PrintMock mock;

  unsigned int const val = 17;

  WHEN("DEC") { mock.print(val, DEC); REQUIRE(mock._str  == "17"); }
  WHEN("HEX") { mock.print(val, HEX); REQUIRE(mock._str  == "11"); }
  WHEN("OCT") { mock.print(val, OCT); REQUIRE(mock._str  == "21"); }
  WHEN("BIN") { mock.print(val, BIN); REQUIRE(mock._str  == "10001"); }
}

TEST_CASE ("Print::print(long, int = DEC|HEX|OCT|BIN)", "[Print-print-06]")
{
  PrintMock mock;

  long const val = -1;

  WHEN("DEC") { mock.print(val, DEC); REQUIRE(mock._str  == "-1"); }
  WHEN("HEX") { mock.print(val, HEX); REQUIRE(mock._str  == "FFFFFFFFFFFFFFFF"); }
  WHEN("OCT") { mock.print(val, OCT); REQUIRE(mock._str  == "1777777777777777777777"); }
  WHEN("BIN") { mock.print(val, BIN); REQUIRE(mock._str  == "1111111111111111111111111111111111111111111111111111111111111111"); }
}

TEST_CASE ("Print::print(unsigned long, int = DEC|HEX|OCT|BIN)", "[Print-print-07]")
{
  PrintMock mock;

  unsigned long const val = 17;

  WHEN("DEC") { mock.print(val, DEC); REQUIRE(mock._str  == "17"); }
  WHEN("HEX") { mock.print(val, HEX); REQUIRE(mock._str  == "11"); }
  WHEN("OCT") { mock.print(val, OCT); REQUIRE(mock._str  == "21"); }
  WHEN("BIN") { mock.print(val, BIN); REQUIRE(mock._str  == "10001"); }
}

TEST_CASE ("Print::print(long long, int = DEC|HEX|OCT|BIN)", "[Print-print-08]")
{
  PrintMock mock;

  long long const val = -1;

  WHEN("DEC") { mock.print(val, DEC); REQUIRE(mock._str  == "-1"); }
  WHEN("HEX") { mock.print(val, HEX); REQUIRE(mock._str  == "FFFFFFFFFFFFFFFF"); }
  WHEN("OCT") { mock.print(val, OCT); REQUIRE(mock._str  == "1777777777777777777777"); }
  WHEN("BIN") { mock.print(val, BIN); REQUIRE(mock._str  == "1111111111111111111111111111111111111111111111111111111111111111"); }
}

TEST_CASE ("Print::print(unsigned long long, int = DEC|HEX|OCT|BIN)", "[Print-print-09]")
{
  PrintMock mock;

  GIVEN("a value of zero ...")
  {
    unsigned long long const val = 0;

    WHEN("DEC") { mock.print(val, DEC); REQUIRE(mock._str  == "0"); }
    WHEN("HEX") { mock.print(val, HEX); REQUIRE(mock._str  == "0"); }
    WHEN("OCT") { mock.print(val, OCT); REQUIRE(mock._str  == "0"); }
    WHEN("BIN") { mock.print(val, BIN); REQUIRE(mock._str  == "0"); }
  }
  GIVEN("a non-zero value ...")
  {
    unsigned long long const val = 17;

    WHEN("DEC") { mock.print(val, DEC); REQUIRE(mock._str  == "17"); }
    WHEN("HEX") { mock.print(val, HEX); REQUIRE(mock._str  == "11"); }
    WHEN("OCT") { mock.print(val, OCT); REQUIRE(mock._str  == "21"); }
    WHEN("BIN") { mock.print(val, BIN); REQUIRE(mock._str  == "10001"); }
  }
}

TEST_CASE ("Print::print(double, int = 2)", "[Print-print-10]")
{
  PrintMock mock;

  WHEN ("val is a positive floating point value")
  {
    double const val = 3.1459;
    WHEN("digits = 0")           { mock.print(val, 0); REQUIRE(mock._str  == "3"); }
    WHEN("digits = 1")           { mock.print(val, 1); REQUIRE(mock._str  == "3.1"); }
    WHEN("digits = 2 (default)") { mock.print(val);    REQUIRE(mock._str  == "3.15"); }
    WHEN("digits = 3")           { mock.print(val, 3); REQUIRE(mock._str  == "3.146"); }
    WHEN("digits = 4")           { mock.print(val, 4); REQUIRE(mock._str  == "3.1459"); }
    WHEN("digits = 5")           { mock.print(val, 5); REQUIRE(mock._str  == "3.14590"); }
  }

  WHEN ("digits are negative")
  {
    double const val = 3.1459;
    WHEN("digits = -1") { mock.print(val, -1); REQUIRE(mock._str  == "3.15"); }
  }

  WHEN ("val is a negative floating point value")
  {
    double const val = -3.1459;
    WHEN("digits = 2 (default)") { mock.print(val); REQUIRE(mock._str  == "-3.15"); }
  }

  WHEN ("val is NAN")      { mock.print(NAN);      REQUIRE(mock._str  == "nan"); }
  WHEN ("val is INFINITY") { mock.print(INFINITY); REQUIRE(mock._str  == "inf"); }
}

TEST_CASE ("Print::print(Printable)", "[Print-print-11]")
{
  PrintMock mock;

  PrintableMock printable;
  printable._i = 1;

  mock.print(printable);

  REQUIRE(mock._str  == "PrintableMock i = 1");
}

TEST_CASE ("Print::print(unsigned char, int)", "[Print-print-12]")
{
  PrintMock mock;

  WHEN("DEC") { mock.print('A', DEC); REQUIRE(mock._str  == "65"); }
  WHEN("HEX") { mock.print('A', HEX); REQUIRE(mock._str  == "41"); }
  WHEN("OCT") { mock.print('A', OCT); REQUIRE(mock._str  == "101"); }
  WHEN("BIN") { mock.print('A', BIN); REQUIRE(mock._str  == "1000001"); }
}

TEST_CASE ("Testing Print::print(const __FlashStringHelper *)", "[Print-print-13]")
{
#undef F
#define F(string_literal) (reinterpret_cast<const arduino::__FlashStringHelper *>(PSTR(string_literal)))
  PrintMock mock;

  mock.print(F("Hello flash string"));

  REQUIRE(mock._str  == "Hello flash string");
}
