/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/CanMsg.h>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

using namespace arduino;

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Verify correct conversion to 11-Bit CAN ID for lowest valid 11-Bit CAN ID", "[CanMsg-CanStandardId-01]")
{
  REQUIRE(CanStandardId(0) == 0U);
}

TEST_CASE ("Verify correct conversion to 11-Bit CAN ID for highest valid 11-Bit CAN ID", "[CanMsg-CanStandardId-02]")
{
  REQUIRE(CanStandardId(0x7FF) == 0x7FF);
}

TEST_CASE ("Verify capping of CAN IDs exceeding 11-Bit CAN ID range", "[CanMsg-CanStandardId-03]")
{
  REQUIRE(CanStandardId(0x800U)      == 0x000U);
  REQUIRE(CanStandardId(0x801U)      == 0x001U);
  REQUIRE(CanStandardId(0x8FFU)      == 0x0FFU);
  REQUIRE(CanStandardId(0x1FFFFFFFU) == 0x7FFU);
  REQUIRE(CanStandardId(0xFFFFFFFFU) == 0x7FFU);
}
