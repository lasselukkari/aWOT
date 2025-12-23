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
#include <PrintMock.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Print IPAddress via print method", "[IPAddress-printTo-01]")
{
  PrintMock mock;
  arduino::IPAddress const ip(192,168,1,2);

  mock.print(ip);

  REQUIRE(mock._str  == "192.168.1.2");
}
