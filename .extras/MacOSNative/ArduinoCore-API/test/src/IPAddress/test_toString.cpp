/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/String.h>
#include <api/IPAddress.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Extract valid string from IPv4address", "[IPAddress-toString-01]")
{
  arduino::IPAddress ip(129,168,1,2);

  REQUIRE(ip.toString().equals("129.168.1.2") == true);
}

TEST_CASE ("Extract valid ipv6 string from IPv6address", "[IPAddress-toString-02]")
{
  arduino::IPAddress ip(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc);

  REQUIRE(ip.toString().equals("2001:0db8:0102:0304:0506:0708:090a:0b0c") == true);
}

TEST_CASE ("Extract 0.0.0.0 string from uninitialized IP address", "[IPAddress-toString-03]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.toString().equals("0.0.0.0") == true);
}
