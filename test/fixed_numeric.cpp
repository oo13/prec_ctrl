/*
  Copyright © 2022 OOTA, Masato

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <catch2/catch.hpp>

#include <cstdint>

#include "fixed_numeric.h"

using namespace prec_ctrl;

TEST_CASE( "significand_adder", "[fixed_numeric]" ) {
    SECTION("int16_t") {
        std::int16_t acc = 0;
        SECTION("normal") {
            acc = 100;
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(10.0)) == 110 );
            acc = 100;
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(-10.0)) == 90 );
        }
        SECTION("wraparound") {
            // Strictly, these operations are undefined behaviors.
            acc = 32700;
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(69.0)) == -32767 );
            acc = -32700;
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(-69.0)) == 32767 );
        }
        SECTION("minimum") {
            acc = -32700;
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(-68.0)) == -32768 );
        }
    }
    SECTION("double") {
        double acc = 0;
        std::uint_fast64_t ui = 1;
        SECTION("normal") {
            acc = 100;
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(10.0)) == 110 );
            acc = 100;
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(-10.0)) == 90 );
            acc = ui<<40;
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(69.0)) == (ui<<40) + 69 );
            acc = -(ui<<50);
            REQUIRE( significand_adder( acc, FixedPoint<8, 0>(-69.0)) == -(ui<<50) - 69 );
        }
    }
}

TEST_CASE( "int_adder", "[fixed_numeric]" ) {
    SECTION("normal") {
        REQUIRE( int_adder<12>( 100, FixedPoint<8, 0>(10.0)) == 110 );
        REQUIRE( int_adder<12>( 100, FixedPoint<8, 0>(-10.0)) == 90 );
    }
    SECTION("wraparound") {
        REQUIRE( int_adder<12>( 2000, FixedPoint<8, 0>(50.0)) == -2046 );
        REQUIRE( int_adder<12>( -2000, FixedPoint<8, 0>(-50.0)) == 2046 );
    }
    SECTION("minimum") {
        REQUIRE( int_adder<12>( -2000, FixedPoint<8, 0>(-48.0)) == -2048 );
    }
}

TEST_CASE( "exact_adder", "[fixed_numeric]" ) {
    SECTION("normal") {
        REQUIRE( exact_adder<12>( 100, FixedPoint<8, 0>(10.0)) == 110 );
        REQUIRE( exact_adder<12>( 100, FixedPoint<8, 0>(-10.0)) == 90 );
    }
    SECTION("inexact") {
        REQUIRE_THROWS_AS( exact_adder<12>( 2000, FixedPoint<8, 0>(50.0)), std::range_error );
        REQUIRE_THROWS_AS( exact_adder<12>( -2000, FixedPoint<8, 0>(-50.0)), std::range_error );
    }
}

TEST_CASE( "clamp_adder", "[fixed_numeric]" ) {
    SECTION("normal") {
        REQUIRE( clamp_adder<12>( 100, FixedPoint<8, 0>(10.0)) == 110 );
        REQUIRE( clamp_adder<12>( 100, FixedPoint<8, 0>(-10.0)) == 90 );
    }
    SECTION("clamping") {
        REQUIRE( clamp_adder<12>( 2000, FixedPoint<8, 0>(50.0)) == 2047 );
        REQUIRE( clamp_adder<12>( -2000, FixedPoint<8, 0>(-50.0)) == -2047 );
    }
}
