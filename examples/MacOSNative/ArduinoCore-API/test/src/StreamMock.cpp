/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <StreamMock.h>

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

void StreamMock::operator << (char const * str)
{
  for (size_t c = 0; c < strlen(str); c++)
    _stream.push_back(str[c]);
}

size_t StreamMock::write(uint8_t ch)
{ 
  _stream.push_back(static_cast<char>(ch));
  return 1;
}

int StreamMock::available()
{
  return _stream.size();
}

int StreamMock::read()
{
  if (available() == 0)
    return -1;

  /* Extract first/oldest element. */
  char const c = _stream.at(0);
  /* Erase first/oldest element. */
  _stream.pop_front();

  return c;
}

int StreamMock::peek()
{
  if (available() == 0)
    return -1;

  return _stream.at(0);
}
