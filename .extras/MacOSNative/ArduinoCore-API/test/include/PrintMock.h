/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PRINT_MOCK_H_
#define PRINT_MOCK_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <string>

#include <api/Print.h>

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class PrintMock : public Print
{
public:
  std::string _str;
  virtual size_t write(uint8_t b) override;
  void mock_setWriteError() { setWriteError(); }
  void mock_setWriteError(int err) { setWriteError(err); }
};

#endif /* PRINT_MOCK_H_ */
