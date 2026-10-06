/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>
#include <algorithm>

#include <api/WCharacter.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("isWhitespace(...) is called with ' '", "[isWhitespace-01]")
{
  REQUIRE(arduino::isWhitespace(' ') == true);
}

TEST_CASE ("isWhitespace(...) is called with '\t'", "[isWhitespace-02]")
{
  REQUIRE(arduino::isWhitespace('\t') == true);
}

TEST_CASE ("isWhitespace(...) is called with a non whitespace char", "[isWhitespace-03]")
{
  REQUIRE(arduino::isWhitespace('\r') == false);
}
