/*
  Copyright © 2023 OOTA, Masato

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

#include <complex>
#include <type_traits>

#include "double_limit.h"
#include "FixedPointComplex.h"

using namespace prec_ctrl;

TEST_CASE( "value_t", "[FixedPointComplex]" ) {
    REQUIRE( std::is_same_v<FixedPointComplex<8, 0>::value_t,
                            FixedPoint<8, 0>> );
    REQUIRE( std::is_same_v<Complex<FixedPoint<16, -8>>::value_t,
                            FixedPoint<16, -8>> );
}

TEST_CASE( "Default constructor (Complex)", "[FixedPointComplex]" ) {
    FixedPoint<8, 0> zero;
    REQUIRE( FixedPointComplex<8, 0>() == Complex(zero) );
    REQUIRE( FixedPointComplex<8, 0>() == Complex(zero, zero) );
    REQUIRE( FixedPointComplex<8, 0>().dbl() == 0.0 );
    REQUIRE( FixedPointComplex<8, 0>().dbl() == std::complex<double>() );
}

TEST_CASE( "Copy constructor (Complex)", "[FixedPointComplex]" ) {
    FixedPointComplex<8, -4> a(3.1234, -2.4321);
    REQUIRE( FixedPointComplex<8, -4>(a) == a );
}

TEST_CASE( "Assign operator (Complex)", "[FixedPointComplex]" ) {
    FixedPointComplex<8, -4> a(3.1234, -2.4321);
    FixedPointComplex<8, -4> b;
    b = a;
    REQUIRE( b == a );
    REQUIRE( static_cast<std::complex<double>>(b) == a.dbl() );
}

TEST_CASE( "Construct out of FixedPoint", "[FixedPointComplex]" ) {
    FixedPoint<8, 0> re(-34.5);
    FixedPoint<8, 0> im(32.1);
    REQUIRE( FixedPointComplex<8, 0>(re).real() == re );
    REQUIRE( FixedPointComplex<8, 0>(re).imag() == 0.0 );
    REQUIRE( FixedPointComplex<8, 0>(re, im).real() == re );
    REQUIRE( FixedPointComplex<8, 0>(re, im).imag() == im );
    REQUIRE( Complex(re).real() == re );
    REQUIRE( Complex(re).imag() == 0.0 );
    REQUIRE( Complex(re, im).real() == re );
    REQUIRE( Complex(re, im).imag() == im );
}

TEST_CASE( "Copy from narrower (FP to Complex)", "[FixedPointComplex]" ) {
    FixedPoint<8, -4> re(-6.9876);
    FixedPoint<8, -4> im(5.8765);
    FixedPointComplex<8, -4> a(re, im);
    // To widen toward LSB
    REQUIRE( FixedPointComplex<9, -5>(re, im) == a );
    REQUIRE( FixedPointComplex<9, -5>(re, im).dbl() == a.dbl() );
    // To widen toward MSB
    REQUIRE( FixedPointComplex<9, -4>(re, im) == a );
    REQUIRE( FixedPointComplex<9, -4>(re, im).dbl() == a.dbl() );
    // To widen both sides
    REQUIRE( FixedPointComplex<20, -8>(re, im) == a );
    REQUIRE( FixedPointComplex<20, -8>(re, im).dbl() == a.dbl() );

    // different types
    FixedPoint<7, -3> im2(5.8765);
    FixedPointComplex<8, -4> b(re, im2);
    REQUIRE( FixedPointComplex<9, -5>(re, im2) == b );
}

TEST_CASE( "Copy from narrower (Complex)", "[FixedPointComplex]" ) {
    FixedPointComplex<8, -4> a(-6.9876, 5.8765);
    // To widen toward LSB
    REQUIRE( FixedPointComplex<9, -5>(a) == a );
    REQUIRE( FixedPointComplex<9, -5>(a).dbl() == a.dbl() );
    // To widen toward MSB
    REQUIRE( FixedPointComplex<9, -4>(a) == a );
    REQUIRE( FixedPointComplex<9, -4>(a).dbl() == a.dbl() );
    // To widen both sides
    REQUIRE( FixedPointComplex<20, -8>(a) == a );
    REQUIRE( FixedPointComplex<20, -8>(a).dbl() == a.dbl() );
}

TEST_CASE( "Assign from narrower (Complex)", "[FixedPointComplex]" ) {
    FixedPointComplex<8, -4> a(-6.9876, 5.8765);
    // To widen toward LSB
    FixedPointComplex<10, -6> b;
    b = a;
    REQUIRE( b == a );
    REQUIRE( b.dbl() == a.dbl() );
    // To widen toward MSB
    FixedPointComplex<10, -4> c;
    c = a;
    REQUIRE( c == a );
    REQUIRE( c.dbl() == a.dbl() );
    // To widen both sides
    FixedPointComplex<20, -8> d;
    d = a;
    REQUIRE( d == a );
    REQUIRE( d.dbl() == a.dbl() );
}

TEST_CASE( "Reduce dynamic range (Complex)", "[FixedPointComplex]" ) {
    FixedPointComplex<12, -4> a(123.9876, -98.7654);
    FixedPoint<8, -4> clamped(4.0 + 2 + 1 + 1.0/2 + 1.0/4 + 1.0/8 + 1.0/16);
    REQUIRE( a.reduce_dynamic_range<8>() == Complex(clamped, -clamped) );
    REQUIRE( std::is_same_v<decltype(a.reduce_dynamic_range<8>()),
                            FixedPointComplex<8, -4>> );
    REQUIRE( a.reduce_dynamic_range<10>()
             == Complex(FixedPoint<10, -4>(123.9876),
                        FixedPoint<10, -4>(-98.7654)) );
    REQUIRE( std::is_same_v<decltype(a.reduce_dynamic_range<10>()),
                            FixedPointComplex<10, -4>> );
    REQUIRE( a.reduce_dynamic_range<6>().dbl()
             == std::complex<double>(limit_precision(a.real(), 6, -4),
                                     limit_precision(a.imag(), 6, -4)) );
    FixedPoint<33, 0> b2;
    b2.set_significand(0x100000001LL);
    FixedPointComplex<33, 0> b(-b2, b2);
    REQUIRE( b.reduce_dynamic_range<16>().dbl()
             == std::complex<double>(-0x7FFF, 0x7FFF) );
}

TEST_CASE( "Copy from double (Complex)", "[FixedPointComplex]" ) {
    double re = 56789.123456;
    double im = -12345.6789;
    // some value
    REQUIRE( FixedPointComplex<33, -16>(re, im).dbl()
             == std::complex<double>(limit_precision(re, 33, -16),
                                     limit_precision(im, 33, -16)) );
    REQUIRE( FixedPointComplex<16, 4>(re, im).dbl()
             == std::complex<double>(limit_precision(re, 16, 4),
                                     limit_precision(im, 16, 4)) );
    // maximum value
    REQUIRE( FixedPointComplex<8, -10>(re, -im).dbl()
             == std::complex<double>(limit_precision(re, 8, -10),
                                     limit_precision(-im, 8, -10)) );
    // minimum value
    REQUIRE( FixedPointComplex<8, 0>(-re, im).dbl()
             == std::complex<double>(-127.0, -127.0) );
    // clamping
    double b = 0x10001000L;
    b *= std::exp2(36);
    REQUIRE( FixedPointComplex<54, 0>(b).dbl()
             == std::complex<double>(0x001FFFFFFFFFFFFFLL) );
    REQUIRE( FixedPointComplex<54, 0>(0.0, b).dbl()
             == std::complex<double>(0.0, 0x001FFFFFFFFFFFFFLL) );
}

TEST_CASE( "Copy from std::complex<double>", "[FixedPointComplex]" ) {
    std::complex<double> a1(56789.123456, -12345.6789);
    std::complex<double> a2(56789.123456, 12345.6789);
    std::complex<double> a3(-56789.123456, -12345.6789);
    // some value
    REQUIRE( FixedPointComplex<33, -16>(a1).dbl()
             == std::complex<double>(limit_precision(a1.real(), 33, -16),
                                     limit_precision(a1.imag(), 33, -16)) );
    REQUIRE( FixedPointComplex<16, 4>(a1).dbl()
             == std::complex<double>(limit_precision(a1.real(), 16, 4),
                                     limit_precision(a1.imag(), 16, 4)) );
    // maximum value
    REQUIRE( FixedPointComplex<8, -10>(a2).dbl()
             == std::complex<double>(limit_precision(a2.real(), 8, -10),
                                     limit_precision(a2.imag(), 8, -10)) );
    // minimum value
    REQUIRE( FixedPointComplex<8, 0>(a3).dbl()
             == std::complex<double>(-127.0, -127.0) );
    // clamping
    double b = 0x10001000L;
    b *= std::exp2(36);
    REQUIRE( FixedPointComplex<54, 0>(b).dbl()
             == std::complex<double>(0x001FFFFFFFFFFFFFLL) );
    REQUIRE( FixedPointComplex<54, 0>(0.0, b).dbl()
             == std::complex<double>(0.0, 0x001FFFFFFFFFFFFFLL) );
}

bool equal(const std::complex<double> &op1, const std::complex<double> &op2)
{
    return op1 == op2;
}

TEST_CASE( "Implicit conversion to std::complex<double>", "[FixedPointComplex]" ) {
    REQUIRE( equal(FixedPointComplex<8, -4>(-1.23, 5.67),
                   std::complex<double>(limit_precision(-1.23, 8, -4),
                                        limit_precision(5.67, 8, -4))) );
}

TEST_CASE( "Explicit conversion to std::complex<double>", "[FixedPointComplex]" ) {
    REQUIRE( FixedPointComplex<8, -4>(-1.23, 5.67).dbl()
             == std::complex<double>(limit_precision(-1.23, 8, -4),
                                     limit_precision(5.67, 8, -4)) );
    REQUIRE( std::is_same_v<decltype(FixedPointComplex<8, -4>(-1.23, 5.67).dbl()),
                            std::complex<double>> );
    REQUIRE( static_cast<std::complex<double>>(FixedPointComplex<8, -4>(-1.23, 5.67))
             == std::complex<double>(limit_precision(-1.23, 8, -4),
                                     limit_precision(5.67, 8, -4)) );
}

TEST_CASE( "Explicit conversion to bool (Complex)", "[FixedPointComplex]" ) {
    REQUIRE( static_cast<bool>(FixedPointComplex<8, -4>(1.23)) );
    REQUIRE( static_cast<bool>(FixedPointComplex<8, -4>(0.0, 1.23)) );
    REQUIRE( static_cast<bool>(FixedPointComplex<8, -4>(0.00001)) == false );
    REQUIRE( static_cast<bool>(FixedPointComplex<8, -4>(0.0, 0.00001)) == false );
}

TEST_CASE( "Unary operator + (Complex)", "[FixedPointComplex]" ) {
    // some value
    REQUIRE( +FixedPointComplex<22, -10>(599.7, -321.1)
             == FixedPointComplex<22, -10>(599.7, -321.1) );
    REQUIRE( +FixedPointComplex<22, -10>(-100000.0, 100000.0)
             == FixedPointComplex<22, -10>(-100000.0, 100000.0) );
    // maximum value
    REQUIRE( +FixedPointComplex<8, -1>(100.0).dbl() == 63.5 );
    REQUIRE( +FixedPointComplex<8, -1>(100.0).real().get_significand() == 0x7F );
    REQUIRE( +FixedPointComplex<8, -1>(0.0, 100.0).dbl()
             == std::complex<double>(0.0, 63.5) );
    REQUIRE( +FixedPointComplex<8, -1>(0.0, 100.0).imag().get_significand() == 0x7F );
    // minimum value
    REQUIRE( +FixedPointComplex<4, 1>(-100.0).dbl() == -14.0 );
    REQUIRE( +FixedPointComplex<4, 1>(-100.0).real().get_significand() == -7 );
    REQUIRE( +FixedPointComplex<4, 1>(0.0, -100.0).dbl()
             == std::complex<double>(0.0, -14.0) );
    REQUIRE( +FixedPointComplex<4, 1>(0.0, -100.0).imag().get_significand() == -7 );
}

TEST_CASE( "Unary operator - (Complex)", "[FixedPointComplex]" ) {
    // some value
    REQUIRE( -FixedPointComplex<22, -10>(599.7, -321.1)
             == FixedPointComplex<22, -10>(-599.7, 321.1) );
    REQUIRE( -FixedPointComplex<22, -10>(-100000.0, 100000.0)
             == FixedPointComplex<22, -10>(100000.0, -100000.0) );
    // maximum value
    REQUIRE( -FixedPointComplex<7, 1>(-128.0).dbl() == 126.0 );
    REQUIRE( -FixedPointComplex<7, 1>(-128.0).real().get_significand() == 0x3F );
    REQUIRE( -FixedPointComplex<7, 1>(0.0, -128.0).dbl()
             == std::complex<double>(0.0, 126.0) );
    REQUIRE( -FixedPointComplex<7, 1>(0.0, -128.0).imag().get_significand() == 0x3F );
    // minimum value
    REQUIRE( -FixedPointComplex<8, -1>(64.0).dbl() == -63.5 );
    REQUIRE( -FixedPointComplex<8, -1>(64.0).real().get_significand() == -0x7F );
    REQUIRE( -FixedPointComplex<8, -1>(0.0, 64.0).dbl()
             == std::complex<double>(0.0, -63.5) );
    REQUIRE( -FixedPointComplex<8, -1>(0.0, 64.0).imag().get_significand() == -0x7F );
}

TEST_CASE( "Unary operator ! (Complex)", "[FixedPointComplex]" ) {
    REQUIRE( !FixedPointComplex<8, -4>(1.23) == false );
    REQUIRE( !FixedPointComplex<8, -4>(0.0, 1.23) == false );
    REQUIRE( !FixedPointComplex<8, -4>(0.00001) );
    REQUIRE( !FixedPointComplex<8, -4>(0.0, 0.00001) );
}

TEST_CASE( "Binary operator + (Complex)", "[FixedPointComplex]" ) {
    SECTION( "Value" ) {
        REQUIRE( (FixedPointComplex<8, -4>(1.23, -3.21)
                  + FixedPointComplex<7, -5>(0.23, -0.43)).dbl()
                 == std::complex<double>(limit_precision(1.23, 8, -4)
                                         + limit_precision(0.23, 7, -5),
                                         limit_precision(-3.21, 8, -4)
                                         + limit_precision(-0.45, 7, -5)) );
    }
    SECTION( "Precision" ) {
        auto a = FixedPointComplex<8, -4>(-1.23, 3.21) + FixedPointComplex<7, -5>(-0.23, 0.43);
        REQUIRE( a.width == 10 );
        REQUIRE( a.place == -5 );
    }
}

TEST_CASE( "Binary operator - (Complex)", "[FixedPointComplex]" ) {
    SECTION( "Value" ) {
        REQUIRE( (FixedPointComplex<8, -4>(1.23, -3.21)
                  - FixedPointComplex<7, -5>(0.23, -0.43)).dbl()
                 == std::complex<double>(limit_precision(1.23, 8, -4)
                                         - limit_precision(0.23, 7, -5),
                                         limit_precision(-3.21, 8, -4)
                                         - limit_precision(-0.45, 7, -5)) );
    }
    SECTION( "Precision" ) {
        auto a = FixedPointComplex<8, -4>(-1.23, 3.21) - FixedPointComplex<7, -5>(-0.23, 0.43);
        REQUIRE( a.width == 10 );
        REQUIRE( a.place == -5 );
    }
}

TEST_CASE( "Binary operator * (Complex)", "[FixedPointComplex]" ) {
    SECTION( "Value" ) {
        double re1 = limit_precision(1.23, 8, -4);
        double im1 = limit_precision(-3.21, 8, -4);
        double re2 = limit_precision(0.23, 7, -5);
        double im2 = limit_precision(-0.43, 7, -5);
        REQUIRE( (FixedPointComplex<8, -4>(1.23, -3.21)
                  * FixedPointComplex<7, -5>(0.23, -0.43)).dbl()
                 == std::complex<double>(re1*re2 - im1*im2, re1*im2 + re2*im1) );
    }
    SECTION( "Precision" ) {
        auto a = FixedPointComplex<8, -4>(-1.23, 3.21) * FixedPointComplex<7, -5>(-0.23, 0.43);
        REQUIRE( a.width == 15 );
        REQUIRE( a.place == -9 );
    }
}

TEST_CASE( "Relational operator == (Complex)", "[FixedPointComplex]" ) {
    REQUIRE( FixedPointComplex<8, -4>(5.25, -4.5) == FixedPointComplex<8, -4>(5.25, -4.5) );
    REQUIRE( FixedPointComplex<8, -4>(5.25, -4.5) == FixedPointComplex<7, -2>(5.25, -4.5) );
    REQUIRE( FixedPointComplex<7, -2>(5.25, -4.5) == FixedPointComplex<8, -4>(5.25, -4.5) );
}

TEST_CASE( "Relational operator != (Complex)", "[FixedPointComplex]" ) {
    REQUIRE( FixedPointComplex<8, -4>(5.3, -4.4) != FixedPointComplex<8, -4>(5.25, -4.5) );
    REQUIRE( FixedPointComplex<8, -4>(5.3, -4.5) != FixedPointComplex<8, -4>(5.25, -4.5) );
    REQUIRE( FixedPointComplex<8, -4>(5.3, -4.5) != FixedPointComplex<7, -2>(5.3, -4.5) );
    REQUIRE( FixedPointComplex<7, -2>(5.3, -4.5) != FixedPointComplex<8, -4>(5.3, -4.5) );
    REQUIRE( FixedPointComplex<8, -4>(5.2, -4.4) != FixedPointComplex<8, -4>(5.25, -4.5) );
    REQUIRE( FixedPointComplex<8, -4>(5.2, -4.4) != FixedPointComplex<7, -2>(5.3, -4.5) );
    REQUIRE( FixedPointComplex<7, -2>(5.2, -4.4) != FixedPointComplex<8, -4>(5.3, -4.5) );
}

TEST_CASE( "norm()", "[FixedPointComplex]" ) {
    REQUIRE( FixedPointComplex<8, -4>(5.25, -2.34).norm()
             == limit_precision(5.25, 8, -4) * limit_precision(5.25, 8, -4)
                + limit_precision(-2.34, 8, -4) * limit_precision(-2.34, 8, -4) );
    FixedPointComplex<8, -4> a(5.25, -2.34);
    REQUIRE( a.norm().width == 16 );
    REQUIRE( a.norm().place == -8 );
}

TEST_CASE( "conj()", "[FixedPointComplex]" ) {
    REQUIRE( FixedPointComplex<8, -4>(5.25, -2.34).conj()
             == FixedPointComplex<8, -4>(5.25, 2.34) );
    REQUIRE( FixedPointComplex<8, -4>(5.25, 2.34).conj()
             == FixedPointComplex<8, -4>(5.25, -2.34) );
    FixedPointComplex<8, -4> a(5.25, -2.34);
    REQUIRE( a.conj().width == 8 );
    REQUIRE( a.conj().place == -4 );
}

TEST_CASE( "inphase()", "[FixedPointComplex]" ) {
    FixedPointComplex<8, 0> a1(63.0, 127.0);
    FixedPointComplex<2, 0> ref1(-1.0, 0.0);
    auto i1 = a1.inphase(ref1);
    REQUIRE( i1 == -63.0 );
    REQUIRE( i1.width == 10 );
    REQUIRE( i1.place == 0 );

    FixedPointComplex<8, 0> a2(63.0, 127.0);
    FixedPointComplex<2, 0> ref2(0.0, 1.0);
    auto i2 = a2.inphase(ref2);
    REQUIRE( i2 == 127.0 );
    REQUIRE( i2.width == 10 );
    REQUIRE( i2.place == 0 );

    FixedPointComplex<8, -4> a3(5.25, -2.34);
    FixedPointComplex<5, -2> ref3(-1.23, 2.1);
    auto i3 = a3.inphase(ref3);
    REQUIRE( i3 == limit_precision(5.25, 8, -4) * limit_precision(-1.23, 5, -2)
                   + limit_precision(-2.34, 8, -4) * limit_precision(2.1, 5, -2) );
    REQUIRE( i3.width == 13 );
    REQUIRE( i3.place == -6 );
}

TEST_CASE( "quadrature()", "[FixedPointComplex]" ) {
    FixedPointComplex<8, 0> a1(63.0, 127.0);
    FixedPointComplex<2, 0> ref1(-1.0, 0.0);
    auto i1 = a1.quadrature(ref1);
    REQUIRE( i1 == -127.0 );
    REQUIRE( i1.width == 10 );
    REQUIRE( i1.place == 0 );

    FixedPointComplex<8, 0> a2(63.0, 127.0);
    FixedPointComplex<2, 0> ref2(0.0, 1.0);
    auto i2 = a2.quadrature(ref2);
    REQUIRE( i2 == -63.0 );
    REQUIRE( i2.width == 10 );
    REQUIRE( i2.place == 0 );

    FixedPointComplex<8, -4> a3(5.25, -2.34);
    FixedPointComplex<5, -2> ref3(-1.23, 2.1);
    auto q3 = a3.quadrature(ref3);
    REQUIRE( q3 == - limit_precision(5.25, 8, -4) * limit_precision(2.1, 5, -2)
                   + limit_precision(-2.34, 8, -4) * limit_precision(-1.23, 5, -2) );
    REQUIRE( q3.width == 13 );
    REQUIRE( q3.place == -6 );
}

TEST_CASE( "mult_i()", "[FixedPointComplex]" ) {
    REQUIRE( FixedPointComplex<8, -4>(5.25, -2.34).mult_i()
             == FixedPointComplex<8, -4>(2.34, 5.25) );
    FixedPointComplex<8, -4> a(5.25, -2.34);
    REQUIRE( a.mult_i().width == 8 );
    REQUIRE( a.mult_i().place == -4 );
}

TEST_CASE( "ceil() (Complex)", "[FixedPointComplex]" ) {
    auto a(FixedPointComplex<8, -4>(1.23, -2.34).ceil());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -2.0) == a);
    REQUIRE( a.width == 5 );
    REQUIRE( a.place == 0 );
}

TEST_CASE( "floor() (Complex)", "[FixedPointComplex]" ) {
    auto a(FixedPointComplex<8, -4>(1.23, -2.34).floor());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -3.0) == a);
    REQUIRE( a.width == 5 );
    REQUIRE( a.place == 0 );
}

TEST_CASE( "trunc() (Complex)", "[FixedPointComplex]" ) {
    auto a(FixedPointComplex<8, -4>(1.23, -2.34).trunc());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0) == a);
    REQUIRE( a.width == 4 );
    REQUIRE( a.place == 0 );
}

TEST_CASE( "round_half_to_even() (Complex)", "[FixedPointComplex]" ) {
    auto a(FixedPointComplex<8, -4>(1.23, -2.34).round_half_to_even());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0) == a);
    REQUIRE( a.width == 5 );
    REQUIRE( a.place == 0 );
    auto b(FixedPointComplex<8, -4>(1.63, -2.64).round_half_to_even());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -3.0) == b);
    REQUIRE( b.width == 5 );
    REQUIRE( b.place == 0 );
    auto c(FixedPointComplex<8, -4>(1.5, -2.5).round_half_to_even());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -2.0) == c);
    REQUIRE( c.width == 5 );
    REQUIRE( c.place == 0 );
    auto d(FixedPointComplex<8, -4>(2.5, -3.5).round_half_to_even());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -4.0) == d);
    REQUIRE( d.width == 5 );
    REQUIRE( d.place == 0 );
}

TEST_CASE( "round_half_away_from_zero() (Complex)", "[FixedPointComplex]" ) {
    auto a(FixedPointComplex<8, -4>(1.23, -2.34).round_half_away_from_zero());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0) == a);
    REQUIRE( a.width == 5 );
    REQUIRE( a.place == 0 );
    auto b(FixedPointComplex<8, -4>(1.63, -2.64).round_half_away_from_zero());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -3.0) == b);
    REQUIRE( b.width == 5 );
    REQUIRE( b.place == 0 );
    auto c(FixedPointComplex<8, -4>(1.5, -2.5).round_half_away_from_zero());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -3.0) == c);
    REQUIRE( c.width == 5 );
    REQUIRE( c.place == 0 );
    auto d(FixedPointComplex<8, -4>(2.5, -3.5).round_half_away_from_zero());
    REQUIRE( FixedPointComplex<8, -4>(3.0, -4.0) == d);
    REQUIRE( d.width == 5 );
    REQUIRE( d.place == 0 );
}

TEST_CASE( "round_half_toward_zero() (Complex)", "[FixedPointComplex]" ) {
    auto a(FixedPointComplex<8, -4>(1.23, -2.34).round_half_toward_zero());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0) == a);
    REQUIRE( a.width == 5 );
    REQUIRE( a.place == 0 );
    auto b(FixedPointComplex<8, -4>(1.63, -2.64).round_half_toward_zero());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -3.0) == b);
    REQUIRE( b.width == 5 );
    REQUIRE( b.place == 0 );
    auto c(FixedPointComplex<8, -4>(1.5, -2.5).round_half_toward_zero());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0) == c);
    REQUIRE( c.width == 5 );
    REQUIRE( c.place == 0 );
    auto d(FixedPointComplex<8, -4>(2.5, -3.5).round_half_toward_zero());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -3.0) == d);
    REQUIRE( d.width == 5 );
    REQUIRE( d.place == 0 );
}

TEST_CASE( "round_half_up() (Complex)", "[FixedPointComplex]" ) {
    auto a(FixedPointComplex<8, -4>(1.23, -2.34).round_half_up());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0) == a);
    REQUIRE( a.width == 5 );
    REQUIRE( a.place == 0 );
    auto b(FixedPointComplex<8, -4>(1.63, -2.64).round_half_up());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -3.0) == b);
    REQUIRE( b.width == 5 );
    REQUIRE( b.place == 0 );
    auto c(FixedPointComplex<8, -4>(1.5, -2.5).round_half_up());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -2.0) == c);
    REQUIRE( c.width == 5 );
    REQUIRE( c.place == 0 );
    auto d(FixedPointComplex<8, -4>(2.5, -3.5).round_half_up());
    REQUIRE( FixedPointComplex<8, -4>(3.0, -3.0) == d);
    REQUIRE( d.width == 5 );
    REQUIRE( d.place == 0 );
}

TEST_CASE( "round_half_down() (Complex)", "[FixedPointComplex]" ) {
    auto a(FixedPointComplex<8, -4>(1.23, -2.34).round_half_down());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0) == a);
    REQUIRE( a.width == 5 );
    REQUIRE( a.place == 0 );
    auto b(FixedPointComplex<8, -4>(1.63, -2.64).round_half_down());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -3.0) == b);
    REQUIRE( b.width == 5 );
    REQUIRE( b.place == 0 );
    auto c(FixedPointComplex<8, -4>(1.5, -2.5).round_half_down());
    REQUIRE( FixedPointComplex<8, -4>(1.0, -3.0) == c);
    REQUIRE( c.width == 5 );
    REQUIRE( c.place == 0 );
    auto d(FixedPointComplex<8, -4>(2.5, -3.5).round_half_down());
    REQUIRE( FixedPointComplex<8, -4>(2.0, -4.0) == d);
    REQUIRE( d.width == 5 );
    REQUIRE( d.place == 0 );
}

TEST_CASE( "real() (get)", "[FixedPointComplex]" ) {
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0).real() == 1.0 );
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0).real().width == 8 );
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0).real().place == -4 );
}

TEST_CASE( "imag() (get)", "[FixedPointComplex]" ) {
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0).imag() == -2.0 );
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0).imag().width == 8 );
    REQUIRE( FixedPointComplex<8, -4>(1.0, -2.0).imag().place == -4 );
}

TEST_CASE( "real() (set FixedPoint)", "[FixedPointComplex]" ) {
    FixedPointComplex<8, -4> a(1.0, -2.0);
    a.real(FixedPoint<8, -4>(3.0));
    REQUIRE( a == FixedPointComplex<8, -4>(3.0, -2.0) );

    a.real(FixedPoint<7, -3>(3.0));
    REQUIRE( a == FixedPointComplex<8, -4>(3.0, -2.0) );
}

TEST_CASE( "imag() (set FixedPoint)", "[FixedPointComplex]" ) {
    FixedPointComplex<8, -4> a(1.0, -2.0);
    a.imag(FixedPoint<8, -4>(-1.5));
    REQUIRE( a == FixedPointComplex<8, -4>(1.0, -1.5) );

    a.imag(FixedPoint<7, -3>(-1.5));
    REQUIRE( a == FixedPointComplex<8, -4>(1.0, -1.5) );
}

TEST_CASE( "real() (set double)", "[FixedPointComplex]" ) {
    FixedPointComplex<8, -4> a(1.0, -2.0);
    a.real(3.1234);
    REQUIRE( a == FixedPointComplex<8, -4>(3.1234, -2.0) );
}

TEST_CASE( "imag() (set double)", "[FixedPointComplex]" ) {
    FixedPointComplex<8, -4> a(1.0, -2.0);
    a.imag(-1.5678);
    REQUIRE( a == FixedPointComplex<8, -4>(1.0, -1.5678) );
}

TEST_CASE( "real() (get non-member)", "[FixedPointComplex]" ) {
    REQUIRE( real(FixedPointComplex<8, -4>(1.0, -2.0)) == 1.0 );
    REQUIRE( real(FixedPointComplex<8, -4>(1.0, -2.0)).width == 8 );
    REQUIRE( real(FixedPointComplex<8, -4>(1.0, -2.0)).place == -4 );
}

TEST_CASE( "imag() (get non-member)", "[FixedPointComplex]" ) {
    REQUIRE( imag(FixedPointComplex<8, -4>(1.0, -2.0)) == -2.0 );
    REQUIRE( imag(FixedPointComplex<8, -4>(1.0, -2.0)).width == 8 );
    REQUIRE( imag(FixedPointComplex<8, -4>(1.0, -2.0)).place == -4 );
}

TEST_CASE( "norm() (non-member)", "[FixedPointComplex]" ) {
    REQUIRE( norm(FixedPointComplex<8, -4>(5.25, -2.34))
             == limit_precision(5.25, 8, -4) * limit_precision(5.25, 8, -4)
                + limit_precision(-2.34, 8, -4) * limit_precision(-2.34, 8, -4) );
    FixedPointComplex<8, -4> a(5.25, -2.34);
    REQUIRE( norm(a).width == 16 );
    REQUIRE( norm(a).place == -8 );
}
