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

std::vector<char> const VALID_DIGIT_VECT = {'0','1','2','3','4','5','6','7','8','9'};

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("isDigit(...) is called with valid digits", "[isDigit-01]")
{
  std::for_each(std::begin(VALID_DIGIT_VECT),
                std::end  (VALID_DIGIT_VECT),
                [](char const c)
                {
                  REQUIRE(arduino::isDigit(c) == true);
                });
}

TEST_CASE ("isDigit(...) is called with non digit", "[isDigit-02]")
{
  REQUIRE(arduino::isDigit('z') == false);
}
