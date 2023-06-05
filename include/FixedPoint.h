/** FixedPoint class
    \file FixedPoint.h
    \author OOTA, Masato
    \copyright Copyright © 2022 OOTA, Masato
    \par License GPL-3.0-or-later
    \parblock
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
    \endparblock
*/

#ifndef PREC_CTRL_FIXEDPOINT_H_
#define PREC_CTRL_FIXEDPOINT_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>

#include "FixedPoint/limit.h"
#include "FixedPoint/significand.h"

namespace prec_ctrl {
    /** A fixed point number class whose maximum precision is the same as double.
        \tparam WIDTH The bit width of the significand, including the sign bit and the hidden bit of double.
        \tparam PLACE The LSB place whose \#n has the weight 2**n.

        FixedPoint can multiply, add, and subtract exactly. The precision and the place is automatically adjusted. Convert a value into double if you would use divide operator because they cannot determine a precision without some instructions from the programmer.

        The result of the functions is the exact value unless the description of the function notes about rounding or clamping, however FixedPoint must contain less than or equal the range of double. If you try to create an instance has more than double precision, a compilation error will be caused. The reason why FixedPoint is restricted to double precision is to make exact conversion FixedPoint to double.

        FixedPoint causes a same result as double operation when it limits the precision in the same manner. FixedPoint shows you the precision required a operation, so you may easily know the maximum required precision of the same operation with double.

        \verbatim
           For example of WIDTH = 8, PLACE = -3:
           Place#   4   3   2   1   0  -1  -2  -3
           Weight  16   8   4   2   1  1/2 1/4 1/8
                  +---+---+---+---+---+---+---+---+
                  | S |   |   |   |   |   |   |   |
                  +---+---+---+---+---+---+---+---+
                                      ^
                               decimal point
                    S: sign bit

           Maximum: +15.875
           Minimum: -15.875
           Resolution: 0.125
        \endverbatim
    */
    template<int WIDTH, int PLACE>
    class FixedPoint {
        static_assert(MIN_BIT_WIDTH <= WIDTH, "");
        static_assert(WIDTH <= MAX_BIT_WIDTH, "");
        static_assert(MIN_LSB_PLACE <= PLACE, "");
        static_assert(WIDTH + PLACE <= MAX_MSB_PLACE, "");

        template<int PR, int PL>
        friend class FixedPoint;


    public:
        /** The computation of a bit width for superset_t and addtion_result_t.
            \param [in] width2 Another WIDTH
            \param [in] place2 Another PLACE
            \param [in] extra_width The extra bits on the MSB side.
            \return The sufficient width.
         */
        static constexpr int superset_width(int width2, int place2, int extra_width = 0) noexcept
        {
            return std::max(WIDTH + PLACE, width2 + place2) - std::min(PLACE, place2) + extra_width;
        }

        /** The instance of this type can contain the value of both FixedPoint types.
            \tparam WIDTH2 Another WIDTH
            \tparam PLACE2 Another PLACE
            \tparam EXTRA_WIDTH The extra bits on the MSB side.

            The type can be automatically converted from both FixedPoint<WIDTH, PLACE> and FixedPoint<WIDTH2, PLACE2>, and also it has EXTRA_WIDTH bits as the extra bits on the MSB side.
        */
        template<int WIDTH2, int PLACE2, int EXTRA_WIDTH = 0>
        using superset_t
            = FixedPoint<superset_width(WIDTH2, PLACE2, EXTRA_WIDTH),
                         std::min(PLACE, PLACE2)>;

        /** The instance of this type can contain the result of the addition and the subtraction.
            \tparam WIDTH2 Another WIDTH
            \tparam PLACE2 Another PLACE

            Basically, the result of the addition and the subtraction needs one more bit, but it's no need in case the bit range (without the sign bits) of the operands isn't overlapped.
        */
        template<int WIDTH2, int PLACE2>
        using addition_result_t
            = FixedPoint<superset_width(WIDTH2, PLACE2, (PLACE >= WIDTH2 + PLACE2 - 1 || PLACE2 >= WIDTH + PLACE - 1) ? 0 : 1),
                         std::min(PLACE, PLACE2)>;

