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

TEST_CASE ("Extract valid IP address 'fromString(const char *)'", "[IPAddress-fromString-01]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("129.168.1.2") == true);

  REQUIRE(ip[0] == 129);
  REQUIRE(ip[1] == 168);
  REQUIRE(ip[2] == 1);
  REQUIRE(ip[3] == 2);
}

TEST_CASE ("Extract valid IP address 'fromString(const String &)'", "[IPAddress-fromString-02]")
{
  arduino::IPAddress ip;

  arduino::String const ip_addr_str("129.168.1.2");

  REQUIRE(ip.fromString(ip_addr_str) == true);

  REQUIRE(ip[0] == 129);
  REQUIRE(ip[1] == 168);
  REQUIRE(ip[2] == 1);
  REQUIRE(ip[3] == 2);
}

TEST_CASE ("Extract invalid IP address 'fromString(const char *)'", "[IPAddress-fromString-03]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("")          == false);
  REQUIRE(ip.fromString("1")         == false);
  REQUIRE(ip.fromString("1.")        == false);
  REQUIRE(ip.fromString("1.1")       == false);
  REQUIRE(ip.fromString("1.1.")      == false);
  REQUIRE(ip.fromString("1.1.1")     == false);
  REQUIRE(ip.fromString("1.1.1.")    == false);
  REQUIRE(ip.fromString("...")       == false);
  REQUIRE(ip.fromString("256.1.1.1") == false);
  REQUIRE(ip.fromString("a.1.1.1")   == false);
  REQUIRE(ip.fromString("-.1.1.1")   == false);
  REQUIRE(ip.fromString("-1.1.1.1")  == false);
}
