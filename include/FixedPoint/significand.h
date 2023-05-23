/** significand_t type
    \file FixedPoint/significand.h
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

#ifndef PREC_CTRL_SIGNIFICAND_H_
#define PREC_CTRL_SIGNIFICAND_H_

#include <cstdint>
#include <limits>
#include <type_traits>

namespace prec_ctrl {
    /** The signed integer type of the significand.
        \tparam WIDTH The required width.

        The signed integer type of the significand, including the hidden bit and the sign bit of double.
    */
    template<int WIDTH>
    using significand_t = std::conditional_t<(WIDTH > 32), std::int_fast64_t, std::int_fast32_t>;
    /** The unsigned integer type of the significand.
        \tparam WIDTH The required width.

        The unsigned integer type of the significand, including the hidden bit and the sign bit of double.
    */
    template<int WIDTH>
    using u_significand_t = std::make_unsigned_t<significand_t<WIDTH>>;

    /** The minimum bit width of significand_t.
        \note significand_t needs the sign bit and 1 bit at least.
        \note The width is always a positive number, but the type of the width is signed int to avoid bored casts to signed int when calculating something from a width and a place. It's not much problem because a compilation error is caused if you try to use a width less than MIN_BIT_WIDTH.
    */
    constexpr int MIN_BIT_WIDTH = 2;

    /** The maximum bit width of significand_t.
        \note This means that FixedPoint can exactly convert to double.
    */
#ifndef NO_NEED_TO_CONVERT_TO_DOUBLE_EXACTLY
    // +1 as the sign bit.
    constexpr int MAX_BIT_WIDTH = std::numeric_limits<double>::digits + 1;
#else
    constexpr int MAX_BIT_WIDTH = 64;
#endif

    static_assert(std::numeric_limits<u_significand_t<MAX_BIT_WIDTH>>::digits >= MAX_BIT_WIDTH, "(u)significand_t must be able to contain the significand, hidden, and sign bit of double.");


    /** The maximum value of the significand.
        \tparam WIDTH The required width.
    */
    template<int WIDTH>
    constexpr significand_t<WIDTH> MAX_SIGNIFICAND_VALUE
        = (static_cast<u_significand_t<WIDTH>>(1) << (WIDTH - 1)) - 1;

    /** The minimum value of the significand.
        \tparam WIDTH The required width.
        \note The minimum value is -MAX_SIGNIFICAND_VALUE. -MAX_SIGNIFICAND_VALUE - 1 is not used. It means the unary minus operator won't cause an overflow.
    */
    template<int WIDTH>
    constexpr significand_t<WIDTH> MIN_SIGNIFICAND_VALUE
        = -MAX_SIGNIFICAND_VALUE<WIDTH>;
}

#endif // PREC_CTRL_SIGNIFICAND_H_
