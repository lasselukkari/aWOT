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

TEST_CASE ("Data is accessed but not removed from the ring buffer via 'peek'", "[Ringbuffer-peek-01]")
{
  arduino::RingBufferN<2> ringbuffer;

  WHEN("The ringbuffer is empty")
    THEN("'peek' should return -1")
      REQUIRE(ringbuffer.peek() == -1);

  WHEN("The ringbuffer contains data")
  {
    ringbuffer.store_char('A');
    ringbuffer.store_char('B');
    THEN("'peek' should return first inserted element first (FIFO) and not remove it")
    {
      REQUIRE(ringbuffer.peek() == 'A');
      REQUIRE(ringbuffer.peek() == 'A');
    }
  }
}
