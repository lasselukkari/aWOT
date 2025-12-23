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

TEST_CASE ("Extract valid IPv6 address 'fromString(const char *)'", "[IPAddress-fromString-01]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("2001:db8:102:304:506:708:90a:b0c") == true);

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

TEST_CASE ("Extract valid IPv6 address 'fromString(const String &)'", "[IPAddress-fromString-02]")
{
  arduino::IPAddress ip;

  arduino::String const ip_addr_str("2001:db8:102:304:506:708:90a:b0c");

  REQUIRE(ip.fromString(ip_addr_str) == true);

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

TEST_CASE ("Extract valid IPv6 any address", "[IPAddress-fromString-03]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("::") == true);

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

TEST_CASE ("Extract valid IPv6 localhost address", "[IPAddress-fromString-04]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("::1") == true);

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
  REQUIRE(ip[15] == 1);
}

TEST_CASE ("Extract valid IPv6 different length segments", "[IPAddress-fromString-05]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("abcd:ef1:23:0:4::") == true);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0xab);
  REQUIRE(ip[1] == 0xcd);
  REQUIRE(ip[2] == 0xe);
  REQUIRE(ip[3] == 0xf1);
  REQUIRE(ip[4] == 0);
  REQUIRE(ip[5] == 0x23);
  REQUIRE(ip[6] == 0);
  REQUIRE(ip[7] == 0);
  REQUIRE(ip[8] == 0);
  REQUIRE(ip[9] == 4);
  REQUIRE(ip[10] == 0);
  REQUIRE(ip[11] == 0);
  REQUIRE(ip[12] == 0);
  REQUIRE(ip[13] == 0);
  REQUIRE(ip[14] == 0);
  REQUIRE(ip[15] == 0);
}


TEST_CASE ("Extract valid IPv6 zero start", "[IPAddress-fromString-06]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("0:2:3:4:5:6:7:8") == true);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0);
  REQUIRE(ip[1] == 0);
  REQUIRE(ip[2] == 0);
  REQUIRE(ip[3] == 2);
  REQUIRE(ip[4] == 0);
  REQUIRE(ip[5] == 3);
  REQUIRE(ip[6] == 0);
  REQUIRE(ip[7] == 4);
  REQUIRE(ip[8] == 0);
  REQUIRE(ip[9] == 5);
  REQUIRE(ip[10] == 0);
  REQUIRE(ip[11] == 6);
  REQUIRE(ip[12] == 0);
  REQUIRE(ip[13] == 7);
  REQUIRE(ip[14] == 0);
  REQUIRE(ip[15] == 8);
}


TEST_CASE ("Extract valid IPv6 zero end", "[IPAddress-fromString-07]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("1:2:3:4:5:6:7:0") == true);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0);
  REQUIRE(ip[1] == 1);
  REQUIRE(ip[2] == 0);
  REQUIRE(ip[3] == 2);
  REQUIRE(ip[4] == 0);
  REQUIRE(ip[5] == 3);
  REQUIRE(ip[6] == 0);
  REQUIRE(ip[7] == 4);
  REQUIRE(ip[8] == 0);
  REQUIRE(ip[9] == 5);
  REQUIRE(ip[10] == 0);
  REQUIRE(ip[11] == 6);
  REQUIRE(ip[12] == 0);
  REQUIRE(ip[13] == 7);
  REQUIRE(ip[14] == 0);
  REQUIRE(ip[15] == 0);
}


TEST_CASE ("Extract valid IPv6 two zero start", "[IPAddress-fromString-08]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("::3:4:5:6:7:0") == true);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0);
  REQUIRE(ip[1] == 0);
  REQUIRE(ip[2] == 0);
  REQUIRE(ip[3] == 0);
  REQUIRE(ip[4] == 0);
  REQUIRE(ip[5] == 3);
  REQUIRE(ip[6] == 0);
  REQUIRE(ip[7] == 4);
  REQUIRE(ip[8] == 0);
  REQUIRE(ip[9] == 5);
  REQUIRE(ip[10] == 0);
  REQUIRE(ip[11] == 6);
  REQUIRE(ip[12] == 0);
  REQUIRE(ip[13] == 7);
  REQUIRE(ip[14] == 0);
  REQUIRE(ip[15] == 0);
}


TEST_CASE ("Extract valid IPv6 two zero end", "[IPAddress-fromString-9]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("0:2:3:4:5:6::") == true);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0);
  REQUIRE(ip[1] == 0);
  REQUIRE(ip[2] == 0);
  REQUIRE(ip[3] == 2);
  REQUIRE(ip[4] == 0);
  REQUIRE(ip[5] == 3);
  REQUIRE(ip[6] == 0);
  REQUIRE(ip[7] == 4);
  REQUIRE(ip[8] == 0);
  REQUIRE(ip[9] == 5);
  REQUIRE(ip[10] == 0);
  REQUIRE(ip[11] == 6);
  REQUIRE(ip[12] == 0);
  REQUIRE(ip[13] == 0);
  REQUIRE(ip[14] == 0);
  REQUIRE(ip[15] == 0);
}

// Non-canonical

TEST_CASE ("Extract valid IPv6 any full long form", "[IPAddress-fromString-10]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("0:0:0:0:0:0:0:0") == true);

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

TEST_CASE ("Extract valid IPv6 upper case", "[IPAddress-fromString-11]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("2001:DB8:102:304:506:708:90A:B0C") == true);

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

TEST_CASE ("Extract valid IPv6 explicit start zero", "[IPAddress-fromString-10]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("0::") == true);

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

TEST_CASE ("Extract valid IPv6 explicit end zero", "[IPAddress-fromString-10]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("::0") == true);

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

TEST_CASE ("Extract valid IPv6 compression of one group of zero", "[IPAddress-fromString-10]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString("1::3:4:5:6:7:8") == true);

  REQUIRE(ip.type() == arduino::IPType::IPv6);
  REQUIRE(ip[0] == 0);
  REQUIRE(ip[1] == 1);
  REQUIRE(ip[2] == 0);
  REQUIRE(ip[3] == 0);
  REQUIRE(ip[4] == 0);
  REQUIRE(ip[5] == 3);
  REQUIRE(ip[6] == 0);
  REQUIRE(ip[7] == 4);
  REQUIRE(ip[8] == 0);
  REQUIRE(ip[9] == 5);
  REQUIRE(ip[10] == 0);
  REQUIRE(ip[11] == 6);
  REQUIRE(ip[12] == 0);
  REQUIRE(ip[13] == 7);
  REQUIRE(ip[14] == 0);
  REQUIRE(ip[15] == 8);
}

// Invalid cases

TEST_CASE ("Extract invalid IPv6 address", "[IPAddress-fromString-12]")
{
  arduino::IPAddress ip;

  REQUIRE(ip.fromString(":::") == false); // three colons by self
  REQUIRE(ip.fromString("::3:4:5:6::") == false); // two compressions
  REQUIRE(ip.fromString("2001:db8:102:10304:506:708:90a:b0c") == false); // 5 character field
  REQUIRE(ip.fromString("200x:db8:102:304:506:708:90a:b0c") == false); // invalid character
  REQUIRE(ip.fromString("2001:db8:102:304::506:708:90a:b0c") == false); // double colon with 8 other fields (so not a compression)
  REQUIRE(ip.fromString("2001:db8:102:304:::708:90a:b0c") == false); // three colons in middle
  REQUIRE(ip.fromString("2001:db8:102:304:506:708:90a:b0c:d0e") == false); // 9 fields
  REQUIRE(ip.fromString("2001:db8:102:304:506:708:90a:") == false); // missing last group (but has a colon)
  REQUIRE(ip.fromString("2001:db8:102:304:506:708:90a") == false); // only seven groups
  REQUIRE(ip.fromString("0:0:0:0:0:0:0:0:0") == false); // nine zeros
  REQUIRE(ip.fromString("0:0:0:0:0:0:0:0:") == false); // extra colon
  REQUIRE(ip.fromString("0:0:0:0:0:0:0:") == false); // missing last group (but has a colon)
  REQUIRE(ip.fromString("0:0:0:0:0:0:0") == false); // only seven groups
}
