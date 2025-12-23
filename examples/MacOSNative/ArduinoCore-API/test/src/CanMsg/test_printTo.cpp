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
#include <PrintMock.h>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

using namespace arduino;

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Print CAN frame with standard ID", "[CanMsg-printTo-1]")
{
  uint8_t const std_msg_data[] = {0xBE, 0xEF};
  CanMsg const std_msg(CanStandardId(0x20), sizeof(std_msg_data), std_msg_data);

  PrintMock mock;
  mock.print(std_msg);

  REQUIRE(mock._str  == "[020] (2) : BEEF");
}

TEST_CASE ("Print CAN frame with extended ID", "[CanMsg-printTo-2]")
{
  uint8_t const ext_msg_data[] = {0xDE, 0xAD, 0xC0, 0xDE};
  CanMsg const ext_msg(CanExtendedId(0x20), sizeof(ext_msg_data), ext_msg_data);

  PrintMock mock;
  mock.print(ext_msg);

  REQUIRE(mock._str  == "[00000020] (4) : DEADC0DE");
}
