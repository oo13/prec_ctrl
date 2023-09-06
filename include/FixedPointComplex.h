/** FixedPointComplex class
    \file FixedPointComplex.h
    \author OOTA, Masato
    \copyright Copyright © 2023 OOTA, Masato
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

#ifndef PREC_CTRL_FIXEDPOINTCOMPLEX_H_
#define PREC_CTRL_FIXEDPOINTCOMPLEX_H_

#include <complex>
#include <type_traits>
#include "FixedPoint.h"

namespace prec_ctrl {
    /** A fixed point complex number class using class FixedPoint.
        \tparam T A type of FixedPoint.
    */
    template<typename T>
    class Complex {
        static_assert(std::is_same_v<T, FixedPoint<T::width, T::place>>, "T must be a class FixedPoint<WIDTH, PLACE>.");

    public:
        /** The type of the real and imaginary parts. */
        using value_t = T;
        /** The bit width of value_t. */
        static constexpr int width = T::width;
        /** The place of value_t. */
        static constexpr int place = T::place;

        /** Default constructor to create zero. */
        constexpr Complex() noexcept = default;
        /** Default copy constructor */
        constexpr Complex(const Complex& src) noexcept = default;
        /** Default copy operator */
        constexpr Complex &operator=(const Complex &src) noexcept = default;

        /** Default destructor */
#ifdef __cpp_constexpr_dynamic_alloc
        // for C++20
        constexpr
#endif
        ~Complex() noexcept = default;

        /** Construct out of FixedPoint.
            \param [in] re The real part.
            \param [in] im The imaginary part.
         */
        constexpr Complex(const T& re, const T& im = T()) noexcept
            : re_(re), im_(im)
        {}

        /** Construct out of narrower FixedPoint.
            \tparam T1 A narrower FixedPoint for the real part.
            \tparam T2 A narrower FixedPoint for the imaginary part.
            \param [in] re The real part.
            \param [in] im The imaginary part.
            \note "Narrower" means that the MSB is less than or equal this type's and the LSB is more than or equal this type's.
            \warning A compilation error is caused if the type of src is wider than this type.
            \attention FixedPoint has no conversion from wider FixedPoint because it can be inexact. Use conversion from double.
         */
        template<typename T1, typename T2>
        constexpr explicit Complex(const T1& re, const T2& im) noexcept
            : re_(re), im_(im)
        {}

        /** Clone from a narrower Complex.
            \tparam T2 A narrower Complex.
            \param [in] src The source value.
            \note "Narrower" means that the MSB is less than or equal this type's and the LSB is more than or equal this type's.
            \warning A compilation error is caused if the type of src is wider than this type.
            \attention Complex has no conversion from wider Complex because it can be inexact. Use conversion from std::complex<double>.
         */
        template<typename T2>
        constexpr explicit Complex(const Complex<T2> &src) noexcept
            : re_(src.real()), im_(src.imag())
        {}

        /** Assignment of a narrower Complex.
            \tparam T2 A narrower Complex.
            \param [in] src The source value.
            \return this
            \note "Narrower" means that the MSB is less than or equal this type's and the LSB is more than or equal this type's.
            \warning A compilation error is caused if the type of src is wider than this type.
            \attention Complex has no conversion from wider Complex because it can be inexact. Use conversion from std::complex<double>.
         */
        template<typename T2>
        constexpr Complex<T> &operator=(const Complex<T2> &src) noexcept
        {
            re_ = src.real();
            im_ = src.imag();
            return *this;
        }

    private:
        /** Apply a member function of FixedPoint to the real and imaginary parts.
            \tparam MF A type of a pointer to a member function of FixedPoint.
            \param [in] mf A pointer to a member function of FixedPoint.
            \return The complex number that consists of the real and imaginary parts that are the result of mf.
         */
        template<typename MF>
        constexpr auto apply_mf(MF &&mf) const noexcept
        {
            auto re = (real().*mf)();
            auto im = (imag().*mf)();
            return Complex<decltype(re)>(re, im);
        }

    public:
        /** Create an instance has the reduced dynamic range.
            \tparam DEST_WIDTH The bit width of the return value. (In fact, DEST_WIDTH can be more than WIDTH.)
            \return The value reduced dynamic range.

            The same value is returned if the return type can hold it, otherwise the clamped value is returned.
        */
        template<int DEST_WIDTH>
        constexpr auto reduce_dynamic_range() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template reduce_dynamic_range<DEST_WIDTH>;
            return apply_mf(mf);
        }

        /** Construct out of double.
            \param [in] re The real part.
            \param [in] im The imaginary part.
            \note The value may be implicitly rounded and clamped in the same manner of limit_precision().
        */
        explicit Complex(double re, double im = 0.0)
            : re_(re), im_(im)
        {}

        /** Clone from a std::complex<double>.
            \param [in] src The source value.
            \note The value may be implicitly rounded and clamped in the same manner of limit_precision().
        */
        explicit Complex(const std::complex<double> &src)
            : re_(src.real()), im_(src.imag())
        {}

        /** Convert the value into std::complex<double>.
            \return The value as std::complex<double>.
            \note The conversion is implicit because the new value is exactly equal to this.
            \note This conversion is implicit, but most std::complex functions are template functions, which don't use implicit conversion.
            \see dbl()
        */