        /** The type can contain the integer part of this type.
            \tparam EXTRA_WIDTH the extra bits on the MSB side.

            The type can contain the integer part of this type, and also it has EXTRA_WIDTH bits as the extra bits on the MSB side. If this type has no integer part, the type is at least a 2-bit integer because the minimum WIDTH is 2. If this type has only an integer part, the result is this type.
        */
        template<int EXTRA_WIDTH>
        using integer_part_t
            = FixedPoint<(PLACE >= 0) ?
                         WIDTH :
                         (WIDTH + PLACE <= 1) ?
                             std::max(2, 1 + EXTRA_WIDTH) :
                             WIDTH + PLACE + EXTRA_WIDTH,
                         (PLACE >= 0) ? PLACE : 0>;

    public:
        /* Constructors, Assignments, and conversions */
        /** Default constructor to create zero. */
        constexpr FixedPoint() noexcept : significand(0) {}
        /** Default copy constructor */
        constexpr FixedPoint(const FixedPoint &src) noexcept = default;
        /** Default copy operator */
        constexpr FixedPoint &operator=(const FixedPoint &src) noexcept = default;
        /** Default destructor */
#ifdef __cpp_constexpr_dynamic_alloc
        // for C++20
        constexpr
#endif
        ~FixedPoint() noexcept = default;

        /** Clone from a narrower FixedPoint.
            \tparam WIDTH_SRC The bit width of src.
            \tparam PLACE_SRC The place of src.
            \param [in] src The source value.
            \note "Narrower" means that the MSB is less than or equal this type's and the LSB is more than or equal this type's.
            \warning A compilation error is caused if the type of src is wider than this type.
            \attention FixedPoint has no conversion from wider FixedPoint because it can be inexact. Use conversion from double.
         */
        template<int WIDTH_SRC, int PLACE_SRC>
        constexpr explicit FixedPoint(const FixedPoint<WIDTH_SRC, PLACE_SRC> &src) noexcept
            : significand(static_cast<decltype(significand)>(src.significand) << (PLACE_SRC - PLACE))
        {
            static_assert(PLACE <= PLACE_SRC
                          && WIDTH + PLACE >= WIDTH_SRC + PLACE_SRC,
                          "The type of the source must be narrower FixedPoint.");
        }

        /** Assignment of a narrower FixedPoint.
            \tparam WIDTH_SRC The bit width of src.
            \tparam PLACE_SRC The place of src.
            \param [in] src the source value.
            \return this
            \note "Narrower" means that the MSB is less than or equal this type's and the LSB is more than or equal this type's.
            \warning A compilation error is caused if the type of src is wider than this type.
            \attention FixedPoint has no conversion from wider FixedPoint because it can be inexact. Use conversion from double.
         */
        template<int WIDTH_SRC, int PLACE_SRC>
        constexpr FixedPoint<WIDTH, PLACE> &operator=(const FixedPoint<WIDTH_SRC, PLACE_SRC> &src) noexcept
        {
            const FixedPoint<WIDTH, PLACE> src_ext(src);
            significand = src_ext.significand;
            return *this;
        }

        /** Create an instance has the reduced dynamic range.
            \tparam DEST_WIDTH The bit width of the return value. (In fact, DEST_WIDTH can be more than WIDTH.)
            \return The value reduced dynamic range.

            The same value is returned if the return type can hold it, otherwise the clamped value is returned.
        */
        template<int DEST_WIDTH>
        constexpr FixedPoint<DEST_WIDTH, PLACE> reduce_dynamic_range() const noexcept
        {
            FixedPoint<DEST_WIDTH, PLACE> result;
            result.significand = clamp<DEST_WIDTH>(significand);
            return result;
        }

        /** Clone from a double.
            \param [in] src The source value.
            \note The value may be implicitly rounded and clamped in the same manner of limit_precision().
        */
        explicit FixedPoint(double src)
            : significand(to_significand<WIDTH, PLACE>(src))
        {}

