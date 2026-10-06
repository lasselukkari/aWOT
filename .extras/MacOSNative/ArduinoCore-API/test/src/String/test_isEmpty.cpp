/*
 * Copyright (c) 2023 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/String.h>

#include "StringPrinter.h"

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Testing String::isEmpty when string is empty", "[String-isEmpty-01]")
{
  arduino::String str;
  REQUIRE(str.isEmpty());
}

TEST_CASE ("Testing String::isEmpty when string contains characters", "[String-isEmpty-02]")
{
  arduino::String str("Testing String::isEmpty");
  REQUIRE(!str.isEmpty());
}
