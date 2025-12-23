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

TEST_CASE ("Testing String::toFloat when string is empty", "[String-toFloat-01]")
{
  arduino::String str;
  float const val = str.toFloat();
  REQUIRE(val == 0.0f);
}

TEST_CASE ("Testing String::toFloat when string contains no number", "[String-toFloat-02]")
{
  arduino::String str("abc");
  float const val = str.toFloat();
  REQUIRE(val == 0.0f);
}

TEST_CASE ("Testing String::toFloat when string contains a number", "[String-toFloat-03]")
{
  arduino::String str("-1.2345");
  float const val = str.toFloat();
  REQUIRE(val == -1.2345f);
}
