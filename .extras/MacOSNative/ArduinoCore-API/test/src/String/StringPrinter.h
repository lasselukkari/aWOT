/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <catch2/catch_test_macros.hpp>
#include <api/String.h>

namespace Catch {
  /**
   * Template specialization that makes sure Catch can properly print
   * Arduino Strings when used in comparisons directly.
   *
   * Note that without this, String objects are printed as 0 and 1,
   * because they are implicitly convertible to StringIfHelperType,
   * which is a dummy pointer.
   */
  template<>
  struct StringMaker<arduino::String> {
      static std::string convert(const arduino::String& str) {
	if (str)
	  return ::Catch::Detail::stringify(std::string(str.c_str(), str.length()));
	else
	  return "{invalid String}";
      }
  };
} // namespace Catch
