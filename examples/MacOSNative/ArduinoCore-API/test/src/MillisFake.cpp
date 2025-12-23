/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <MillisFake.h>

#include <chrono>

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class MillisFakeMode
{
  Auto, Manual
};

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

static unsigned long millis_val = 0;
static MillisFakeMode millis_fake_mode = MillisFakeMode::Auto;
static std::chrono::steady_clock::time_point const millis_begin = std::chrono::steady_clock::now();

/**************************************************************************************
 * FUNCTION DEFINITION
 **************************************************************************************/

void millis_autoOn()
{
  millis_fake_mode = MillisFakeMode::Auto;
}

void millis_autoOff()
{
  millis_fake_mode = MillisFakeMode::Manual;
}

void set_millis(unsigned long const val)
{
  millis_autoOff();
  millis_val = val;
}

unsigned long millis()
{
  if (millis_fake_mode == MillisFakeMode::Manual)
    return millis_val;

  if (millis_fake_mode == MillisFakeMode::Auto)
  {
    std::chrono::steady_clock::time_point millis_now = std::chrono::steady_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::microseconds>(millis_now - millis_begin).count();
    return static_cast<unsigned long>(millis);
  }

  return 0;
}
