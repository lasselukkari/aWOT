/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/Common.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Calling 'makeWord(w)'", "[makeWord-01]")
{
  REQUIRE(makeWord(0xDEAD) == 0xDEAD);
  REQUIRE(makeWord(0xDE)   == 0x00DE);
}

TEST_CASE ("Calling 'makeWord(h,l)'", "[makeWord-02]")
{
  REQUIRE(makeWord(0xDE, 0xAD) == 0xDEAD);
}

TEST_CASE ("Calling 'word(...)'", "[makeWord-03]")
{
  REQUIRE(word(0xDEAD)     == 0xDEAD);
  REQUIRE(word(0xDE, 0xAD) == 0xDEAD);
}
