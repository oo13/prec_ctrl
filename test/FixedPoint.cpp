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

#include <cmath>
#include <type_traits>

#include "double_limit.h"
#include "FixedPoint.h"

using namespace prec_ctrl;

TEST_CASE( "Default constructor", "[FixedPoint]" ) {
    REQUIRE( FixedPoint<8, 0>() == 0.0 );
}

TEST_CASE( "Copy constructor", "[FixedPoint]" ) {
    FixedPoint<8, -4> a(3.1234);
    REQUIRE( FixedPoint<8, -4>(a) == a );
    REQUIRE( FixedPoint<8, -4>(a) == static_cast<double>(a) );
}

TEST_CASE( "Assign operator", "[FixedPoint]" ) {
    FixedPoint<8, -4> a(3.1234);
    FixedPoint<8, -4> b;
    b = a;
    REQUIRE( b == a );
    REQUIRE( static_cast<double>(b) == a );
}

TEST_CASE( "Copy from narrower", "[FixedPoint]" ) {
    FixedPoint<8, -4> a(6.9876);
    // To widen toward LSB
    REQUIRE( FixedPoint<9, -5>(a) == a );
    // To widen toward MSB
    REQUIRE( FixedPoint<9, -4>(a) == static_cast<double>(a) );
    // To widen both sides
    REQUIRE( FixedPoint<20, -8>(a) == a );
}

TEST_CASE( "Assign from narrower", "[FixedPoint]" ) {
    FixedPoint<8, -4> a(6.9876);
    // To widen toward LSB
    FixedPoint<10, -6> b;
    b = a;
    REQUIRE( b == a );
    REQUIRE( static_cast<double>(b) == a );
    // To widen toward MSB
    FixedPoint<10, -4> c;
    c = a;
    REQUIRE( c == a );
    REQUIRE( static_cast<double>(c) == a );
    // To widen both sides
    FixedPoint<20, -8> d;
    d = a;
    REQUIRE( d == a );
    REQUIRE( static_cast<double>(d) == a );
}

TEST_CASE( "Reduce dynamic range", "[FixedPoint]" ) {
    FixedPoint<12, -4> a(123.9876);
    REQUIRE( a.reduce_dynamic_range<8>() == 4 + 2 + 1 + 1.0/2 + 1.0/4 + 1.0/8 + 1.0/16 );
    REQUIRE( a.reduce_dynamic_range<10>() == FixedPoint<10, -4>(123.9876) );
    REQUIRE( a.reduce_dynamic_range<6>() == limit_precision(a, 6, -4) );
    FixedPoint<33, 0> b;
    b.set_significand(0x100000001LL);
    REQUIRE( b.reduce_dynamic_range<16>() == 0x7FFF );
}

TEST_CASE( "Copy from double", "[FixedPoint]" ) {
    double a = 56789.123456;
    // some value
    REQUIRE( FixedPoint<33, -16>(a) == limit_precision(a, 33, -16) );
    REQUIRE( FixedPoint<16, 4>(a) == limit_precision(a, 16, 4) );
    // maximum value
    REQUIRE( FixedPoint<8, -10>(a) == limit_precision(a, 8, -10) );
    // minimum value
    REQUIRE( FixedPoint<8, 0>(-a) == -127.0 );
    // clamping
    double b = 0x10001000L;
    b *= std::exp2(36);
    REQUIRE( FixedPoint<54, 0>(b) == 0x001FFFFFFFFFFFFFLL );
}

TEST_CASE( "Implicit conversion to double", "[FixedPoint]" ) {
    SECTION( "Possibility of double operation" ) {
        REQUIRE( FixedPoint<8, -4>(1.23) + 2.34 ==  limit_precision(1.23, 8, -4) + 2.34 );
        REQUIRE( floor(FixedPoint<6, -2>(4.56)) == 4.0 );
        REQUIRE( FixedPoint<8, 1>(13.1) / 7.0 == 2.0 );
    }
    SECTION( "Type of expression" ) {
        REQUIRE( std::is_same<double, decltype(FixedPoint<8, -4>(1.23) + 2.34)>::value );
        REQUIRE( std::is_same<double, decltype(FixedPoint<8, 1>(13.1) / 7.0)>::value );
    }
}

TEST_CASE( "Explicit conversion to bool", "[FixedPoint]" ) {
    REQUIRE( static_cast<bool>(FixedPoint<8, -4>(1.23)) );
    REQUIRE( static_cast<bool>(FixedPoint<8, -4>(0.00001)) == false );
}

TEST_CASE( "Unary operator +", "[FixedPoint]" ) {
    // some value
    REQUIRE( +FixedPoint<22, -10>(599.7) == FixedPoint<22, -10>(599.7) );
    REQUIRE( +FixedPoint<22, -10>(100000.0) == FixedPoint<22, -10>(100000.0) );
    // maximum value
    REQUIRE( +FixedPoint<8, -1>(100.0) == 63.5 );
    REQUIRE( +FixedPoint<8, -1>(100.0).get_significand() == 0x7F );
    // minimum value
    REQUIRE( +FixedPoint<4, 1>(-100.0) == -14 );
    REQUIRE( +FixedPoint<4, 1>(-100.0).get_significand() == -7 );
}

