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

TEST_CASE ("'availableForStore' should return ring buffer size for empty ring buffer", "[Ringbuffer-availableForStore-01]")
{
  arduino::RingBufferN<2> ringbuffer;
  REQUIRE(ringbuffer.availableForStore() == 2);
}

TEST_CASE ("'availableForStore' should return number of free elements in ringbuffer", "[Ringbuffer-availableForStore-02]")
{
  arduino::RingBufferN<2> ringbuffer;
  ringbuffer.store_char('A');
  REQUIRE(ringbuffer.availableForStore() == 1);
  ringbuffer.store_char('B');
  REQUIRE(ringbuffer.availableForStore() == 0);
}
