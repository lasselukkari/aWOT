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

TEST_CASE ("Testing String::charAt(unsigned int)", "[String-charAt-01]")
{
  arduino::String str1("Hello");
  REQUIRE(str1.charAt(2) == 'l');
}

TEST_CASE ("Testing String::setCharAt(unsigned int, char )", "[String-setCharAt-02]")
{
  arduino::String str1("Hello");
  str1.setCharAt(1, 'a');
  REQUIRE(str1 == "Hallo");
}

TEST_CASE ("Testing String::getBytes(unsigned char, unsigned int, unsigned int)", "[String-getBytes-02]")
{
  WHEN("No bufsize") {
    arduino::String str("Hello");
    unsigned char buf[2];
    str.getBytes(buf, 0, 0);
  }

  WHEN("Index >= len") {
    arduino::String str("Hello");
    unsigned char buf[2];
    str.getBytes(buf, 5, 6);
  }

  WHEN("Valid operation") {
    arduino::String str("Hello");
    unsigned char buf[3];
    str.getBytes(buf, 5, 3);
    REQUIRE(buf[0] == 'l');
    REQUIRE(buf[1] == 'o');
    REQUIRE(buf[2] == '\0');
  }
}

TEST_CASE ("Testing & String::operator[]", "[String-&operator subscript-03]")
{
  arduino::String str("Hello");
  str[0] = 'M';
  REQUIRE(str == "Mello");
}

TEST_CASE ("Testing & String::operator[] with invalid buffer", "[String-&operator subscript-04]")
{
  char *buffer = NULL;
  arduino::String str(buffer);
  str[0] = 'M';
  REQUIRE(str[0] == 0);
}
