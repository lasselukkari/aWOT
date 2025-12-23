/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/RingBuffer.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("'available' should return 0 for empty ring buffer", "[Ringbuffer-available-01]")
{
  arduino::RingBufferN<2> ringbuffer;
  REQUIRE(ringbuffer.available() == 0);
}

TEST_CASE ("'available' should return number of elements in ringbuffer", "[Ringbuffer-available-02]")
{
  arduino::RingBufferN<2> ringbuffer;
  ringbuffer.store_char('A');
  REQUIRE(ringbuffer.available() == 1);
  ringbuffer.store_char('B');
  REQUIRE(ringbuffer.available() == 2);
}
