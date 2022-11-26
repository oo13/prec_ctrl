/** A function to limit the precision for double.
    \file double_limit.h
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

#ifndef PREC_CTRL_DOUBLE_LIMIT_H_
#define PREC_CTRL_DOUBLE_LIMIT_H_

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

namespace prec_ctrl {
    static_assert(std::numeric_limits<double>::radix == 2, "The radix of double must be 2.");

    /** The function to limit the precision of a double value.
        \param [in] a The source value.
        \param [in] width The bit width of the significand, including the hidden bit and the sign bit of double.
        \param [in] place The LSB place whose \#n has the weight 2**n.
        \return The limited value.
        \note The standard rounding method is used. (cf. std::fesetround())

        This function rounds and clamps a double value to limit the precision.

        In this library, a binary place\#n has the weight 2**n.

        \verbatim
           For example of width = 8, place = -3:
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
    inline double limit_precision(double a, int width, int place)
    {
        // rounding
#ifndef NDEBUG
        constexpr int DBL_MIN_LSB_PLACE = std::numeric_limits<double>::min_exponent - 1;
        constexpr int DBL_MAX_MSB_PLACE = std::numeric_limits<double>::max_exponent;
        assert(DBL_MIN_LSB_PLACE <= place);
        assert(width + place <= DBL_MAX_MSB_PLACE);
#endif

        a = std::nearbyint(a * std::exp2(-place));

        // clmaping
#ifndef NDEBUG
        constexpr int DBL_MAX_BIT_WIDTH = std::numeric_limits<double>::digits + 1;
        constexpr int DBL_MIN_BIT_WIDTH = 2;
        assert(DBL_MIN_BIT_WIDTH <= width);
        assert(width <= DBL_MAX_BIT_WIDTH);
#endif
        const int_fast64_t MAX_VALUE
            = (static_cast<uint_fast64_t>(1) << (width - 1)) - 1;
        assert(width <= std::numeric_limits<decltype(MAX_VALUE)>::digits);
        if (a > MAX_VALUE) {
            a = MAX_VALUE;
        } else if (a < -MAX_VALUE) {
            a = -MAX_VALUE;
        }
        return a * std::exp2(place);
    }
}

#endif // PREC_CTRL_DOUBLE_LIMIT_H_
