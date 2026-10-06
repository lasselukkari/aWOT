/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/IPAddress.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

// These comparisons should always return false, as you can't compare an IPv6 to an int32_t

TEST_CASE ("Testing implicit cast of IPv6 compatible (little endian) to uint32_t always false", "[IPAddress6-Operator-()-01]")
{
  // On little endian systems, considering only last four octets (ignoring the rest)
  arduino::IPAddress ip(0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 129,168, 1,2);
  uint32_t const val_expected = ip;
  uint32_t const val_actual = (129 | (168 << 8) | (1 << 16) | (2 << 24));
  REQUIRE((val_expected == val_actual) == false);
}

TEST_CASE ("Testing implicit cast of IPv6 full little endian to uint32_t always false", "[IPAddress6-Operator-()-01]")
{
  // On little endian systems (full value)
  arduino::IPAddress ip(129,168, 1,2, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0);
  uint32_t const val_expected = ip;
  uint32_t const val_actual = (129 | (168 << 8) | (1 << 16) | (2 << 24));
  REQUIRE((val_expected == val_actual) == false);
}

TEST_CASE ("Testing implicit cast of IPv6 to uint32_t always false", "[IPAddress6-Operator-()-01]")
{
  // Actual value of the 128-bit IPv6 address, which is network byte order
  arduino::IPAddress ip(0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 129,168, 1,2);
  uint32_t const val_expected = ip;
  uint32_t const val_actual = ((129 << 24) | (168 << 16) | (1 << 8) | 2);
  REQUIRE((val_expected == val_actual) == false);
}