TEST_CASE( "Unary operator -", "[FixedPoint]" ) {
    // some value
    REQUIRE( -FixedPoint<22, -10>(599.7) == FixedPoint<22, -10>(-599.7) );
    REQUIRE( -FixedPoint<22, -10>(100000.0) == FixedPoint<22, -10>(-100000.0) );
    // maximum value
    REQUIRE( -FixedPoint<7, 1>(-128.0) == 126 );
    REQUIRE( -FixedPoint<7, 1>(-128.0).get_significand() == 0x3F );
    // minimum value
    REQUIRE( -FixedPoint<8, -1>(64.0) == -63.5 );
    REQUIRE( -FixedPoint<8, -1>(64.0).get_significand() == -0x7F );
}

TEST_CASE( "Unary operator !", "[FixedPoint]" ) {
    REQUIRE( !FixedPoint<8, -4>(1.23) == false);
    REQUIRE( !FixedPoint<8, -4>(0.00001) );
}

TEST_CASE( "Binary operator +", "[FixedPoint]" ) {
    SECTION( "Value" ) {
        // some value
        REQUIRE( FixedPoint<8, -4>(1.23) + FixedPoint<7, -5>(0.23) == limit_precision(1.23, 8, -4) + limit_precision(0.23, 7, -5) );
        // maximum value
        REQUIRE( FixedPoint<8, 1>(1000.0) + FixedPoint<8, 1>(1000.0) ==  508 );
        // minimum value
        REQUIRE( FixedPoint<8, -1>(-1000.0) + FixedPoint<8, -1>(-1000.0) ==  -127 );
        // precision without overlap
        REQUIRE( FixedPoint<4, -4>(1000.0) + FixedPoint<7, 1>(1000.0) ==  1.0/4 + 1.0/8 + 1.0/16 + 126);
        REQUIRE( FixedPoint<4, -4>(-1000.0) + FixedPoint<7, 1>(-1000.0) ==  -(1.0/4 + 1.0/8 + 1.0/16 + 126) );
        // precision overlapped only sign bit
        REQUIRE( FixedPoint<4, -4>(1000.0) + FixedPoint<7, -1>(1000.0) ==  1.0/4 + 1.0/8 + 1.0/16 + 31.5);
        REQUIRE( FixedPoint<4, -4>(-1000.0) + FixedPoint<7, -1>(-1000.0) ==  -(1.0/4 + 1.0/8 + 1.0/16 + 31.5) );
    }
    SECTION( "Precision" ) {
        // same width and place
        auto a = FixedPoint<8, -5>(1.23) + FixedPoint<8, -5>(0.23);
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == -5 );
        // same place
        auto b = FixedPoint<9, -5>(1.23) + FixedPoint<7, -5>(0.23);
        REQUIRE( b.width == 10 );
        REQUIRE( b.place == -5 );
        // wider + narrower
        auto c1 = FixedPoint<10, -8>(1.23) + FixedPoint<5, -4>(0.23);
        REQUIRE( c1.width == 11 );
        REQUIRE( c1.place == -8 );
        // narrower + wider
        auto c2 = FixedPoint<5, -4>(0.23) + FixedPoint<10, -8>(1.23);
        REQUIRE( c2.width == 11 );
        REQUIRE( c2.place == -8 );
        // lower + higher
        auto d1 = FixedPoint<13, -10>(1.23) + FixedPoint<7, 1>(0.23);
        REQUIRE( d1.width == 19 );
        REQUIRE( d1.place == -10 );
        // higher + lower
        auto d2 = FixedPoint<7, 1>(0.23) + FixedPoint<13, -10>(1.23);
        REQUIRE( d2.width == 19 );
        REQUIRE( d2.place == -10 );
        // lower + higher (without overlap)
        auto e1 = FixedPoint<10, -10>(1.23) + FixedPoint<7, 4>(0.23);
        REQUIRE( e1.width == 21 );
        REQUIRE( e1.place == -10 );
        // higher + lower (without overlap)
        auto e2 = FixedPoint<7, 4>(0.23) + FixedPoint<10, -10>(1.23);
        REQUIRE( e2.width == 21 );
        REQUIRE( e2.place == -10 );
        // lower + higher (overlap only sign bit)
        auto f1 = FixedPoint<10, -10>(1.23) + FixedPoint<7, -1>(0.23);
        REQUIRE( f1.width == 16 );
        REQUIRE( f1.place == -10 );
        // higher + lower (overlap only sign bit)
        auto f2 = FixedPoint<7, -1>(0.23) + FixedPoint<10, -10>(1.23);
        REQUIRE( f2.width == 16 );
        REQUIRE( f2.place == -10 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(2) * ((1u << 31) - 1);
        auto a = FixedPoint<32, 0>(1E+10) + FixedPoint<32, 0>(1E+10);
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        auto b = FixedPoint<32, 0>(-1E+10) + FixedPoint<32, 0>(-1E+10);
        REQUIRE( b.get_significand() == -expected );
        REQUIRE( b.width == 33 );
    }
}

