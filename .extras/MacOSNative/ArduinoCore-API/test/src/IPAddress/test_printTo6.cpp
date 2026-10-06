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

TEST_CASE ("Print IPv6", "[IPAddress-printTo6-01]")
{
  PrintMock mock;
  arduino::IPAddress ip(0x20,0x01, 0xd,0xb8, 1,2, 3,4, 5,6, 7,8, 9,0xa, 0xb,0xc);

  mock.print(ip);

  REQUIRE(mock._str  == "2001:db8:102:304:506:708:90a:b0c");
}

TEST_CASE ("Print IPv6 any", "[IPAddress-printTo6-02]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

  mock.print(ip);

  REQUIRE(mock._str  == "::");
}

TEST_CASE ("Print IPv6 localhost", "[IPAddress-printTo6-03]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1);

  mock.print(ip);

  REQUIRE(mock._str  == "::1");
}

TEST_CASE ("Print IPv6 different length segments", "[IPAddress-printTo6-04]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0xab,0xcd, 0x0e,0xf1, 0x00,0x23, 0,0, 0x00,0x04, 0,0, 0,0, 0,0);

  mock.print(ip);

  REQUIRE(mock._str  == "abcd:ef1:23:0:4::");
}

TEST_CASE ("Print IPv6 zero longest run end", "[IPAddress-printTo6-05]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,0, 0,1, 0,0, 0,0, 0,2, 0,0, 0,0, 0,0);

  mock.print(ip);

  REQUIRE(mock._str  == "0:1:0:0:2::");
}

TEST_CASE ("Print IPv6 zero longest run mid", "[IPAddress-printTo6-06]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,0, 0,1, 0,0, 0,0, 0,0, 0,2, 0,0, 0,0);

  mock.print(ip);

  REQUIRE(mock._str  == "0:1::2:0:0");
}

TEST_CASE ("Print IPv6 start zero", "[IPAddress-printTo6-07]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,0, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7, 0,8);

  mock.print(ip);

  REQUIRE(mock._str  == "0:2:3:4:5:6:7:8");
}

TEST_CASE ("Print IPv6 ending zero", "[IPAddress-printTo6-08]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7, 0,0);

  mock.print(ip);

  REQUIRE(mock._str  == "1:2:3:4:5:6:7:0");
}

TEST_CASE ("Print IPv6 start two zero", "[IPAddress-printTo6-09]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,0, 0,0, 0,3, 0,4, 0,5, 0,6, 0,7, 0,0);

  mock.print(ip);

  REQUIRE(mock._str  == "::3:4:5:6:7:0");
}

TEST_CASE ("Print IPv6 ending two zero", "[IPAddress-printTo6-10]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,0, 0,2, 0,3, 0,4, 0,5, 0,6, 0,0, 0,0);

  mock.print(ip);

  REQUIRE(mock._str  == "0:2:3:4:5:6::");
}

TEST_CASE ("Print IPv6 first out of same length", "[IPAddress-printTo6-11]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,1, 0,0, 0,0, 0,4, 0,5, 0,0, 0,0, 0,8);

  mock.print(ip);

  REQUIRE(mock._str  == "1::4:5:0:0:8");
}


TEST_CASE ("Print IPv6 single zeros not compressed", "[IPAddress-printTo6-12]")
{
  PrintMock mock;
  arduino::IPAddress const ip(0,1, 0,0, 0,3, 0,0, 0,5, 0,0, 0,7, 0,8);

  mock.print(ip);

  REQUIRE(mock._str  == "1:0:3:0:5:0:7:8");
}
