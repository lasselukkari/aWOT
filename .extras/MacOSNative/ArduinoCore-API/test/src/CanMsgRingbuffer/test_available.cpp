/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <api/CanMsgRingbuffer.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("'available' should return 0 for empty CanMsg ring buffer", "[CanMsgRingbuffer-available-01]")
{
  arduino::CanMsgRingbuffer ringbuffer;
  REQUIRE(ringbuffer.available() == 0);
}

TEST_CASE ("'available' should return number of elements in CanMsg ringbuffer", "[CanMsgRingbuffer-available-02]")
{
  arduino::CanMsgRingbuffer ringbuffer;
  arduino::CanMsg msg;
  ringbuffer.enqueue(msg);
  REQUIRE(ringbuffer.available() == 1);
  ringbuffer.enqueue(msg);
  REQUIRE(ringbuffer.available() == 2);
  ringbuffer.dequeue();
  REQUIRE(ringbuffer.available() == 1);
}
