/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/Print.h>

#include <PrintMock.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Clear write error should set the error code back to 0", "[Print-clearWriteError-01]")
{
  PrintMock mock;
  mock.mock_setWriteError(5);
  mock.clearWriteError();
  REQUIRE(mock.getWriteError() == 0);
}
