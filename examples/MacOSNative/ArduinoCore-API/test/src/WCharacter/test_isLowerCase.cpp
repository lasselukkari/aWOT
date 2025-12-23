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

TEST_CASE ("isLowerCase(...) is called with a upper case number", "[isLowerCase-01]")
{
  REQUIRE(arduino::isLowerCase('A') == false);
}

TEST_CASE ("isLowerCase(...) is called with a lower case number", "[isLowerCase-02]")
{
  REQUIRE(arduino::isLowerCase('a') == true);
}

TEST_CASE ("isLowerCase(...) is called with a non-alphabetic number", "[isLowerCase-03]")
{
  REQUIRE(arduino::isLowerCase('0') == false);
}
