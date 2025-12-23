/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MILLIS_FAKE_H_
#define MILLIS_FAKE_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <api/Common.h>

/**************************************************************************************
 * FUNCTION DECLARATION
 **************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void millis_autoOn();
void millis_autoOff();
void set_millis(unsigned long const val);

#ifdef __cplusplus
}
#endif

#endif /* MILLIS_FAKE_H_ */