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

TEST_CASE ("Testing 'map(val, fromLow, fromHigh, toLow, toHigh)' usage with reduction of output", "[map-01]")
{
  REQUIRE(map(50, 0, 100, 0, 10) == 5);
}

TEST_CASE ("Testing 'map(val, fromLow, fromHigh, toLow, toHigh)' usage with increase of output", "[map-02]")
{
  REQUIRE(map(5, 0, 10, 0, 100) == 50);
}

TEST_CASE ("Testing 'map(val, fromLow, fromHigh, toLow, toHigh)' where output is reduced to a value < 0", "[map-03]")
{
  REQUIRE(map(1, 0, 100, 0, 10) == 0); /* Would be 0.1 if we'd be using floating point. */
}

TEST_CASE ("Testing 'map(val, fromLow, fromHigh, toLow, toHigh)' default usage with negative values", "[map-04]")
{
  WHEN ("Negative 'from' values")
  {
    REQUIRE(map(0,    0, -256, 0, 1024) ==    0);
    REQUIRE(map(-256, 0, -256, 0, 1024) == 1024);
    REQUIRE(map(-128, 0, -256, 0, 1024) ==  512);
  }
  WHEN ("Negative 'to' values")
  {
    REQUIRE(map(0,   0, 256, 0, -1024) ==     0);
    REQUIRE(map(256, 0, 256, 0, -1024) == -1024);
    REQUIRE(map(128, 0, 256, 0, -1024) ==  -512);
  }
}
