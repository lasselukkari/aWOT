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

TEST_CASE ("toAscii(...) is called with a valid ascii character", "[toAscii-01]")
{
  std::for_each(std::begin(VALID_ASCII_VECT),
                std::end  (VALID_ASCII_VECT),
                [](char const c)
                {
                  REQUIRE(arduino::toAscii(c) == c);
                });
}

TEST_CASE ("toAscii(...) is called with a invalid ascii character", "[toAscii-02]")
{
  REQUIRE(arduino::toAscii(0xf7) == 0x77);
}

TEST_CASE ("toAscii(...) is called with a invalid casted ascii character", "[toAscii-03]")
{
  REQUIRE(arduino::toAscii((unsigned char)0xf7) == 0x77);
}

TEST_CASE ("toAscii(...) is called with a character larger than 1 byte", "[toAscii-04]")
{
  REQUIRE(arduino::toAscii(0x3030) == 0x30);
}
