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


  FixedPoint should be equivalent to double.

*/
#include <catch2/catch.hpp>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "double_limit.h"
#include "fixed_numeric.h"
#include "FixedPoint.h"

using namespace prec_ctrl;


// This function retuens a random positive integer (including 0) vector.
// The sum of the result is `sum`.
// The minimum size of the resule is `minimum_size`.
std::vector<unsigned int> gen_int_vector(unsigned int sum, const unsigned int minimum_size)
{
    std::mt19937 random_engine;
    std::uniform_int_distribution<unsigned int> dist(0, sum/minimum_size);
    std::vector<unsigned int> v;
    while (sum > 0) {
        const unsigned int n = std::min(sum, dist(random_engine));
        v.push_back(n);
        sum -= n;
    }
    return v;
}

// This function retuens a random double vector.
std::vector<double> gen_real_vector(const size_t s, const double abs_max)
{
    std::mt19937_64 random_engine;
    std::uniform_real_distribution<double> dist(-abs_max, +abs_max);
    std::vector<double> v(s);
    for (auto &elm : v) {
        elm = dist(random_engine);
    }
    return v;
}

// copy and limit_precision
std::vector<double> copy_and_limit(const std::vector<double> &a, int width, int place)
{
    const size_t sz = a.size();
    std::vector<double> result(sz);
    for (size_t i=0; i<sz; ++i) {
        result[i] = limit_precision(a[i], width, place);
    }
    return result;
}

// copy from integer vector to FixedPoint
template<int WIDTH>
std::vector<FixedPoint<WIDTH, 0>>
copy_from_int_vector(const std::vector<unsigned int> &n)
{
    const size_t sz = n.size();
    std::vector<FixedPoint<WIDTH, 0>> result(sz);
    for (size_t i=0; i<sz; ++i) {
        result[i].set_significand(n[i]);
    }
    return result;
}

// copy from double vector to FixedPoint
template<int WIDTH, int PLACE>
std::vector<FixedPoint<WIDTH, PLACE>>
copy_from_double_vector(const std::vector<double> &a)
{
    const size_t sz = a.size();
    std::vector<FixedPoint<WIDTH, PLACE>> result(sz);
    for (size_t i=0; i<sz; ++i) {
        result[i] = FixedPoint<WIDTH, PLACE>(a[i]);
    }
    return result;
}


// double version of test fucntion
double multi_and_sum(const std::vector<unsigned int> &n, const std::vector<double> &r)
{
    double sum = 0.0;
    for (size_t i=0; i<n.size(); ++i) {
        sum += n[i] * r[i];
    }
    return sum;
}

// FixedPoint version of test fucntion
template<int WIDTH_sig, int WIDTH_n, int PLACE_n, int WIDTH_r, int PLACE_r>
significand_t<WIDTH_sig>
multi_and_sum(const std::vector<FixedPoint<WIDTH_n, PLACE_n>> &n,
              const std::vector<FixedPoint<WIDTH_r, PLACE_r>> &r)
{
    significand_t<WIDTH_sig> sum = 0;
    for (size_t i=0; i<n.size(); ++i) {
        sum = significand_adder(sum, n[i] * r[i]);
    }
    return sum;
}


TEST_CASE( "FixedPoint should be equivalent to double", "[equivalence]" ) {
    constexpr int WIDTH_sum = 54;
    constexpr int WIDTH_n = 21; // max 1 048 575
    constexpr int WIDTH_r = WIDTH_sum - WIDTH_n + 1;
    constexpr int PLACE_r = -16; // max slight more than 131071.999

    const auto n = gen_int_vector(MAX_SIGNIFICAND_VALUE<WIDTH_n + 1>, 100);
    const auto r = gen_real_vector(n.size(), static_cast<std::uint_fast64_t>(1) << (WIDTH_r + PLACE_r - 1)); // slightly greater than maximum but it's no problem, because it's clamped later.

    // double version
    const std::vector<double> r1(copy_and_limit(r, WIDTH_r, PLACE_r));
    const auto sum1 = multi_and_sum(n, r1);

    // FixedPoint version
    const std::vector<FixedPoint<WIDTH_n, 0>> n2(copy_from_int_vector<WIDTH_n>(n));
    const std::vector<FixedPoint<WIDTH_r, PLACE_r>> r2(copy_from_double_vector<WIDTH_r, PLACE_r>(r));
    const auto sum2_sig = multi_and_sum<54>(n2, r2);
    decltype(n2[0] * r2[0]) sum2;
    sum2.set_significand(sum2_sig);

    SECTION( "multi_and_sum" ) {
        REQUIRE( sum1 == sum2 );
    }
    SECTION( "nothing remained" ) {
        // double version
        double sum_double = sum1;
        for (size_t i=0; i<n.size(); ++i) {
            for (size_t j=0; j<n[i]; ++j) {
                sum_double -= r1[i];
            }
        }
        REQUIRE( sum_double == 0 );

        // FixedPoint version
        REQUIRE( sum2.get_place() == PLACE_r );
        significand_t<WIDTH_sum> sum_significand = sum2.get_significand();
        for (size_t i=0; i<n2.size(); ++i) {
            const size_t num = static_cast<double>(n2[i]);
            for (size_t j=0; j<num; ++j) {
                sum_significand = significand_adder(sum_significand, -r2[i]);
            }
        }
        REQUIRE( sum_significand == 0 );
    }
}

TEST_CASE( "FixedPoint and double Benchmark", "[!benchmark]" ) {
    constexpr int WIDTH_sum = 54;
    constexpr int WIDTH_n = 21; // max 1 048 575
    constexpr int WIDTH_r = WIDTH_sum - WIDTH_n + 1;
    constexpr int PLACE_r = -16; // max slight more than 131071.999

    const auto n = gen_int_vector(MAX_SIGNIFICAND_VALUE<WIDTH_n + 1>, 100);
    const auto r = gen_real_vector(n.size(), static_cast<std::uint_fast64_t>(1) << (WIDTH_r + PLACE_r - 1)); // slightly greater than maximum but it's no problem, because it's clamped later.

    const std::vector<double> r1(copy_and_limit(r, WIDTH_r, PLACE_r));
    const std::vector<FixedPoint<WIDTH_n, 0>> n2(copy_from_int_vector<WIDTH_n>(n));
    const std::vector<FixedPoint<WIDTH_r, PLACE_r>> r2(copy_from_double_vector<WIDTH_r, PLACE_r>(r));

    // double version
    BENCHMARK( "double multi_and_sum()" ) {
        return multi_and_sum(n, r1);
    };

    BENCHMARK( "double setup codes and multi_and_sum()" ) {
        const std::vector<unsigned int> n12(n);
        const std::vector<double> r12(copy_and_limit(r, WIDTH_r, PLACE_r));
        return multi_and_sum(n12, r12);
    };

    // FixedPoint version
    BENCHMARK( "FixedPoint multi_and_sum()" ) {
        const auto sum_significand = multi_and_sum<54>(n2, r2);
        decltype(n2[0] * r2[0]) sum;
        sum.set_significand(sum_significand);
        return sum;
    };

    BENCHMARK( "FixedPoint setup codes and multi_and_sum()" ) {
        const std::vector<FixedPoint<WIDTH_n, 0>> n22(copy_from_int_vector<WIDTH_n>(n));
        const std::vector<FixedPoint<WIDTH_r, PLACE_r>> r22(copy_from_double_vector<WIDTH_r, PLACE_r>(r));
        const auto sum_significand = multi_and_sum<54>(n22, r22);
        decltype(n22[0] * r22[0]) sum;
        sum.set_significand(sum_significand);
        return sum;
    };
}

