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

std::vector<char> const VALID_ASCII_VECT = {' ', 'a', 'b', 'q', '\n', '\r'};

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("isAscii(...) is called with a valid ascii character", "[isAscii-01]")
{
  std::for_each(std::begin(VALID_ASCII_VECT),
                std::end  (VALID_ASCII_VECT),
                [](char const c)
                {
                  REQUIRE(arduino::isAscii(c) == true);
                });
}

TEST_CASE ("isAscii(...) is called with a invalid ascii character", "[isAscii-02]")
{
  REQUIRE(arduino::isAscii(0xf7) == false);
}

TEST_CASE ("isAscii(...) is called with a invalid casted ascii character", "[isAscii-03]")
{
  REQUIRE(arduino::isAscii((unsigned char)0xf7) == false);
}

TEST_CASE ("isAscii(...) is called with a character latger than 1 byte", "[isAscii-04]")
{
  REQUIRE(arduino::isAscii(0x3030) == false);
}

