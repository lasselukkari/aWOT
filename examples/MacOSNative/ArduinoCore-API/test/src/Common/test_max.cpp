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

TEST_CASE ("Calling 'max(a,b)' with a < b", "[max-01]")
{
  WHEN("a > 0 and b > 0") REQUIRE(max(1,5)   ==  5);
  WHEN("a < 0 and b > 0") REQUIRE(max(-1,5)  ==  5);
  WHEN("a < 0 and b < 0") REQUIRE(max(-5,-1) == -1);
}

TEST_CASE ("Calling 'max(a,b)' with a > b", "[max-02]")
{
  WHEN("a > 0 and b > 0") REQUIRE(max(5,1)   ==  5);
  WHEN("a > 0 and b < 0") REQUIRE(max(5,-1)  ==  5);
  WHEN("a < 0 and b < 0") REQUIRE(max(-1,-5) == -1);
}

TEST_CASE ("Calling 'max(a,b)' with a == b", "[max-03]")
{
  WHEN("a = b > 0") REQUIRE(max(5,5)   ==  5);
  WHEN("a = b < 0") REQUIRE(max(-5,-5) == -5);
  WHEN("a = b = 0") REQUIRE(max(0,0)   ==  0);
}

TEST_CASE ("Calling 'max(a,b)' with type(a) != type(b)", "[max-04]")
{
  WHEN("type(A) = uint8_t, type(b) = uint16_t")
  {
    uint8_t  const a = 32;
    uint16_t const b = 10;
    REQUIRE(typeid(max(a,b)) == typeid(int));
  }
  WHEN("type(A) = uint16_t, type(b) = uint32_t")
  {
    uint16_t const a = 32;
    uint32_t const b = 10;
    REQUIRE(typeid(max(a,b)) == typeid(unsigned int));
  }
  WHEN("type(A) = uint32_t, type(b) = uint64_t")
  {
    uint32_t const a = 32;
    uint64_t const b = 10;
    REQUIRE(typeid(max(a,b)) == typeid(uint64_t));
  }
  WHEN("type(A) = int8_t, type(b) = int16_t")
  {
    int8_t  const a = -32;
    int16_t const b = -10;
    REQUIRE(typeid(max(a,b)) == typeid(int));
  }
  WHEN("type(A) = int16_t, type(b) = int32_t")
  {
    int16_t const a = -32;
    int32_t const b = -10;
    REQUIRE(typeid(max(a,b)) == typeid(int));
  }
  WHEN("type(A) = int32_t, type(b) = int64_t")
  {
    int32_t const a = -32;
    int64_t const b = -10;
    REQUIRE(typeid(max(a,b)) == typeid(int64_t));
  }
}