        /** Convert a value into double.
            \return The value as double.
            \note The conversion isn't explicit because the new value is exactly equal to this.
        */
#if __cplusplus > 202002L
        // for C++23
        constexpr
#endif
        operator double() const
        {
            return std::ldexp(significand, PLACE);
        }

        /** Convert into bool.
            \return true if this is not zero.
        */
        explicit constexpr operator bool() const noexcept
        {
            return significand != 0;
        }

        /* Unary Operators */
        /** Unary + operator.
            \return The same value of this.
         */
        constexpr FixedPoint<WIDTH, PLACE> operator+() const noexcept
        {
            return *this;
        }

        /** Unary - operator.
            \return The value multiplied by -1.
            \note This operator never causes overflow.
         */
        constexpr FixedPoint<WIDTH, PLACE> operator-() const noexcept
        {
            FixedPoint<WIDTH, PLACE> result;
            result.significand = -significand;
            return result;
        }

        /** logical not operator.
            \return true if this is zero.
        */
        constexpr bool operator!() const noexcept
        {
            return significand == 0;
        }


        /* Binary operators */
        /** Binary + operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return this + op2
            \note Basically, the return type is one more bit wider.
        */
        template<int WIDTH2, int PLACE2>
        constexpr addition_result_t<WIDTH2, PLACE2>
        operator+(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            addition_result_t<WIDTH2, PLACE2> result(*this);
            const decltype(result) op2_ext(op2);
            result.significand += op2_ext.significand;
            return result;
        }

        /** Binary - operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return this - op2
            \note Basically, the return type is one more bit wider.
        */
        template<int WIDTH2, int PLACE2>
        constexpr addition_result_t<WIDTH2, PLACE2>
        operator-(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            addition_result_t<WIDTH2, PLACE2> result(*this);
            const decltype(result) op2_ext(op2);
            result.significand -= op2_ext.significand;
            return result;
        }

        /** Binary * operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return this * op2
        */
        template<int WIDTH2, int PLACE2>
        constexpr FixedPoint<WIDTH + WIDTH2 - 1, PLACE + PLACE2>
        operator*(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            FixedPoint<WIDTH + WIDTH2 - 1, PLACE + PLACE2> result;
            using result_t = decltype(result.significand);
            const result_t significand1 = significand;
            const result_t significand2 = op2.significand;
            result.significand = significand1 * significand2;
            return result;
        }


        /* Relational operators */
        /** Relational == operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return true if The value of this is equal to the value of op2.
        */
        template<int WIDTH2, int PLACE2>
        constexpr bool operator==(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            const superset_t<WIDTH2, PLACE2> op1(*this);
            const decltype(op1) op2_ext(op2);
            return op1.significand == op2_ext.significand;
        }

        /** Relational != operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return true if The value of this is not equal to the value of op2.
        */
        template<int WIDTH2, int PLACE2>
        constexpr bool operator!=(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            const superset_t<WIDTH2, PLACE2> op1(*this);
            const decltype(op1) op2_ext(op2);
            return op1.significand != op2_ext.significand;
        }

        /** Relational < operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return true if The value of this is less than the value of op2.
        */
        template<int WIDTH2, int PLACE2>
        constexpr bool operator<(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            const superset_t<WIDTH2, PLACE2> op1(*this);
            const decltype(op1) op2_ext(op2);
            return op1.significand < op2_ext.significand;
        }

        /** Relational <= operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return true if The value of this is less than or equal the value of op2.
        */
        template<int WIDTH2, int PLACE2>
        constexpr bool operator<=(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            const superset_t<WIDTH2, PLACE2> op1(*this);
            const decltype(op1) op2_ext(op2);
            return op1.significand <= op2_ext.significand;
        }

        /** Relational > operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return true if The value of this is greater than the value of op2.
        */
        template<int WIDTH2, int PLACE2>
        constexpr bool operator>(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            const superset_t<WIDTH2, PLACE2> op1(*this);
            const decltype(op1) op2_ext(op2);
            return op1.significand > op2_ext.significand;
        }

