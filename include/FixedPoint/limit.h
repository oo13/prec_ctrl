/** Clamping and converting functions for significand_t.
    \file FixedPoint/limit.h
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

#ifndef PREC_CTRL_LIMIT_H_
#define PREC_CTRL_LIMIT_H_

#include <limits>
#include <cmath>

#include "FixedPoint/significand.h"

namespace prec_ctrl {
    /** Lowest place of LSB. */
    // min/max_exponent means the value can contain 2**(exponent-1), so -1 is needed.
    constexpr int MIN_LSB_PLACE = std::numeric_limits<double>::min_exponent - 1;
    /** Highest place of MSB. */
    // +1 for the sign bit.
    constexpr int MAX_MSB_PLACE = std::numeric_limits<double>::max_exponent;


    /** Clamp the value when it's out of range.
        \tparam WIDTH The bit width of the significand, including the hidden bit and the sign bit of double.
        \param [in] i The source value.
        \return The clamped value.
    */
    template<int WIDTH>
    constexpr significand_t<WIDTH> clamp(significand_t<WIDTH> i)
    {
        static_assert(MIN_BIT_WIDTH <= WIDTH, "");
        static_assert(WIDTH <= MAX_BIT_WIDTH, "");
        if (i > MAX_SIGNIFICAND_VALUE<WIDTH>) {
            return MAX_SIGNIFICAND_VALUE<WIDTH>;
        } else if (i < MIN_SIGNIFICAND_VALUE<WIDTH>) {
            return MIN_SIGNIFICAND_VALUE<WIDTH>;
        } else {
            return i;
        }
    }

    /** Convert a double value into the significand.
        \tparam WIDTH The bit width of the significand, including the hidden bit and the sign bit of double.
        \tparam PLACE The LSB place whose \#n has the weight 2**n.
        \param [in] a The source value.
        \return The significand of the rounded and clamped value of the source.
        \note The standard rounding method is used. (cf. std::fesetround())

        This function converts a double value into the significand. The double value may be rounded and clamped if it cannot represent exactly in WIDTH and PLACE.
    */
    template<int WIDTH, int PLACE>
    significand_t<WIDTH> to_significand(double a)
    {
        static_assert(MIN_LSB_PLACE <= PLACE, "");
        static_assert(WIDTH + PLACE <= MAX_MSB_PLACE, "");
        return clamp<WIDTH>(std::nearbyint(a * std::exp2(-PLACE)));
    }
}

#endif // PREC_CTRL_LIMIT_H_
