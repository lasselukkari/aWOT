/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PRINTABLE_MOCK_H_
#define PRINTABLE_MOCK_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <string>

#include <api/Printable.h>

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class PrintableMock : public arduino::Printable
{
public:
  int _i;
  virtual size_t printTo(arduino::Print& p) const override
  {
    size_t written = 0;
    written += p.print("PrintableMock i = ");
    written += p.print(_i);
    return written;
  }
};

#endif /* PRINTABLE_MOCK_H_ */