        /** Relational >= operator.
            \tparam WIDTH2 The bit width of op2.
            \tparam PLACE2 The place of op2.
            \param [in] op2 The second operand.
            \return true if The value of this is greater than or equal the value of op2.
        */
        template<int WIDTH2, int PLACE2>
        constexpr bool operator>=(const FixedPoint<WIDTH2, PLACE2> &op2) const noexcept
        {
            const superset_t<WIDTH2, PLACE2> op1(*this);
            const decltype(op1) op2_ext(op2);
            return op1.significand >= op2_ext.significand;
        }


        /* Math functions */
        /** exp2 function.
            \tparam EXP The exponent of 2.
            \return this * 2**EXP
        */
        template<int EXP>
        constexpr FixedPoint<WIDTH, PLACE + EXP> exp2() const noexcept
        {
            FixedPoint<WIDTH, PLACE + EXP> result;
            result.significand = significand;
            return result;
        }


        /** ceil function.
            \return The nearest integer away from zero of this.
            \note This function may cause overflow, so it increases one bit than the integer part of this type.
        */
        constexpr integer_part_t<1> ceil() const noexcept
        {
            integer_part_t<1> result;
            if constexpr (PLACE >= 0) {
                // this is an integer.
                result.significand = significand;
            } else {
                constexpr auto FRAC_MASK = MAX_SIGNIFICAND_VALUE<1 - PLACE>;
                const significand_t<((WIDTH + PLACE) <= 1) ? 2 - PLACE : WIDTH + 1>
                    significand_ext = significand;
                result.significand = (significand_ext + FRAC_MASK) >> -PLACE;
            }
            return result;
        }

        /** floor function.
            \return The nearest integer toward zero of this.
            \note This function may cause overflow, so it increases one bit than the integer part of this type.
        */
        constexpr integer_part_t<1> floor() const noexcept
        {
            integer_part_t<1> result;
            if constexpr (PLACE >= 0) {
                // this is an integer.
                result.significand = significand;
            } else {
                const significand_t<((WIDTH + PLACE) <= 1) ? 2 - PLACE : WIDTH>
                    significand_ext = significand;
                result.significand = significand_ext >> -PLACE;
            }
            return result;
        }

        /** Round to nearest, ties to even.
            \return The nearest integer of this, basically.
            \note This function may cause overflow, so it increases one bit than the integer part of this type.

            This function convert a value to the nearest integer. It converts this to nearest even integer if the value falls midway. It seems to be recommended by IEEE-745.
        */
        constexpr integer_part_t<1> round_half_to_even() const noexcept
        {
            integer_part_t<1> result;
            if constexpr (PLACE >= 0) {
                // this is an integer.
                result.significand = significand;
            } else {
                constexpr auto HALF_MINUS_1 = MAX_SIGNIFICAND_VALUE<-PLACE>;
                const significand_t<((WIDTH + PLACE) <= 1) ? 2 - PLACE : WIDTH + 1>
                    significand_ext = significand;
                const auto int_lsb = (significand_ext >> -PLACE) & 1;
                result.significand = (significand_ext + HALF_MINUS_1 + int_lsb) >> -PLACE;
            }
            return result;
        }

        /** Round to nearest, ties away from zero.
            \return The nearest integer of this, basically.
            \note This function may cause overflow, so it increases one bit than the integer part of this type.

            This function convert a value to the nearest integer. It converts this to nearest integer away from zero if the value falls midway.
        */
        constexpr integer_part_t<1> round_half_away_from_zero() const noexcept
        {
            integer_part_t<1> result;
            if constexpr (PLACE >= 0) {
                // this is an integer.
                result.significand = significand;
            } else {
                constexpr auto HALF = static_cast<significand_t<WIDTH>>(1) << (-PLACE - 1);
                const significand_t<((WIDTH + PLACE) <= 1) ? 2 - PLACE : WIDTH + 1>
                    significand_ext = significand;
                const auto sign_bit = (significand_ext >> (WIDTH - 1)) & 1;
                result.significand = (significand_ext + HALF - sign_bit) >> -PLACE;
            }
            return result;
        }

