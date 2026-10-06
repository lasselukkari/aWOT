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

TEST_CASE ("Testing String::toDouble when string is empty", "[String-toDouble-01]")
{
  arduino::String str;
  double const val = str.toDouble();
  REQUIRE(val == 0.0);
}

TEST_CASE ("Testing String::toDouble when string contains no number", "[String-toDouble-02]")
{
  arduino::String str("abc");
  double const val = str.toDouble();
  REQUIRE(val == 0.0);
}

TEST_CASE ("Testing String::toDouble when string contains a number", "[String-toDouble-03]")
{
  arduino::String str("-1.2345");
  double const val = str.toDouble();
  REQUIRE(val == -1.2345);
}

TEST_CASE ("Testing String::toDouble when string does not have a buffer", "[String-toDouble-04]")
{
  char *buffer = NULL;
  arduino::String str(buffer);
  REQUIRE(str.toDouble() == 0);
}
