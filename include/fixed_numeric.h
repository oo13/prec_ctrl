/** Numerical functions for FixedPoint.
    \file fixed_numeric.h
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

    For convenience, this library provides some functions for std::accumulate() and you can specify the precision of the accumulation.

    FixedPoint::operator+() automatically determines the precision of the result, but:
    - It cannot use for std::accumulation() because FixedPoint::operator+() returns the value whose type is the different from the type of the input.
    - Some programmers can use the knowledge of the application and determine to reduce the required precision.

    The functions don't use FixedPoint as the accumulator, so you should recover the value of FixedPoint after you get the last result of the accumulation.

    For example:
    \code
      std::vector<FixedPoint<16, -8>> a;
      ...
      auto tmp = std::accumulate(a.begin(), a.end(), significand_t<20>(0), int_adder<20, 16, -8>);
      FixedPoint<20, -8> sum;
      sum.set_significand(tmp); // This may cause to clamp tmp.
    \endcode
*/

#ifndef PREC_CTRL_ACCUMULATE_H_
#define PREC_CTRL_ACCUMULATE_H_

#include <stdexcept>

#include "FixedPoint.h"

namespace prec_ctrl {
    /** A function adding an accumulation and a significand by + operator.
        \tparam T The type of accumulator.
        \tparam WIDTH The bit width of the increment.
        \tparam PLACE The place of the accumulator and the increment.
        \param [in] accumulator The accumulated value.
        \param [in] increment The value of increment.
        \return accumulator + increment.get_significand()

        This function is an adder for the significand. The way to add two operands depends on the operator +.
    */
    template<typename T, int WIDTH, int PLACE>
    constexpr T
    significand_adder(const T accumulator,
                      const FixedPoint<WIDTH, PLACE> &increment) noexcept
    {
        return accumulator + increment.get_significand();
    }

    /** A binary integer adder for accumulate.
        \tparam WIDTH1 The bit width of the accumulator.
        \tparam WIDTH2 The bit width of the increment.
        \tparam PLACE The place of the accumulator and the increment.
        \param [in] accumulator The accumulated value.
        \param [in] increment The value of increment.
        \return The sum of accumulator and increment.get_significand() by a signed integer adder that has just WIDTH1 bit width.
        \note The result can be -2**(WIDTH1 - 1), which is not accepted by FixedPoint<WIDTH1, any_place>.

        This function is a binary integer adder that has just the specified bit width. It causes wraparound when overflowing. This function assumes the negative number is expressed 2's complement.
    */
    template<int WIDTH1, int WIDTH2, int PLACE>
    constexpr significand_t<WIDTH1>
    int_adder(const significand_t<WIDTH1> accumulator,
              const FixedPoint<WIDTH2, PLACE> &increment) noexcept
    {
        static_assert(WIDTH1 >= WIDTH2, "");
        auto result = accumulator + increment.get_significand();
        // sign extend, so it means wraparound.
        constexpr u_significand_t<WIDTH1> UMASK
            = static_cast<u_significand_t<WIDTH1>>(MAX_SIGNIFICAND_VALUE<WIDTH1>);
        if ((result & (UMASK + 1)) == 0) {
            result &= UMASK;
        } else {
            result |= ~UMASK;
        }
        return result;
    }

    /** An exact adder for accumulate.
        \tparam WIDTH1 The bit width of the accumulator.
        \tparam WIDTH2 The bit width of the increment.
        \tparam PLACE The place of the accumulator and the increment.
        \param [in] accumulator The accumulated value.
        \param [in] increment The value of increment.
        \exception std::range_error The result would be inexact.
        \return The exact sum of accumulator and increment.get_significand().
        \attention You want to use this function when you are certain it does not cause overflow.

        This adder returns the exact result, however, raises an exception when overflowing.
    */
    template<int WIDTH1, int WIDTH2, int PLACE>
    significand_t<WIDTH1>
    exact_adder(const significand_t<WIDTH1> accumulator,
                const FixedPoint<WIDTH2, PLACE> &increment)
    {
        static_assert(WIDTH1 >= WIDTH2, "");
        // result has the exact value because it has WIDTH1 + 1 bits.
        const significand_t<WIDTH1 + 1> result
            = accumulator + increment.get_significand();
        constexpr u_significand_t<WIDTH1 + 1> SIGN_MASK
            = ~static_cast<u_significand_t<WIDTH1 + 1>>(MAX_SIGNIFICAND_VALUE<WIDTH1>);
        if ((result & SIGN_MASK) != 0 && (result & SIGN_MASK) != SIGN_MASK) {
            throw std::range_error("overflow");
        }
        return result;
    }

    /** An adder with clamping for accumulate.
        \tparam WIDTH1 The bit width of the accumulator.
        \tparam WIDTH2 The bit width of the increment.
        \tparam PLACE The place of the accumulator and the increment.
        \param [in] accumulator The accumulated value.
        \param [in] increment The value of increment.
        \return The clamped sum of accumulator and increment.get_significand().
        \attention The last result is a junk if the accumulation causes clamping at a point, unless all input values have a same sign.

        This adder clamps the result if it's out of range. It means that if the sum is more than  +(2 ** (WIDTH1 - 1) - 1) the result is +(2 ** (WIDTH1 - 1) - 1), if the sum is less than  -(2 ** (WIDTH1 - 1) - 1) the result is -(2 ** (WIDTH1 - 1) - 1).
    */
    template<int WIDTH1, int WIDTH2, int PLACE>
    constexpr significand_t<WIDTH1>
    clamp_adder(const significand_t<WIDTH1> accumulator,
                const FixedPoint<WIDTH2, PLACE> &increment) noexcept
    {
        static_assert(WIDTH1 >= WIDTH2, "");
        // result has the exact value because it has WIDTH1 + 1 bits.
        const significand_t<WIDTH1 + 1> result
            = accumulator + increment.get_significand();
        return clamp<WIDTH1>(result);
    }
}

#endif // PREC_CTRL_ACCUMULATE_H_