TEST_CASE( "Binary operator -", "[FixedPoint]" ) {
    SECTION( "Value" ) {
        // some value
        REQUIRE( FixedPoint<8, -4>(1.23) - FixedPoint<7, -5>(0.23) == limit_precision(1.23, 8, -4) - limit_precision(0.23, 7, -5) );
        // maximum value
        REQUIRE( FixedPoint<8, 1>(1000.0) - FixedPoint<8, 1>(-1000.0) ==  508 );
        // minimum value
        REQUIRE( FixedPoint<8, -1>(-1000.0) - FixedPoint<8, -1>(1000.0) ==  -127 );
        // precision without overlap
        REQUIRE( FixedPoint<4, -4>(1000.0) - FixedPoint<7, 1>(-1000.0) ==  1.0/4 + 1.0/8 + 1.0/16 + 126);
        REQUIRE( FixedPoint<4, -4>(-1000.0) - FixedPoint<7, 1>(1000.0) ==  -(1.0/4 + 1.0/8 + 1.0/16 + 126) );
        // precision overlapped only sign bit
        REQUIRE( FixedPoint<4, -4>(1000.0) - FixedPoint<7, -1>(-1000.0) ==  1.0/4 + 1.0/8 + 1.0/16 + 31.5);
        REQUIRE( FixedPoint<4, -4>(-1000.0) - FixedPoint<7, -1>(1000.0) ==  -(1.0/4 + 1.0/8 + 1.0/16 + 31.5) );
    }
    SECTION( "Precision" ) {
        // same width and place
        auto a = FixedPoint<8, -5>(1.23) - FixedPoint<8, -5>(0.23);
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == -5 );
        // same place
        auto b = FixedPoint<9, -5>(1.23) - FixedPoint<7, -5>(0.23);
        REQUIRE( b.width == 10 );
        REQUIRE( b.place == -5 );
        // wider - narrower
        auto c1 = FixedPoint<10, -8>(1.23) - FixedPoint<5, -4>(0.23);
        REQUIRE( c1.width == 11 );
        REQUIRE( c1.place == -8 );
        // narrower - wider
        auto c2 = FixedPoint<5, -4>(0.23) - FixedPoint<10, -8>(1.23);
        REQUIRE( c2.width == 11 );
        REQUIRE( c2.place == -8 );
        // lower - higher
        auto d1 = FixedPoint<13, -10>(1.23) - FixedPoint<7, 1>(0.23);
        REQUIRE( d1.width == 19 );
        REQUIRE( d1.place == -10 );
        // higher - lower
        auto d2 = FixedPoint<7, 1>(0.23) - FixedPoint<13, -10>(1.23);
        REQUIRE( d2.width == 19 );
        REQUIRE( d2.place == -10 );
        // lower - higher (without overlap)
        auto e1 = FixedPoint<10, -10>(1.23) - FixedPoint<7, 4>(0.23);
        REQUIRE( e1.width == 21 );
        REQUIRE( e1.place == -10 );
        // higher - lower (without overlap)
        auto e2 = FixedPoint<7, 4>(0.23) - FixedPoint<10, -10>(1.23);
        REQUIRE( e2.width == 21 );
        REQUIRE( e2.place == -10 );
        // lower - higher (overlap only sign bit)
        auto f1 = FixedPoint<10, -10>(1.23) - FixedPoint<7, -1>(0.23);
        REQUIRE( f1.width == 16 );
        REQUIRE( f1.place == -10 );
        // higher - lower (overlap only sign bit)
        auto f2 = FixedPoint<7, -1>(0.23) - FixedPoint<10, -10>(1.23);
        REQUIRE( f2.width == 16 );
        REQUIRE( f2.place == -10 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(2) * ((1u << 31) - 1);
        auto a = FixedPoint<32, 0>(1E+10) - FixedPoint<32, 0>(-1E+10);
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        auto b = FixedPoint<32, 0>(-1E+10) - FixedPoint<32, 0>(1E+10);
        REQUIRE( b.get_significand() == -expected );
        REQUIRE( b.width == 33 );
    }
}

