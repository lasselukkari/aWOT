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

std::vector<char> const VALID_SPACE_VECT = {' ', '\t', '\n', '\v', '\f', '\r'};

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("isSpace(...) is called with a valid white space character", "[isSpace-01]")
{
  std::for_each(std::begin(VALID_SPACE_VECT),
                std::end  (VALID_SPACE_VECT),
                [](char const c)
                {
                  REQUIRE(arduino::isSpace(c) == true);
                });
}

TEST_CASE ("isSpace(...) is called with a invalid white space character", "[isSpace-02]")
{
  REQUIRE(arduino::isSpace('{') == false);
}
