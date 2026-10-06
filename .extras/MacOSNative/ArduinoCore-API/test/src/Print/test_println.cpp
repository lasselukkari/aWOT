/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/Print.h>

#include <PrintMock.h>
#include <PrintableMock.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Print::println(char)", "[Print-println-01]")
{
  PrintMock mock;

  mock.println('A');

  REQUIRE(mock._str == "A\r\n");
}

TEST_CASE ("Print::println(const String &)", "[Print-println-02]")
{
  PrintMock mock;
  arduino::String const str("Test String");

  mock.println(str);

  REQUIRE(mock._str == "Test String\r\n");
}

TEST_CASE ("Print::println(const char str[])", "[Print-println-03]")
{
  PrintMock mock;
  const char str[] = "Test String";

  mock.println(str);

  REQUIRE(mock._str == "Test String\r\n");
}

TEST_CASE ("Print::println(int, int = DEC (default))", "[Print-println-04]")
{
  PrintMock mock;
  int const val = -1;

  mock.println(val);

  REQUIRE(mock._str == "-1\r\n");
}

TEST_CASE ("Print::println(unsigned int, int = DEC (default))", "[Print-println-05]")
{
  PrintMock mock;
  unsigned int const val = 17;

  mock.println(val);

  REQUIRE(mock._str == "17\r\n");
}

TEST_CASE ("Print::println(long, int = DEC (default))", "[Print-println-06]")
{
  PrintMock mock;
  long const val = -1;

  mock.println(val);

  REQUIRE(mock._str == "-1\r\n");
}

TEST_CASE ("Print::println(unsigned long, int = DEC (default))", "[Print-println-07]")
{
  PrintMock mock;
  unsigned long const val = 17;

  mock.println(val);

  REQUIRE(mock._str == "17\r\n");
}

TEST_CASE ("Print::println(long long, int = DEC (default))", "[Print-println-08]")
{
  PrintMock mock;
  long long const val = -1;

  mock.println(val);

  REQUIRE(mock._str == "-1\r\n");
}

TEST_CASE ("Print::println(unsigned long long, int = DEC|HEX|OCT|BIN)", "[Print-println-09]")
{
  PrintMock mock;
  unsigned long long const val = 17;

  mock.println(val);

  REQUIRE(mock._str == "17\r\n");
}

TEST_CASE ("Print::println(double, int = 2)", "[Print-println-10]")
{
  PrintMock mock;
  double const val = 3.1459;

  mock.println(val);

  REQUIRE(mock._str == "3.15\r\n");
}

TEST_CASE ("Print::println(Printable)", "[Print-println-11]")
{
  PrintMock mock;
  PrintableMock printable;
  printable._i = 1;

  mock.println(printable);

  REQUIRE(mock._str  == "PrintableMock i = 1\r\n");
}

TEST_CASE ("Print::println(unsigned char, int base = DEC (default))", "[Print-println-12]")
{
  PrintMock mock;

  mock.println('A', DEC);

  REQUIRE(mock._str == "65\r\n");
}

TEST_CASE ("Testing Print::println(const __FlashStringHelper *)", "[Print-println-13]")
{
#undef F
#define F(string_literal) (reinterpret_cast<const arduino::__FlashStringHelper *>(PSTR(string_literal)))
  PrintMock mock;

  mock.println(F("Hello flash string"));

  REQUIRE(mock._str  == "Hello flash string\r\n");
}
