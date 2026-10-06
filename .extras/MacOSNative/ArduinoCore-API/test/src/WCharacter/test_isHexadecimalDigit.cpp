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

#include <vector>

#include <api/WCharacter.h>

/**************************************************************************************
 * CONSTANTS
 **************************************************************************************/

std::vector<char> const VALID_HEXADECIMAL_NUMBER_VECT = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','A','B','C','D','E','F'};

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("isHexadecimalDigit(...) is called with valid hexadecimal numbers", "[isHexadecimalDigit-01]")
{
  std::for_each(std::begin(VALID_HEXADECIMAL_NUMBER_VECT),
                std::end  (VALID_HEXADECIMAL_NUMBER_VECT),
                [](char const c)
                {
                  REQUIRE(arduino::isHexadecimalDigit(c) == true);
                });
}

TEST_CASE ("isHexadecimalDigit(...) is called with non hexadecimal number", "[isHexadecimalDigit-02]")
{
  REQUIRE(arduino::isHexadecimalDigit('z') == false);
}
