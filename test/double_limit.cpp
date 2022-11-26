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

#include <cfenv>
#include <sstream>
#include <iomanip>

#include "double_limit.h"

using namespace prec_ctrl;

TEST_CASE( "Rounding & Capability to switch the rounding methods", "[double_limit]" ) {
    SECTION("FE_TONEAREST") {
        std::fesetround(FE_TONEAREST);

        REQUIRE( limit_precision(0.49, 50, 0) == 0.0 );
        REQUIRE( limit_precision(1.50, 50, 0) == 2.0 );
        REQUIRE( limit_precision(7.00, 50, 1) == 8.0 );
        REQUIRE( limit_precision(9.70, 50, -1) == 9.5 );
        REQUIRE( limit_precision(-0.49, 50, 0) == 0.0 );
        REQUIRE( limit_precision(-1.50, 50, 0) == -2.0 );
        REQUIRE( limit_precision(-7.00, 50, 1) == -8.0 );
        REQUIRE( limit_precision(-9.70, 50, -1) == -9.5 );
    }
    SECTION("FE_DOWNWARD") {
        std::fesetround(FE_DOWNWARD);

        REQUIRE( limit_precision(0.49, 50, 0) == 0.0 );
        REQUIRE( limit_precision(1.50, 50, 0) == 1.0 );
        REQUIRE( limit_precision(7.00, 50, 1) == 6.0 );
        REQUIRE( limit_precision(9.70, 50, -1) == 9.5 );
        REQUIRE( limit_precision(-0.49, 50, 0) == -1.0 );
        REQUIRE( limit_precision(-1.50, 50, 0) == -2.0 );
        REQUIRE( limit_precision(-7.00, 50, 1) == -8.0 );
        REQUIRE( limit_precision(-9.70, 50, -1) == -10.0 );
    }
}

TEST_CASE( "Clamping", "[double_limit]" ) {
    REQUIRE( limit_precision(1000, 8, 0) == 127.0 );
    REQUIRE( limit_precision(1000, 9, 0) == 255.0 );
    REQUIRE( limit_precision(1000, 10, 0) == 511.0 );
    REQUIRE( limit_precision(-1000, 8, 0) == -127.0 );
    REQUIRE( limit_precision(-1000, 9, 0) == -255.0 );
    REQUIRE( limit_precision(-1000, 10, 0) == -511.0 );

    std::ostringstream buf;
    buf << std::hexfloat;
    buf << limit_precision(1000, 54, -50) << std::flush;
    REQUIRE( buf.str() == "0x1.fffffffffffffp+2" );
    buf.str("");
    buf << limit_precision(-1000, 54, -55) << std::flush;
    REQUIRE( buf.str() == "-0x1.fffffffffffffp-3" );
    buf.str("");
    buf << limit_precision(1E+100, 54, 50) << std::flush;
    REQUIRE( buf.str() == "0x1.fffffffffffffp+102" );
    buf.str("");
    buf << limit_precision(-1E+100, 54, 55) << std::flush;
    REQUIRE( buf.str() == "-0x1.fffffffffffffp+107" );
}

TEST_CASE( "Round and then Clamp", "[double_limit]" ) {
    std::fesetround(FE_TONEAREST);

    REQUIRE( limit_precision(7.96876, 8, -4) == 7.9375 );
    REQUIRE( limit_precision(-7.96876, 8, -4) == -7.9375 );
}
