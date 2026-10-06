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

TEST_CASE ("Testing String::length when string is empty", "[String-length-01]")
{
  arduino::String str;
  REQUIRE(str.length() == 0);
}

TEST_CASE ("Testing String::length when string contains characters", "[String-length-02]")
{
  arduino::String str("Testing String::length");
  REQUIRE(str.length() == strlen("Testing String::length"));
}
