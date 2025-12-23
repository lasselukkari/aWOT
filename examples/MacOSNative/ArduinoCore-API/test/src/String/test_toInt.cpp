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

TEST_CASE ("Testing String::toInt when string is empty", "[String-toInt-01]")
{
  arduino::String str;
  int const val = str.toInt();
  REQUIRE(val == 0);
}

TEST_CASE ("Testing String::toInt when string contains no number", "[String-toInt-02]")
{
  arduino::String str("abc");
  int const val = str.toInt();
  REQUIRE(val == 0);
}

TEST_CASE ("Testing String::toInt when string contains a number", "[String-toInt-03]")
{
  arduino::String str("-1");
  int const val = str.toInt();
  REQUIRE(val == -1);
}

TEST_CASE ("Testing String::toInt when string does not have a buffer", "[String-toInt-04]")
{
  char *buffer = NULL;
  arduino::String str(buffer);
  REQUIRE(str.toInt() == 0);
}