#if __cplusplus > 202002L
        // for C++23
        constexpr
#endif
        operator std::complex<double>() const
        {
            return std::complex<double>(real(), imag());
        }

        /** Get the value as std::complex<double>.
            \return The value as std::complex<double>.
            \note The return value is exactly equal to this.
        */
        constexpr std::complex<double> dbl() const
        {
            return std::complex<double>(real(), imag());
        }

        /** Convert into bool.
            \return true if this is not zero.
        */
        explicit constexpr operator bool() const noexcept
        {
            return static_cast<bool>(real()) || static_cast<bool>(imag());
        }

        /* Unary Operators */
        /** Unary + operator.
            \return The same value of this.
         */
        constexpr Complex<T> operator+() const noexcept
        {
            return *this;
        }

        /** Unary - operator.
            \return The value multiplied by -1.
            \note This operator never causes overflow. The minimum value of FixedPoint is -1 * the maximum value.
         */
        constexpr Complex<T> operator-() const noexcept
        {
            return Complex<T>(-real(), -imag());
        }

        /** logical not operator.
            \return true if this is zero.
        */
        constexpr bool operator!() const noexcept
        {
            return !static_cast<bool>(*this);
        }


        /* Binary operators */
        /** Binary + operator.
            \tparam T2 The type of the op2.
            \param [in] op2 The second operand.
            \return this + op2
        */
        template<typename T2>
        constexpr auto operator+(const Complex<T2> &op2) const noexcept
        {
            auto re = real() + op2.real();
            auto im = imag() + op2.imag();
            return Complex<decltype(re)>(re, im);
        }

        /** Binary - operator.
            \tparam T2 The type of the op2.
            \param [in] op2 The second operand.
            \return this - op2
        */
        template<typename T2>
        constexpr auto operator-(const Complex<T2> &op2) const noexcept
        {
            auto re = real() - op2.real();
            auto im = imag() - op2.imag();
            return Complex<decltype(re)>(re, im);
        }

        /** Binary * operator.
            \tparam T2 The type of the op2.
            \param [in] op2 The second operand.
            \return this * op2
        */
        template<typename T2>
        constexpr auto operator*(const Complex<T2> &op2) const noexcept
        {
            auto re = real() * op2.real() - imag() * op2.imag();
            auto im = real() * op2.imag() + imag() * op2.real();
            return Complex<decltype(re)>(re, im);
        }

        /* Relational operators */
        /** Relational == operator.
            \tparam T2 The type of the op2.
            \param [in] op2 The second operand.
            \return true if The value of this is equal to the value of op2.
        */
        template<typename T2>
        constexpr bool operator==(const Complex<T2> &op2) const noexcept
        {
            return real() == op2.real() && imag() == op2.imag();
        }

        /** Relational != operator.
            \tparam T2 The type of the op2.
            \param [in] op2 The second operand.
            \return true if The value of this is not equal to the value of op2.
        */
        template<typename T2>
        constexpr bool operator!=(const Complex<T2> &op2) const noexcept
        {
            return !(*this == op2);
        }


        /* Math functions */
        /** norm function.
            \return norm of this.
        */
        constexpr auto norm() const noexcept
        {
            return real() * real() + imag() * imag();
        }

        /** conj function.
            \return complex conjugate of this.
        */
        constexpr Complex<T> conj() const noexcept
        {
            return Complex(real(), -imag());
        }

        /** Get the inphase value of this and reference vector.
            \tparam T2 The type of the ref.
            \param [in] ref The reference vector.
            \return The inphase value, which is equal to |ref| * real(this * ref.conj()).
        */
        template<typename T2>
        constexpr auto inphase(const Complex<T2> &ref) const noexcept
        {
            return real() * ref.real() + imag() * ref.imag();
        }

        /** Get the quadrature value of this and reference vector.
            \tparam T2 The type of the ref.
            \param [in] ref The reference vector.
            \return The quadrature value, which is equal to |ref| * imag(this * ref.conj()).
        */
        template<typename T2>
        constexpr auto quadrature(const Complex<T2> &ref) const noexcept
        {
            return imag() * ref.real() - real() * ref.imag();
        }

        /** Multiplied by the imaginary unit.
            \return The value multiplied by the imaginary unit.
            \note That is a rotation of 90 degrees.
            \note A rotation of -90 degrees: a * (-i) == -(a * i) == -a.mult_i()
        */
        constexpr Complex<T> mult_i() const noexcept
        {
            return Complex<T>(-imag(), real());
        }

        /* Rounding functions */
        /** ceil function.
            \tparam LSB_PLACE the LSB place of the result.
            \return The minimum expressible value more than or equal this.
            \note This function may cause the result to exceed width, so it increases one bit to the upper side of this type.
        */
        template<int LSB_PLACE = 0>
        constexpr auto ceil() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template ceil<LSB_PLACE>;
            return apply_mf(mf);
        }

        /** floor function.
            \tparam LSB_PLACE the LSB place of the result.
            \return The maximum expressible value less than or equal this.
            \note This function may cause the result to exceed width, so it increases one bit to the upper side of this type.
        */
        template<int LSB_PLACE = 0>
        constexpr auto floor() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template floor<LSB_PLACE>;
            return apply_mf(mf);
        }

        /** trunc function.
            \tparam LSB_PLACE the LSB place of the result.
            \return The nearest expressible value that has the absolute value less than or equal the absolute value of this.
        */
        template<int LSB_PLACE = 0>
        constexpr auto trunc() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template trunc<LSB_PLACE>;
            return apply_mf(mf);
        }

        /** Round to nearest, ties to even.
            \tparam LSB_PLACE the LSB place of the result.
            \return The nearest expressible value of this, basically.
            \note This function may cause the result to exceed width, so it increases one bit to the upper side of this type.

            This function converts a value to the nearest expressible value. It converts this to nearest value whose LSB is zero if the value falls midway. It seems to be recommended by IEEE-745.
        */
        template<int LSB_PLACE = 0>
        constexpr auto round_half_to_even() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template round_half_to_even<LSB_PLACE>;
            return apply_mf(mf);
        }

        /** Round to nearest, ties away from zero.
            \tparam LSB_PLACE the LSB place of the result.
            \return The nearest expressible value of this, basically.
            \note This function may cause the result to exceed width, so it increases one bit to the upper side of this type.

            This function converts a value to the nearest expressible value. It converts this to nearest value away from zero if the value falls midway.
        */
        template<int LSB_PLACE = 0>
        constexpr auto round_half_away_from_zero() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template round_half_away_from_zero<LSB_PLACE>;
            return apply_mf(mf);
        }

        /** Round to nearest, ties to zero.
            \tparam LSB_PLACE the LSB place of the result.
            \return The nearest expressible value of this, basically.
            \note This function may cause the result to exceed width, so it increases one bit to the upper side of this type.

            This function converts a value to the nearest expressible value. It converts this to nearest value toward zero if the value falls midway.
        */
        template<int LSB_PLACE = 0>
        constexpr auto round_half_toward_zero() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template round_half_toward_zero<LSB_PLACE>;
            return apply_mf(mf);
        }

        /** Round to nearest, ties to upper.
            \tparam LSB_PLACE the LSB place of the result.
            \return The nearest expressible value of this, basically.
            \note This function may cause the result to exceed width, so it increases one bit to the upper side of this type.

            This function converts a value to the nearest expressible value. It converts this to nearest upper value if the value falls midway.
        */
        template<int LSB_PLACE = 0>
        constexpr auto round_half_up() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template round_half_up<LSB_PLACE>;
            return apply_mf(mf);
        }

        /** Round to nearest, ties to lower.
            \tparam LSB_PLACE the LSB place of the result.
            \return The nearest expressible value of this, basically.
            \note This function may cause the result to exceed width, so it increases one bit to the upper side of this type.

            This function converts a value to the nearest expressible value. It converts this to nearest lower value if the value falls midway.
        */
        template<int LSB_PLACE = 0>
        constexpr auto round_half_down() const noexcept
        {
            auto (value_t::*mf)() const = &value_t::template round_half_down<LSB_PLACE>;
            return apply_mf(mf);
        }

        /* Real and imaginary parts */
        /** Get the real part.
            \return The real part.
        */
        constexpr T real() const noexcept
        {
            return re_;
        }

        /** Get the imaginary part.
            \return The imaginary part.
        */
        constexpr T imag() const noexcept
        {
            return im_;
        }

        /** Set the real part.
            \param [in] val The real part.
        */
        constexpr void real(T val) noexcept
        {
            re_ = val;
        }

        /** Set the imaginary part.
            \param [in] val The imaginary part.
        */
        constexpr void imag(T val) noexcept
        {
            im_ = val;
        }

        /** Set the real part.
            \tparam T2 A narrower FixedPoint.
            \param [in] val The real part.
            \note "Narrower" means that the MSB is less than or equal this type's and the LSB is more than or equal this type's.
            \warning A compilation error is caused if the type of val is wider than this type.
            \attention Complex has no conversion from wider Complex because it can be inexact. Use conversion from std::complex<double>.
        */
        template<typename T2>
        constexpr void real(T2 val) noexcept
        {
            re_ = val;
        }

        /** Set the imaginary part.
            \tparam T2 A narrower FixedPoint.
            \param [in] val The imaginary part.
            \note "Narrower" means that the MSB is less than or equal this type's and the LSB is more than or equal this type's.
            \warning A compilation error is caused if the type of val is wider than this type.
            \attention Complex has no conversion from wider Complex because it can be inexact. Use conversion from std::complex<double>.
        */
        template<typename T2>
        constexpr void imag(T2 val) noexcept
        {
            im_ = val;
        }

        /** Set the real part.
            \param [in] val The real part.
        */
        constexpr void real(double val) noexcept
        {
            re_ = static_cast<value_t>(val);
        }

        /** Set the imaginary part.
            \param [in] val The imaginary part.
        */
        constexpr void imag(double val) noexcept
        {
            im_ = static_cast<value_t>(val);
        }
    private:
        /** Forbid to clone from an integer type.
            \note Because it may make easy mistake the assignment of a significand for the assignment of an integer value.
        */
        constexpr Complex(std::intmax_t re, std::intmax_t im = 0) noexcept = delete;
        /** Forbid to clone from an integer type.
            \note Because it may make easy mistake the assignment of a significand for the assignment of an integer value.
        */
        constexpr Complex(std::uintmax_t re, std::uintmax_t im = 0) noexcept = delete;

    private:
        value_t re_; /**< The real part. */
        value_t im_; /**< The imaginary part. */
    };

    /** A fixed point complex number class using class FixedPoint.
        \tparam WIDTH The bit width of the significand, including the sign bit and the hidden bit of double.
        \tparam PLACE The LSB place whose \#n has the weight 2**n.
        \note An alias of a type of Complex.
    */
    template<int WIDTH, int PLACE>
    using FixedPointComplex = Complex<FixedPoint<WIDTH, PLACE>>;

    /** Get the real part.
        \tparam T A type of FixedPoint.
        \param [in] x A value of Complex.
        \return The real part.
    */
    template<typename T>
    inline constexpr T real(const Complex<T>& x) noexcept
    {
        return x.real();
    }

    /** Get the imaginary part.
        \tparam T A type of FixedPoint.
        \param [in] x A value of Complex.
        \return The imaginary part.
    */
    template<typename T>
    inline constexpr T imag(const Complex<T>& x) noexcept
    {
        return x.imag();
    }

    /** norm function.
        \tparam T A type of FixedPoint.
        \param [in] x A value of Complex.
        \return norm of x.
    */
    template<typename T>
    inline constexpr auto norm(const Complex<T>& x) noexcept
    {
        return x.norm();
    }
}
#endif // PREC_CTRL_FIXEDPOINTCOMPLEX_H_