TEST_CASE( "Binary operator *", "[FixedPoint]" ) {
    SECTION( "Value" ) {
        // some value
        REQUIRE( FixedPoint<8, -4>(1.23) * FixedPoint<7, -5>(0.23) == limit_precision(1.23, 8, -4) * limit_precision(0.23, 7, -5) );
        // minimum value
        REQUIRE( FixedPoint<8, 1>(-1000.0) * FixedPoint<8, 2>(1000.0) ==  -129032 );
        // maximum value
        REQUIRE( FixedPoint<8, 1>(-1000.0) * FixedPoint<8, 2>(-1000.0) ==  129032 );
        REQUIRE( FixedPoint<10, -2>(1000.0) * FixedPoint<6, 3>(1000.0) ==  31682.0 );
    }
    SECTION( "Precision" ) {
        auto a = FixedPoint<8, -4>(1.23) * FixedPoint<7, -5>(0.23);
        REQUIRE( a.width == 14 );
        REQUIRE( a.place == -9 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(0xFFFF) * 0xFFFF;
        auto a = FixedPoint<17, 0>(1E+10) * FixedPoint<17, 0>(1E+10);
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        auto b = FixedPoint<17, 0>(-1E+10) * FixedPoint<17, 0>(1E+10);
        REQUIRE( b.get_significand() == -expected );
        REQUIRE( b.width == 33 );
    }
}

TEST_CASE( "Relational operator ==", "[FixedPoint]" ) {
    REQUIRE( FixedPoint<8, -4>(5.25) == FixedPoint<8, -4>(5.25) );
    REQUIRE( FixedPoint<8, -4>(5.25) == FixedPoint<7, -2>(5.25) );
    REQUIRE( FixedPoint<7, -2>(5.25) == FixedPoint<8, -4>(5.25) );
    REQUIRE( ! (FixedPoint<8, 0>(1E+10) == FixedPoint<4, 0>(1E+10)) );
    REQUIRE( ! (FixedPoint<8, 0>(-1E+10) == FixedPoint<4, 0>(-1E+10)) );
    // no overlapped range
    REQUIRE( ! (FixedPoint<2, -2>(1000.0) == FixedPoint<2, 8>(1000.0)) );
}

TEST_CASE( "Relational operator !=", "[FixedPoint]" ) {
    REQUIRE( FixedPoint<8, -4>(-5.3) != FixedPoint<8, -4>(-5.25) );
    REQUIRE( FixedPoint<8, -4>(-5.3) != FixedPoint<7, -2>(-5.3) );
    REQUIRE( FixedPoint<7, -2>(-5.3) != FixedPoint<8, -4>(-5.3) );
    REQUIRE( FixedPoint<2, -2>(1000.0) != FixedPoint<2, 8>(1000.0) );
    REQUIRE( FixedPoint<8, 0>(1E+10) != FixedPoint<4, 0>(1E+10) );
    REQUIRE( FixedPoint<8, 0>(-1E+10) != FixedPoint<4, 0>(-1E+10) );
    // no overlapped range
    REQUIRE( FixedPoint<2, -2>(1000.0) != FixedPoint<2, 8>(1000.0) );
}

TEST_CASE( "Relational operator <", "[FixedPoint]" ) {
    // some value
    REQUIRE( FixedPoint<8, -4>(-5.3) < FixedPoint<8, -4>(-5.25) );
    REQUIRE( FixedPoint<8, -4>(-5.3) < FixedPoint<7, -2>(-5.3) );
    REQUIRE( FixedPoint<7, -2>(5.3) < FixedPoint<8, -4>(5.3) );
    // no overlapped range
    REQUIRE( FixedPoint<2, -2>(1000.0) < FixedPoint<2, 8>(1000.0) );
}

TEST_CASE( "Relational operator <=", "[FixedPoint]" ) {
    // equal
    REQUIRE( FixedPoint<8, -4>(5.25) <= FixedPoint<8, -4>(5.25) );
    REQUIRE( FixedPoint<8, -4>(5.25) <= FixedPoint<7, -2>(5.25) );
    REQUIRE( FixedPoint<7, -2>(5.25) <= FixedPoint<8, -4>(5.25) );
    // less
    REQUIRE( FixedPoint<8, -4>(-5.3) <= FixedPoint<8, -4>(-5.25) );
    REQUIRE( FixedPoint<8, -4>(-5.3) <= FixedPoint<7, -2>(-5.3) );
    REQUIRE( FixedPoint<7, -2>(5.3) <= FixedPoint<8, -4>(5.3) );
    // no overlapped range
    REQUIRE( FixedPoint<2, -2>(1000.0) <= FixedPoint<2, 8>(1000.0) );
}

TEST_CASE( "Relational operator >", "[FixedPoint]" ) {
    // some value
    REQUIRE( FixedPoint<8, -4>(5.3) > FixedPoint<8, -4>(5.25) );
    REQUIRE( FixedPoint<8, -4>(5.3) > FixedPoint<7, -2>(5.3) );
    REQUIRE( FixedPoint<7, -2>(-5.3) > FixedPoint<8, -4>(-5.3) );
    // no overlapped range
    REQUIRE( FixedPoint<2, 8>(1000.0) > FixedPoint<2, -2>(1000.0) );
}

TEST_CASE( "Relational operator >=", "[FixedPoint]" ) {
    // equal
    REQUIRE( FixedPoint<8, -4>(-5.25) >= FixedPoint<8, -4>(-5.25) );
    REQUIRE( FixedPoint<8, -4>(-5.25) >= FixedPoint<7, -2>(-5.25) );
    REQUIRE( FixedPoint<7, -2>(-5.25) >= FixedPoint<8, -4>(-5.25) );
    // greater
    REQUIRE( FixedPoint<8, -4>(5.3) >= FixedPoint<8, -4>(5.25) );
    REQUIRE( FixedPoint<8, -4>(5.3) >= FixedPoint<7, -2>(5.3) );
    REQUIRE( FixedPoint<7, -2>(-5.3) >= FixedPoint<8, -4>(-5.3) );
    // no overlapped range
    REQUIRE( FixedPoint<2, 8>(1000.0) >= FixedPoint<2, -2>(1000.0) );
}

TEST_CASE( "exp2()", "[FixedPoint]" ) {
    REQUIRE( FixedPoint<8, -4>(5.25).exp2<10>() == limit_precision(5.25 * (1u<<10), 8, 6) );
    FixedPoint<8, 3> a(12345.0);
    REQUIRE( a.exp2<-10>().width == 8 );
    REQUIRE( a.exp2<-10>().place == -7 );
}

TEST_CASE( "ceil()", "[FixedPoint]" ) {
    SECTION( "Positive number" ) {
        // integer
        auto a(FixedPoint<8, 0>(100.0).ceil());
        REQUIRE( a == 100 );
        REQUIRE( a.width == 8 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, 2>(200.0).ceil());
        REQUIRE( b == 200 );
        REQUIRE( b.width == 16 );
        REQUIRE( b.place == 2 );
        // with number after the decimal point
        auto c1(FixedPoint<8, -4>(2.0 + 1.0/16).ceil());
        REQUIRE( c1 == 3 );
        REQUIRE( c1.width == 5 );
        REQUIRE( c1.place == 0 );
        auto c2(FixedPoint<8, -4>(2.5).ceil());
        REQUIRE( c2 == 3 );
        REQUIRE( c2.width == 5 );
        REQUIRE( c2.place == 0 );
        // only number after the decimal point
        auto d1(FixedPoint<8, -10>(1.0/1024).ceil());
        REQUIRE( d1 == 1 );
        REQUIRE( d1.width == 2 );
        REQUIRE( d1.place == 0 );
        auto d2(FixedPoint<8, -10>(0.5).ceil());
        REQUIRE( d2 == 1 );
        REQUIRE( d2.width == 2 );
        REQUIRE( d2.place == 0 );
    }
    SECTION( "Negative number" ) {
        // integer
        auto a(FixedPoint<8, 0>(-100.0).ceil());
        REQUIRE( a == -100 );
        REQUIRE( a.width == 8 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, 2>(-200.0).ceil());
        REQUIRE( b == -200 );
        REQUIRE( b.width == 16 );
        REQUIRE( b.place == 2 );
        // with number after the decimal point
        auto c1(FixedPoint<8, -4>(-2.0 - 1.0/16).ceil());
        REQUIRE( c1 == -2 );
        REQUIRE( c1.width == 5 );
        REQUIRE( c1.place == 0 );
        auto c2(FixedPoint<8, -4>(-2.5).ceil());
        REQUIRE( c2 == -2 );
        REQUIRE( c2.width == 5 );
        REQUIRE( c2.place == 0 );
        // only number after the decimal point
        auto d1(FixedPoint<8, -10>(-1.0/1024).ceil());
        REQUIRE( d1 == 0 );
        REQUIRE( d1.width == 2 );
        REQUIRE( d1.place == 0 );
        auto d2(FixedPoint<8, -10>(-0.5).ceil());
        REQUIRE( d2 == 0 );
        REQUIRE( d2.width == 2 );
        REQUIRE( d2.place == 0 );
    }
    SECTION( "Min/Max" ) {
        auto a(FixedPoint<16, -8>(128.0).ceil());
        REQUIRE( a == 128 );
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, -8>(-128.0).ceil());
        REQUIRE( b == -127 );
        REQUIRE( b.width == 9 );
        REQUIRE( b.place == 0 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(0x80000000u);
        auto a = FixedPoint<33, -1>(1E+10).ceil();
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        REQUIRE( a.place == 0 );
    }
}

TEST_CASE( "floor()", "[FixedPoint]" ) {
    SECTION( "Positive number" ) {
        // integer
        auto a(FixedPoint<8, 0>(100.0).floor());
        REQUIRE( a == 100 );
        REQUIRE( a.width == 8 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, 2>(200.0).floor());
        REQUIRE( b == 200 );
        REQUIRE( b.width == 16 );
        REQUIRE( b.place == 2 );
        // with number after the decimal point
        auto c1(FixedPoint<8, -4>(2.0 + 1.0/16).floor());
        REQUIRE( c1 == 2 );
        REQUIRE( c1.width == 5 );
        REQUIRE( c1.place == 0 );
        auto c2(FixedPoint<8, -4>(2.5).floor());
        REQUIRE( c2 == 2 );
        REQUIRE( c2.width == 5 );
        REQUIRE( c2.place == 0 );
        // only number after the decimal point
        auto d1(FixedPoint<8, -10>(1.0/1024).floor());
        REQUIRE( d1 == 0 );
        REQUIRE( d1.width == 2 );
        REQUIRE( d1.place == 0 );
        auto d2(FixedPoint<8, -10>(0.5).floor());
        REQUIRE( d2 == 0 );
        REQUIRE( d2.width == 2 );
        REQUIRE( d2.place == 0 );
    }
    SECTION( "Negative number" ) {
        // integer
        auto a(FixedPoint<8, 0>(-100.0).floor());
        REQUIRE( a == -100 );
        REQUIRE( a.width == 8 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, 2>(-200.0).floor());
        REQUIRE( b == -200 );
        REQUIRE( b.width == 16 );
        REQUIRE( b.place == 2 );
        // with number after the decimal point
        auto c1(FixedPoint<8, -4>(-2.0 - 1.0/16).floor());
        REQUIRE( c1 == -3 );
        REQUIRE( c1.width == 5 );
        REQUIRE( c1.place == 0 );
        auto c2(FixedPoint<8, -4>(-2.5).floor());
        REQUIRE( c2 == -3 );
        REQUIRE( c2.width == 5 );
        REQUIRE( c2.place == 0 );
        // only number after the decimal point
        auto d1(FixedPoint<8, -10>(-1.0/1024).floor());
        REQUIRE( d1 == -1 );
        REQUIRE( d1.width == 2 );
        REQUIRE( d1.place == 0 );
        auto d2(FixedPoint<8, -10>(-0.5).floor());
        REQUIRE( d2 == -1 );
        REQUIRE( d2.width == 2 );
        REQUIRE( d2.place == 0 );
    }
    SECTION( "Min/Max" ) {
        auto a(FixedPoint<16, -8>(128.0).floor());
        REQUIRE( a == 127 );
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, -8>(-128.0).floor());
        REQUIRE( b == -128 );
        REQUIRE( b.width == 9 );
        REQUIRE( b.place == 0 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(0x80000000u);
        auto a = FixedPoint<33, -1>(-1E+10).floor();
        REQUIRE( a.get_significand() ==  -expected );
        REQUIRE( a.width == 33 );
        REQUIRE( a.place == 0 );
    }
}

TEST_CASE( "round_half_to_even()", "[FixedPoint]" ) {
    SECTION( "not midway" ) {
        SECTION( "Positive number" ) {
            // integer
            auto a(FixedPoint<8, 0>(100.0).round_half_to_even());
            REQUIRE( a == 100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(200.0).round_half_to_even());
            REQUIRE( b == 200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(2.0 + 1.0/16).round_half_to_even());
            REQUIRE( c1 == 2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(2.5 + 1.0/16).round_half_to_even());
            REQUIRE( c2 == 3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(1.0/128).round_half_to_even());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(0.5 + 1.0/128).round_half_to_even());
            REQUIRE( d == 1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
        SECTION( "Negative number" ) {
            // integer
            auto a(FixedPoint<8, 0>(-100.0).round_half_to_even());
            REQUIRE( a == -100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(-200.0).round_half_to_even());
            REQUIRE( b == -200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(-2.0 - 1.0/16).round_half_to_even());
            REQUIRE( c1 == -2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(-2.5 - 1.0/16).round_half_to_even());
            REQUIRE( c2 == -3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-1.0/128).round_half_to_even());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(-0.5 - 1.0/128).round_half_to_even());
            REQUIRE( d == -1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
    }
    SECTION( "midway" ) {
        SECTION( "Positive number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(3.5).round_half_to_even());
            REQUIRE( c1 == 4 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(2.5).round_half_to_even());
            REQUIRE( c2 == 2 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(0.5).round_half_to_even());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
        SECTION( "Negative number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(-2.5).round_half_to_even());
            REQUIRE( c1 == -2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(-3.5).round_half_to_even());
            REQUIRE( c2 == -4 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-0.5).round_half_to_even());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
    }
    SECTION( "Min/Max" ) {
        auto a(FixedPoint<16, -8>(128.0).round_half_to_even());
        REQUIRE( a == 128 );
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, -8>(-128.0).round_half_to_even());
        REQUIRE( b == -128 );
        REQUIRE( b.width == 9 );
        REQUIRE( b.place == 0 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(0x80000000u);
        auto a = FixedPoint<34, -2>(1E+10).round_half_to_even();
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        REQUIRE( a.place == 0 );
    }
}

TEST_CASE( "round_half_away_from_zero()", "[FixedPoint]" ) {
    SECTION( "not midway" ) {
        SECTION( "Positive number" ) {
            // integer
            auto a(FixedPoint<8, 0>(100.0).round_half_away_from_zero());
            REQUIRE( a == 100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(200.0).round_half_away_from_zero());
            REQUIRE( b == 200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(2.0 + 1.0/16).round_half_away_from_zero());
            REQUIRE( c1 == 2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(2.5 + 1.0/16).round_half_away_from_zero());
            REQUIRE( c2 == 3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(1.0/128).round_half_away_from_zero());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(0.5 + 1.0/128).round_half_away_from_zero());
            REQUIRE( d == 1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
        SECTION( "Negative number" ) {
            // integer
            auto a(FixedPoint<8, 0>(-100.0).round_half_away_from_zero());
            REQUIRE( a == -100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(-200.0).round_half_away_from_zero());
            REQUIRE( b == -200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(-2.0 - 1.0/16).round_half_away_from_zero());
            REQUIRE( c1 == -2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(-2.5 - 1.0/16).round_half_away_from_zero());
            REQUIRE( c2 == -3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-1.0/128).round_half_away_from_zero());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(-0.5 - 1.0/128).round_half_away_from_zero());
            REQUIRE( d == -1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
    }
    SECTION( "midway" ) {
        SECTION( "Positive number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(3.5).round_half_away_from_zero());
            REQUIRE( c1 == 4 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(2.5).round_half_away_from_zero());
            REQUIRE( c2 == 3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(0.5).round_half_away_from_zero());
            REQUIRE( d1 == 1 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
        SECTION( "Negative number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(-2.5).round_half_away_from_zero());
            REQUIRE( c1 == -3 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(-3.5).round_half_away_from_zero());
            REQUIRE( c2 == -4 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-0.5).round_half_away_from_zero());
            REQUIRE( d1 == -1 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
    }
    SECTION( "Min/Max" ) {
        auto a(FixedPoint<16, -8>(128.0).round_half_away_from_zero());
        REQUIRE( a == 128 );
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, -8>(-128.0).round_half_away_from_zero());
        REQUIRE( b == -128 );
        REQUIRE( b.width == 9 );
        REQUIRE( b.place == 0 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(0x80000000u);
        auto a = FixedPoint<34, -2>(1E+10).round_half_away_from_zero();
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        REQUIRE( a.place == 0 );
    }
}

TEST_CASE( "round_half_toward_zero()", "[FixedPoint]" ) {
    SECTION( "not midway" ) {
        SECTION( "Positive number" ) {
            // integer
            auto a(FixedPoint<8, 0>(100.0).round_half_toward_zero());
            REQUIRE( a == 100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(200.0).round_half_toward_zero());
            REQUIRE( b == 200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(2.0 + 1.0/16).round_half_toward_zero());
            REQUIRE( c1 == 2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(2.5 + 1.0/16).round_half_toward_zero());
            REQUIRE( c2 == 3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(1.0/128).round_half_toward_zero());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(0.5 + 1.0/128).round_half_toward_zero());
            REQUIRE( d == 1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
        SECTION( "Negative number" ) {
            // integer
            auto a(FixedPoint<8, 0>(-100.0).round_half_toward_zero());
            REQUIRE( a == -100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(-200.0).round_half_toward_zero());
            REQUIRE( b == -200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(-2.0 - 1.0/16).round_half_toward_zero());
            REQUIRE( c1 == -2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(-2.5 - 1.0/16).round_half_toward_zero());
            REQUIRE( c2 == -3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-1.0/128).round_half_toward_zero());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(-0.5 - 1.0/128).round_half_toward_zero());
            REQUIRE( d == -1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
    }
    SECTION( "midway" ) {
        SECTION( "Positive number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(3.5).round_half_toward_zero());
            REQUIRE( c1 == 3 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(2.5).round_half_toward_zero());
            REQUIRE( c2 == 2 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(0.5).round_half_toward_zero());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
        SECTION( "Negative number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(-2.5).round_half_toward_zero());
            REQUIRE( c1 == -2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(-3.5).round_half_toward_zero());
            REQUIRE( c2 == -3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-0.5).round_half_toward_zero());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
    }
    SECTION( "Min/Max" ) {
        auto a(FixedPoint<16, -8>(128.0).round_half_toward_zero());
        REQUIRE( a == 128 );
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, -8>(-128.0).round_half_toward_zero());
        REQUIRE( b == -128 );
        REQUIRE( b.width == 9 );
        REQUIRE( b.place == 0 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(0x80000000u);
        auto a = FixedPoint<34, -2>(1E+10).round_half_toward_zero();
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        REQUIRE( a.place == 0 );
    }
}

TEST_CASE( "round_half_up()", "[FixedPoint]" ) {
    SECTION( "not midway" ) {
        SECTION( "Positive number" ) {
            // integer
            auto a(FixedPoint<8, 0>(100.0).round_half_up());
            REQUIRE( a == 100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(200.0).round_half_up());
            REQUIRE( b == 200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(2.0 + 1.0/16).round_half_up());
            REQUIRE( c1 == 2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(2.5 + 1.0/16).round_half_up());
            REQUIRE( c2 == 3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(1.0/128).round_half_up());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(0.5 + 1.0/128).round_half_up());
            REQUIRE( d == 1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
        SECTION( "Negative number" ) {
            // integer
            auto a(FixedPoint<8, 0>(-100.0).round_half_up());
            REQUIRE( a == -100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(-200.0).round_half_up());
            REQUIRE( b == -200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(-2.0 - 1.0/16).round_half_up());
            REQUIRE( c1 == -2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(-2.5 - 1.0/16).round_half_up());
            REQUIRE( c2 == -3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-1.0/128).round_half_up());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(-0.5 - 1.0/128).round_half_up());
            REQUIRE( d == -1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
    }
    SECTION( "midway" ) {
        SECTION( "Positive number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(3.5).round_half_up());
            REQUIRE( c1 == 4 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(2.5).round_half_up());
            REQUIRE( c2 == 3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(0.5).round_half_up());
            REQUIRE( d1 == 1 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
        SECTION( "Negative number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(-2.5).round_half_up());
            REQUIRE( c1 == -2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(-3.5).round_half_up());
            REQUIRE( c2 == -3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-0.5).round_half_up());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
    }
    SECTION( "Min/Max" ) {
        auto a(FixedPoint<16, -8>(128.0).round_half_up());
        REQUIRE( a == 128 );
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, -8>(-128.0).round_half_up());
        REQUIRE( b == -128 );
        REQUIRE( b.width == 9 );
        REQUIRE( b.place == 0 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(0x80000000u);
        auto a = FixedPoint<34, -2>(1E+10).round_half_up();
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        REQUIRE( a.place == 0 );
    }
}

TEST_CASE( "round_half_down()", "[FixedPoint]" ) {
    SECTION( "not midway" ) {
        SECTION( "Positive number" ) {
            // integer
            auto a(FixedPoint<8, 0>(100.0).round_half_down());
            REQUIRE( a == 100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(200.0).round_half_down());
            REQUIRE( b == 200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(2.0 + 1.0/16).round_half_down());
            REQUIRE( c1 == 2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(2.5 + 1.0/16).round_half_down());
            REQUIRE( c2 == 3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(1.0/128).round_half_down());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(0.5 + 1.0/128).round_half_down());
            REQUIRE( d == 1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
        SECTION( "Negative number" ) {
            // integer
            auto a(FixedPoint<8, 0>(-100.0).round_half_down());
            REQUIRE( a == -100 );
            REQUIRE( a.width == 8 );
            REQUIRE( a.place == 0 );
            auto b(FixedPoint<16, 2>(-200.0).round_half_down());
            REQUIRE( b == -200 );
            REQUIRE( b.width == 16 );
            REQUIRE( b.place == 2 );
            // with number after the decimal point
            auto c1(FixedPoint<8, -4>(-2.0 - 1.0/16).round_half_down());
            REQUIRE( c1 == -2 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            auto c2(FixedPoint<8, -4>(-2.5 - 1.0/16).round_half_down());
            REQUIRE( c2 == -3 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-1.0/128).round_half_down());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
            auto d(FixedPoint<8, -7>(-0.5 - 1.0/128).round_half_down());
            REQUIRE( d == -1 );
            REQUIRE( d.width == 2 );
            REQUIRE( d.place == 0 );
        }
    }
    SECTION( "midway" ) {
        SECTION( "Positive number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(3.5).round_half_down());
            REQUIRE( c1 == 3 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(2.5).round_half_down());
            REQUIRE( c2 == 2 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(0.5).round_half_down());
            REQUIRE( d1 == 0 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
        SECTION( "Negative number" ) {
            // with number after the decimal point
            // upward
            auto c1(FixedPoint<8, -4>(-2.5).round_half_down());
            REQUIRE( c1 == -3 );
            REQUIRE( c1.width == 5 );
            REQUIRE( c1.place == 0 );
            // downward
            auto c2(FixedPoint<8, -4>(-3.5).round_half_down());
            REQUIRE( c2 == -4 );
            REQUIRE( c2.width == 5 );
            REQUIRE( c2.place == 0 );
            // only number after the decimal point
            auto d1(FixedPoint<8, -7>(-0.5).round_half_down());
            REQUIRE( d1 == -1 );
            REQUIRE( d1.width == 2 );
            REQUIRE( d1.place == 0 );
        }
    }
    SECTION( "Min/Max" ) {
        auto a(FixedPoint<16, -8>(128.0).round_half_down());
        REQUIRE( a == 128 );
        REQUIRE( a.width == 9 );
        REQUIRE( a.place == 0 );
        auto b(FixedPoint<16, -8>(-128.0).round_half_down());
        REQUIRE( b == -128 );
        REQUIRE( b.width == 9 );
        REQUIRE( b.place == 0 );
    }
    SECTION( "over 32 bits" ) {
        const std::int_fast64_t expected = static_cast<std::int_fast64_t>(0x80000000u);
        auto a = FixedPoint<34, -2>(1E+10).round_half_down();
        REQUIRE( a.get_significand() ==  expected );
        REQUIRE( a.width == 33 );
        REQUIRE( a.place == 0 );
    }
}

TEST_CASE( "get_significand()", "[FixedPoint]" ) {
    // some value
    REQUIRE( FixedPoint<8, -4>(3.0 + 1.0/16).get_significand() == 0x31 );
    REQUIRE( FixedPoint<8, -4>(-3.0 - 1.0/16).get_significand() == -0x31 );
    // maximum value
    REQUIRE( FixedPoint<8, -4>(1000.0).get_significand() == 0x7F );
    // minimum value
    REQUIRE( FixedPoint<8, -4>(-1000.0).get_significand() == -0x7F );
}

TEST_CASE( "set_significand()", "[FixedPoint]" ) {
    // some value
    FixedPoint<8, -4> a;
    a.set_significand(0x31);
    REQUIRE( a == 3.0 + 1.0/16 );
    a.set_significand(-0x31);
    REQUIRE( a == -3.0 - 1.0/16 );
    // maximum value
    a.set_significand(1000);
    REQUIRE( a == 7.0 + 1.0/2 + 1.0/4 + 1.0/8 + 1.0/16 );
    // minimum value
    a.set_significand(-1000);
    REQUIRE( a == -(7.0 + 1.0/2 + 1.0/4 + 1.0/8 + 1.0/16) );
    // clamping
    a.set_significand(0x100000001LL);
    REQUIRE( a == 7.0 + 1.0/2 + 1.0/4 + 1.0/8 + 1.0/16 );
}

TEST_CASE( "width", "[FixedPoint]" ) {
    // some value
    REQUIRE( FixedPoint<8, -4>::width == 8 );
    // maximum value
    REQUIRE( FixedPoint<54, -4>::width == 54 );
    // minimum value
    REQUIRE( FixedPoint<2, -4>::width == 2 );
}

TEST_CASE( "place", "[FixedPoint]" ) {
    // some value
    REQUIRE( FixedPoint<8, 3>::place == 3 );
    REQUIRE( FixedPoint<8, -4>::place == -4 );
    // maximum value
    REQUIRE( FixedPoint<2, 1022>::place == 1022 );
    REQUIRE( FixedPoint<10, 1014>::place == 1014 );
    // minimum value
    REQUIRE( FixedPoint<2, -1022>::place == -1022 );
    REQUIRE( FixedPoint<10, -1022>::place == -1022 );
}
