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

TEST_CASE ("Testing IPAddress::operator == (IPAddress ip1, IPAddress ip2) with ip1 == ip2", "[IPAddress-Operator-==-01]")
{
  arduino::IPAddress ip1(129,168,1,2), ip2(129,168,1,2);
  REQUIRE((ip1 == ip2) == true);
}

TEST_CASE ("Testing IPAddress::operator == (IPAddress ip1, IPAddress ip2) with ip1 != ip2", "[IPAddress-Operator-==-02]")
{
  arduino::IPAddress ip1(129,168,1,2), ip2(10,0,0,1);
  REQUIRE((ip1 == ip2) == false);
}

TEST_CASE ("Testing IPAddress::operator == (IPAddress ip1, uint8_t const * ip2) with ip1 == ip2", "[IPAddress-Operator-==-03]")
{
  arduino::IPAddress ip1(129,168,1,2);
  uint8_t const ip2[] = {129,168,1,2};
  REQUIRE((ip1 == ip2) == true);
}

TEST_CASE ("Testing IPAddress::operator == (IPAddress ip1, uint8_t const * ip2) with ip1 != ip2", "[IPAddress-Operator-==-04]")
{
  arduino::IPAddress ip1(129,168,1,2);
  uint8_t const ip2[] = {10,0,0,1};
  REQUIRE((ip1 == ip2) == false);
}

TEST_CASE ("Testing IPAddress::operator != (IPAddress ip1, uint8_t const * ip2) with ip1 != ip2", "[IPAddress-Operator-==-05]")
{
  arduino::IPAddress ip1(129,168,1,2), ip2(10,0,0,1);
  REQUIRE((ip1 != ip2) == true);
}

TEST_CASE ("Testing IPAddress::operator != (IPAddress ip1, uint8_t const * ip2) with ip1 == ip2", "[IPAddress-Operator-==-05]")
{
  arduino::IPAddress ip1(129,168,1,2), ip2(129,168,1,2);
  REQUIRE((ip1 != ip2) == false);
}
