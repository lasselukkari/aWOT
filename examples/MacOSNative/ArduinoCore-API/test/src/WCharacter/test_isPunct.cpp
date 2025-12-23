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

std::vector<char> const VALID_PUNCT_VECT = {'!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',':',';','<','=','>','?','@','[','\\',']','^','_','`','{','|','}','~'};

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("isPunct('.') is called with a valid punct character", "[isPunct-01]")
{
  std::for_each(std::begin(VALID_PUNCT_VECT),
                std::end  (VALID_PUNCT_VECT),
                [](char const c)
                {
                  REQUIRE(arduino::isPunct(c) == true);
                });
}

TEST_CASE ("isPunct(...) is called with a invalid punct character", "[isPunct-02]")
{
  REQUIRE(arduino::isPunct('a') == false);
}
