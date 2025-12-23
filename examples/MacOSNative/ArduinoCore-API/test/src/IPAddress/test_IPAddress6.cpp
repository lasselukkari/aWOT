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

TEST_CASE ("Testing IPAddress(type) constructor()", "[IPAddress6-Ctor-01]")
{
  arduino::IPAddress ip (arduino::IPType::IPv6);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0);
  REQUIRE(ip[1] == 0);
  REQUIRE(ip[2] == 0);
  REQUIRE(ip[3] == 0);
  REQUIRE(ip[4] == 0);
  REQUIRE(ip[5] == 0);
  REQUIRE(ip[6] == 0);
  REQUIRE(ip[7] == 0);
  REQUIRE(ip[8] == 0);
  REQUIRE(ip[9] == 0);
  REQUIRE(ip[10] == 0);
  REQUIRE(ip[11] == 0);
  REQUIRE(ip[12] == 0);
  REQUIRE(ip[13] == 0);
  REQUIRE(ip[14] == 0);
  REQUIRE(ip[15] == 0);
}

TEST_CASE ("Testing IPAddress(o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o) constructor", "[IPAddress-Ctor6-02]")
{
  arduino::IPAddress ip(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0x20);
  REQUIRE(ip[1] == 0x01);
  REQUIRE(ip[2] == 0xd);
  REQUIRE(ip[3] == 0xb8);
  REQUIRE(ip[4] == 1);
  REQUIRE(ip[5] == 2);
  REQUIRE(ip[6] == 3);
  REQUIRE(ip[7] == 4);
  REQUIRE(ip[8] == 5);
  REQUIRE(ip[9] == 6);
  REQUIRE(ip[10] == 7);
  REQUIRE(ip[11] == 8);
  REQUIRE(ip[12] == 9);
  REQUIRE(ip[13] == 0xa);
  REQUIRE(ip[14] == 0xb);
  REQUIRE(ip[15] == 0xc);
}

TEST_CASE ("Testing IPAddress(type, a *) constructor", "[IPAddress6-Ctor-03]")
{
  uint8_t const ip_addr_array[] = {0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc};
  arduino::IPAddress ip(arduino::IPType::IPv6, ip_addr_array);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0x20);
  REQUIRE(ip[1] == 0x01);
  REQUIRE(ip[2] == 0xd);
  REQUIRE(ip[3] == 0xb8);
  REQUIRE(ip[4] == 1);
  REQUIRE(ip[5] == 2);
  REQUIRE(ip[6] == 3);
  REQUIRE(ip[7] == 4);
  REQUIRE(ip[8] == 5);
  REQUIRE(ip[9] == 6);
  REQUIRE(ip[10] == 7);
  REQUIRE(ip[11] == 8);
  REQUIRE(ip[12] == 9);
  REQUIRE(ip[13] == 0xa);
  REQUIRE(ip[14] == 0xb);
  REQUIRE(ip[15] == 0xc);
}
