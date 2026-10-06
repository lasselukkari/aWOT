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

TEST_CASE ("Verify correct conversion to 29-Bit CAN ID for lowest valid 29-Bit CAN ID", "[CanMsg-CanExtendedId-01]")
{
  REQUIRE(CanExtendedId(0) == (CanMsg::CAN_EFF_FLAG | 0U));
}

TEST_CASE ("Verify correct conversion to 29-Bit CAN ID for highest valid 29-Bit CAN ID", "[CanMsg-CanExtendedId-02]")
{
  REQUIRE(CanExtendedId(0x1FFFFFFFU) == (CanMsg::CAN_EFF_FLAG | 0x1FFFFFFFU));
}

TEST_CASE ("Verify capping of CAN IDs exceeding 29-Bit CAN ID range", "[CanMsg-CanExtendedId-03]")
{
  REQUIRE(CanExtendedId(0x2FFFFFFFU) == (CanMsg::CAN_EFF_FLAG | 0x0FFFFFFFU));
  REQUIRE(CanExtendedId(0x3FFFFFFFU) == (CanMsg::CAN_EFF_FLAG | 0x1FFFFFFFU));
  REQUIRE(CanExtendedId(0x4FFFFFFFU) == (CanMsg::CAN_EFF_FLAG | 0x0FFFFFFFU));
  REQUIRE(CanExtendedId(0x5FFFFFFFU) == (CanMsg::CAN_EFF_FLAG | 0x1FFFFFFFU));
  /* ... */
  REQUIRE(CanExtendedId(0xFFFFFFFFU) == (CanMsg::CAN_EFF_FLAG | 0x1FFFFFFFU));
}
