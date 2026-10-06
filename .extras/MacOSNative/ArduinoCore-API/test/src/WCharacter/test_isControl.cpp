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

std::vector<char> const VALID_CONTROL_VECT = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,'\t','\f','\v','\n','\r',0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x7F};

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("isControl(...) is called with control char", "[isControl-01]")
{
  std::for_each(std::begin(VALID_CONTROL_VECT),
                std::end  (VALID_CONTROL_VECT),
                [](char const c)
                {
                  REQUIRE(arduino::isControl(c) == true);
                });
}

TEST_CASE ("isControl(...) is called with non control char", "[isControl-02]")
{
  REQUIRE(arduino::isControl('z') == false);
}
