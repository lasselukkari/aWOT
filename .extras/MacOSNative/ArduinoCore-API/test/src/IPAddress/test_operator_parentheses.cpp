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

TEST_CASE ("Testing IPAddress::operator uint32_t() const", "[IPAddress-Operator-()-01]")
{
  arduino::IPAddress ip(129,168,1,2);
  uint32_t const val_expected = ip;
  uint32_t const val_actual = (129 | (168 << 8) | (1 << 16) | (2 << 24));
  // NOTE: Only correct on little-endian systems
  REQUIRE(val_expected == val_actual);
}
