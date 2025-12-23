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

TEST_CASE ("Testing two basic constructs the same", "[IPAddress6-Operator-==-01]")
{
  arduino::IPAddress ip1(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc), ip2(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc);
  REQUIRE((ip1 == ip2) == true);
}

TEST_CASE ("Testing two addresses different", "[IPAddress-Operator-==-02]")
{
  arduino::IPAddress ip1(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc), ip2(0xfd,0x12, 0x34,0x56, 0x78,0x9a, 0,1, 0,0, 0,0, 0,0, 0,1);
  REQUIRE((ip1 == ip2) == false);
}

TEST_CASE ("Testing not equals different address is true", "[IPAddress-Operator-==-03]")
{
  arduino::IPAddress ip1(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc), ip2(0xfd,0x12, 0x34,0x56, 0x78,0x9a, 0,1, 0,0, 0,0, 0,0, 0,1);
  REQUIRE((ip1 != ip2) == true);
}

TEST_CASE ("Testing not equals same address is false", "[IPAddress-Operator-==-04]")
{
  arduino::IPAddress ip1(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc), ip2(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc);
  REQUIRE((ip1 != ip2) == false);
}

// IPv4 and IPv6 differ based on type (irrespective of bytes)

TEST_CASE ("Testing IPv4 vs IPv6", "[IPAddress6-Operator-==-05]")
{
  arduino::IPAddress ip1(10, 0, 0, 1), ip2(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc);
  REQUIRE((ip1 == ip2) == false);
}

TEST_CASE ("Testing IPv4 vs IPv6 equivalent IPv4-compatible address (deprecated)", "[IPAddress6-Operator-==-05]")
{
  arduino::IPAddress ip1(10, 0, 0, 1), ip2(0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 10,0, 0,1);
  REQUIRE((ip1 == ip2) == false);
}

TEST_CASE ("Testing IPv4 vs IPv6 localhost", "[IPAddress6-Operator-==-05]")
{
  arduino::IPAddress ip1(127, 0, 0, 1), ip2(0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 10,0, 0,1);
  REQUIRE((ip1 == ip2) == false);
}

TEST_CASE ("Testing IPv4 equivalent compatible address vs IPv6 localhost", "[IPAddress6-Operator-==-05]")
{
  arduino::IPAddress ip1(0, 0, 0, 1), ip2(0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,1);
  REQUIRE((ip1 == ip2) == false);
}

TEST_CASE ("Testing IPv6 never matches as raw byte sequence assumed to be length 4", "[IPAddress6-Operator-==-06]")
{
  arduino::IPAddress ip1(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc);
  uint8_t const ip2[] = {0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc};
  REQUIRE((ip1 == ip2) == false);
}
