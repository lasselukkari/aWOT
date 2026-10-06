/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <catch2/catch_test_macros.hpp>

#include <api/String.h>

#include "StringPrinter.h"

#include <utility>

TEST_CASE("Testing String move constructor", "[String-move-01]")
{
    arduino::String a("src");
    char const* const a_str = a.c_str();
    arduino::String b(std::move(a));
    REQUIRE(a.length() == 0);
    REQUIRE(a.c_str() == nullptr);
    REQUIRE(b.c_str() == a_str);
    REQUIRE(b.length() == 3);
}

TEST_CASE("Testing String move assignment", "[String-move-02]")
{
    arduino::String a("src");
    char const* const a_str = a.c_str();
    arduino::String b;
    b = std::move(a);
    REQUIRE(a.length() == 0);
    REQUIRE(a.c_str() == nullptr);
    REQUIRE(b == arduino::String("src"));
    REQUIRE(b.c_str() == a_str);
}

TEST_CASE("Testing String move self assignment", "[String-move-03]")
{
#if (defined(GCC_VERSION) && GCC_VERSION >= 13) || (defined(__clang_major__) && __clang_major__ >= 14) \
    || (defined(__GNUC__) && __GNUC__ >= 13)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif
    arduino::String a("src");
    a = std::move(a);
    REQUIRE(a == "src");
#if defined(GCC_VERSION) && GCC_VERSION >= 13 || (defined(__clang_major__) && __clang_major__ >= 14) \
    || (defined(__GNUC__) && __GNUC__ >= 13)
#pragma GCC diagnostic pop
#endif
}
