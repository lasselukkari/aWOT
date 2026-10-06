/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

using namespace Catch;

#include <StreamMock.h>

#include <float.h>

using namespace arduino;

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Testing parseFloat(LookaheadMode lookahead = SKIP_ALL, char ignore = NO_IGNORE_CHAR)", "[Stream-parseFloat-01]")
{
  StreamMock mock;

  WHEN ("Only a integer (no comma) is contained in stream")
  {
    mock << "12";
    REQUIRE(mock.parseFloat() == 12.0f);
  }
  WHEN ("A positive float is contained in stream")
  {
    mock << "12.34";
    REQUIRE(mock.parseFloat() == 12.34f);
  }
  WHEN ("A negative float is contained in stream")
  {
    mock << "-12.34";
    REQUIRE(mock.parseFloat() == -12.34f);
  }
  WHEN ("A float is prepended by digits")
  {
    mock << "abcdef12.34";
    REQUIRE(mock.parseFloat() == 12.34f);
  }
  WHEN ("The integer is prepended by whitespace chars")
  {
    mock << "\r\n\t 12.34";
    REQUIRE(mock.parseFloat() == 12.34f);
  }
  WHEN ("A float is provided with too many digits after the decimal point")
  {
    mock << "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870064";
    REQUIRE(mock.parseFloat() == Approx(3.141592654f));
  }
  WHEN ("A float is larger than LONG_MAX")
  {
    mock << "602200000000000000000000.00";
    REQUIRE(mock.parseFloat() == Approx(6.022e23f));
  }
}

TEST_CASE ("Testing parseFloat(LookaheadMode lookahead = SKIP_NONE, char ignore = NO_IGNORE_CHAR)", "[Stream-parseFloat-02]")
{
  StreamMock mock;

  WHEN ("Only a integer is contained in stream")
  {
    mock << "12.34";
    REQUIRE(mock.parseFloat(SKIP_NONE) == 12.34f);
    REQUIRE(mock.readString() == arduino::String(""));
  }
  WHEN ("The integer is prepended by digits")
  {
    mock << "abcdef12.34";
    REQUIRE(mock.parseFloat(SKIP_NONE) == 0);
    REQUIRE(mock.readString() == arduino::String("abcdef12.34"));
  }
  WHEN ("The integer is prepended by whitespace chars")
  {
    mock << "\r\n\t 12.34";
    REQUIRE(mock.parseFloat(SKIP_NONE) == 0);
    REQUIRE(mock.readString() == arduino::String("\r\n\t 12.34"));
  }
}

TEST_CASE ("Testing parseFloat(LookaheadMode lookahead = SKIP_WHITESPACE, char ignore = NO_IGNORE_CHAR)", "[Stream-parseFloat-03]")
{
  StreamMock mock;

  WHEN ("The integer is prepended by whitespace chars")
  {
    mock << "\r\n\t 12.34";
    REQUIRE(mock.parseFloat(SKIP_WHITESPACE) == 12.34f);
    REQUIRE(mock.readString() == arduino::String(""));
  }
}


TEST_CASE ("Testing parseFloat(LookaheadMode lookahead = SKIP_ALL, char ignore = 'a')", "[Stream-parseFloat-04]")
{
  StreamMock mock;

  WHEN ("A float is contained in stream")
  {
    mock << "12.34";
    REQUIRE(mock.parseFloat(SKIP_ALL, 'a') == 12.34f);
    REQUIRE(mock.readString() == arduino::String(""));
  }
  WHEN ("The float contains only ignore char values")
  {
    mock << "12a.3a4a";
    REQUIRE(mock.parseFloat(SKIP_ALL, 'a') == 12.34f);
    REQUIRE(mock.readString() == arduino::String(""));
  }
  WHEN ("The integer contains other than ignore chars")
  {
    mock << "1bed234";
    REQUIRE(mock.parseFloat(SKIP_ALL, 'a') == 1.0f);
    REQUIRE(mock.readString() == arduino::String("bed234"));
  }
}