        /** Round to nearest, ties to zero.
            \return The nearest integer of this, basically.
            \note This function may cause overflow, so it increases one bit than the integer part of this type.

            This function convert a value to the nearest integer. It converts this to nearest integer toward zero if the value falls midway.
        */
        constexpr integer_part_t<1> round_half_toward_zero() const noexcept
        {
            integer_part_t<1> result;
            if constexpr (PLACE >= 0) {
                // this is an integer.
                result.significand = significand;
            } else {
                constexpr auto HALF = static_cast<significand_t<WIDTH>>(1) << (-PLACE - 1);
                const significand_t<((WIDTH + PLACE) <= 1) ? 2 - PLACE : WIDTH + 1>
                    significand_ext = significand;
                const auto sign_bit = (significand_ext >> (WIDTH - 1)) & 1;
                result.significand = (significand_ext + HALF - (sign_bit ^ 1)) >> -PLACE;
            }
            return result;
        }

        /** Round to nearest, ties to upper.
            \return The nearest integer of this, basically.
            \note This function may cause overflow, so it increases one bit than the integer part of this type.

            This function convert a value to the nearest integer. It converts this to nearest upper integer if the value falls midway.
        */
        constexpr integer_part_t<1> round_half_up() const noexcept
        {
            integer_part_t<1> result;
            if constexpr (PLACE >= 0) {
                // this is an integer.
                result.significand = significand;
            } else {
                constexpr auto HALF = static_cast<significand_t<WIDTH>>(1) << (-PLACE - 1);
                const significand_t<((WIDTH + PLACE) <= 1) ? 2 - PLACE : WIDTH + 1>
                    significand_ext = significand;
                result.significand = (significand_ext + HALF) >> -PLACE;
            }
            return result;
        }

        /** Round to nearest, ties to lower.
            \return The nearest integer of this, basically.
            \note This function may cause overflow, so it increases one bit than the integer part of this type.

            This function convert a value to the nearest integer. It converts this to nearest lower integer if the value falls midway.
        */
        constexpr integer_part_t<1> round_half_down() const noexcept
        {
            integer_part_t<1> result;
            if constexpr (PLACE >= 0) {
                // this is an integer.
                result.significand = significand;
            } else {
                constexpr auto HALF_MINUS_1 = MAX_SIGNIFICAND_VALUE<-PLACE>;
                const significand_t<((WIDTH + PLACE) <= 1) ? 2 - PLACE : WIDTH + 1>
                    significand_ext = significand;
                result.significand = (significand_ext + HALF_MINUS_1) >> -PLACE;
            }
            return result;
        }


        /* Property access */
        /** Get the value of the significand.
            \return significand.
        */
        constexpr significand_t<WIDTH> get_significand() const noexcept
        {
            return significand;
        }

        /** Set the value of the significand.
            \param [in] i The value tried to set into significand.
            \return this
            \note i is clamped if it's out of range.
        */
        constexpr FixedPoint &set_significand(const significand_t<WIDTH> i) noexcept
        {
            significand = clamp<WIDTH>(i);
            return *this;
        }

        /** Get the bit width.
            \return The bit width of the significand, including the hidden bit and the sign bit of double.
        */
        constexpr int get_width() const noexcept
        {
            return WIDTH;
        }

        /** Get the place.
            \return The place of LSB. The place\#n has the weight 2**n.
        */
        constexpr int get_place() const noexcept
        {
            return PLACE;
        }


    private:
        /** Forbid to clone from an integer type.
            \note Because it may make easy mistake the assignment of a significand for the assignment of an integer value.
        */
        constexpr FixedPoint(std::intmax_t a) noexcept = delete;
        /** Forbid to clone from an integer type.
            \note Because it may make easy mistake the assignment of a significand for the assignment of an integer value.
        */
        constexpr FixedPoint(std::uintmax_t a) noexcept = delete;


    private:
        significand_t<WIDTH> significand;
    };
}

#endif // PREC_CTRL_FIXEDPOINT_H_
