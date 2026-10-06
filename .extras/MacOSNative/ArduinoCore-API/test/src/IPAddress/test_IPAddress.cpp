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

TEST_CASE ("Testing IPAddress() default constructor()", "[IPAddress-Ctor-01]")
{
  arduino::IPAddress ip;
  REQUIRE(ip[0] == 0);
  REQUIRE(ip[1] == 0);
  REQUIRE(ip[2] == 0);
  REQUIRE(ip[3] == 0);
}

TEST_CASE ("Testing IPAddress(o,o,o,o) constructor", "[IPAddress-Ctor-02]")
{
  arduino::IPAddress ip(129,168,1,2);
  REQUIRE(ip[0] == 129);
  REQUIRE(ip[1] == 168);
  REQUIRE(ip[2] == 1);
  REQUIRE(ip[3] == 2);
}

TEST_CASE ("Testing IPAddress(a) constructor", "[IPAddress-Ctor-03]")
{
  arduino::IPAddress ip(129 | (168 << 8) | (1 << 16) | (2 << 24));
  REQUIRE(ip[0] == 129);
  REQUIRE(ip[1] == 168);
  REQUIRE(ip[2] == 1);
  REQUIRE(ip[3] == 2);
}

TEST_CASE ("Testing IPAddress(a *) constructor", "[IPAddress-Ctor-04]")
{
  uint8_t const ip_addr_array[] = {129,168,1,2};

  arduino::IPAddress ip(ip_addr_array);
  REQUIRE(ip[0] == 129);
  REQUIRE(ip[1] == 168);
  REQUIRE(ip[2] == 1);
  REQUIRE(ip[3] == 2);
}
