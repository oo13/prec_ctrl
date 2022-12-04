
<!--
This file is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This file is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file.  If not, see <http://www.gnu.org/licenses/>.

Copyright © 2022 OOTA, Masato
-->

# Controlling the Precision of Double to Accumulate Exactly
## In Short
You can limit the precision of the input value so that the multiplication and the addition/subtraction result the exact double precision value because the operations doesn't cause a rounding.

Only what you need is [prec_ctrl::limit_precision(double, int, int)](@ref limit_precision()) in ["include/double_limit.h"](@ref include/double_limit.h). By the way, you can easily write your version of it because it's a simple function.

## Background
### Condition for the Exact Operation in Double
In IEEE-754, the double has 54 bits significand. (In this document, "significand" includes the hidden bit and the sign bit.) A result of a double operation is exact if the significand of the result stays in the range between -(2<sup>53</sup> - 1) and +(2<sup>53</sup> - 1).

In practical range, exactness doesn't depend on the exponent.

### The Sufficient Bit-Width of the Significand for Double Operations
It's almost same as the integer operation. To make it clear, I created [FixedPoint](@ref prec_ctrl::FixedPoint) class calculating with the same way of double. It handles the number by manually shifting and calculating of the integer.

#### For Addition/Subtraction
Basically, the addition and the subtraction add one bit-width to the significand. More strictly, it's two plus the difference of the places of the higher MSB and the lower LSB.

For example: (I would use the fixed point to explain about the sufficient bit-width of the significand because it's easily understandable.)
```
                  Place#   4   3   2   1   0  -1  -2  -3  -4  -5
Operand#1                    +---+---+---+---+---+---+---+---+
(8 bit)                      | S |   |   |   |   |   |   |   |
-7.9375 to +7.9375           +---+---+---+---+---+---+---+---+
                           MSB#1 = 3                   LSB#1 = -4
Operand#2                            +---+---+---+---+---+---+---+
(7 bit)                              | S |   |   |   |   |   |   |
-1.96875 to +1.96875                 +---+---+---+---+---+---+---+
                        +          MSB#2 = 1               LSB#2 = -5
                      ------------------------------------------------

Result                   +---+---+---+---+---+---+---+---+---+---+
(10 bit)                 | S |   |   |   |   |   |   |   |   |   |
-9.90625 to +9.90625     +---+---+---+---+---+---+---+---+---+---+
                                             ^
                                      Decimal Point
- LSB of Result = min(LSB#1, LSB#2)
- MSB of Result = max(MSB#1, MSB#2) + 1
- Sufficient Bit-Width of Result = MSB - LSB + 1 = max(MSB#1, MSB#2) - min(LSB#1, LSB#2) + 2
```
The addition or the subtraction results an exact value of double if the significand bit-width of the result is less than 55. If you want an exact result you need to limit the precision of the operands so that the significand bit-width of the result is less than 55. You can use [prec_ctrl::limit_precision(double, int, int)](@ref limit_precision()) to limit the precision of the operands.

#### For Multiplication
For the multiplication, the sufficient bit-width of the significand is one less than the sum of the bit-width of the two operands. The bit-width doesn't depend on the places.

For example:
```
                  Place#   4   3   2   1   0  -1  -2  -3  -4  -5
Operand#1                    +---+---+---+---+---+---+
(6 bit)                      | S |   |   |   |   |   |
-7.75 to +7.75               +---+---+---+---+---+---+
                           MSB#1 = 3           LSB#1 = -2
Operand#2                            +---+---+---+---+---+
(5 bit)                              | S |   |   |   |   |
-1.875 to +1.875                     +---+---+---+---+---+
                        *          MSB#2 = 1       LSB#2 = -3
                      ------------------------------------------------

Result                   +---+---+---+---+---+---+---+---+---+---+
(10 bit)                 | S |   |   |   |   |   |   |   |   |   |
-14.53125 to +14.53125   +---+---+---+---+---+---+---+---+---+---+
                                             ^
                                      Decimal Point
- LSB of Result = LSB#1 + LSB#2
- Sufficient Bit-Width of Result = Bit-Width#1 + Bit-Width#2 - 1
```

#### Division
For division, the bit-width of the significand depends on the values of the operands. For example, if a division is indivisible, the result is rounded. It's same as the integer division.

If a divisor is the power of 2, the division doesn't change the bit-width of the significand. It's always safe to use.

<a id="multi_and_sum"></a>
## A Example of a Determination of a Bit-Width
We would need exactly to add and subtract some double values and the maximum number of times is 1,048,575 = 2<sup>20</sup> - 1:

- Maximum Summation = Σ(_n_<sub>i</sub> * _a_<sub>i</sub>)
- Σ(_n_<sub>i</sub>) <= 2<sup>20</sup> - 1
- _a_<sub>i</sub>: A double number that we would add and subtract.

The significand bit-width of _n_ is 21, including the sign bit, so the maximum _a_'s bit-width is 34 = 54 - 21 + 1.

The dynamic range and the resolution is a trade-off. If you want that the resolution of (_n_<sub>i</sub> * _a_<sub>i</sub>) is less than or equal to 1, the LSB of _a_ has to be less than or equal to 2<sup>-20</sup>, so the maximum _a_'s dynamic range is about ±8192. If you want that the dynamic range of _a_ should be more than or equal to ±(2<sup>20</sup> - 1), the LSB of _a_ has to be more than or equal to 2<sup>-13</sup>.

## (FYI) Pretty Printing
### To Exchange Double (and other precision) Values
There is some algorithms for a pretty printing in order to exchange the values, such as [Dragon4](https://dl.acm.org/doi/10.1145/93548.93559), [Grisu](https://dl.acm.org/citation.cfm?id=1806623) ([an implementation](https://github.com/google/double-conversion)), and [Ryu](https://dl.acm.org/doi/10.1145/3192366.3192369) ([an implementation](https://github.com/ulfjack/ryu)). In brief, they choose a number that is the shortest digits of radix 10 in the range of the true value ± LSB/2. The standard library seem to use a similar algorithm for the standard floating point types. When you limit the precision, you may create your own version to fit the precision.

### To Output a Value in the User Interface
You may forge an output as the users wish. I recommend using a radix 10 rounding (such as rounding at the place 1/10) only for an output in a UI; Use a radix 2 rounding (such as rounding at the place 1/16) for others, including the input from the users. If you really need to handle exact 10<sup>-n</sup>, you should change the unit to 10<sup>-n</sup> (it means the value 1 represents 10<sup>-n</sup>), and also if you are quite sure that the unit should be 10<sup>-n</sup>, you may consider using an integer type instead of double.

## The Precision Control Library
This repository includes the library to control the precision of double and to handle a fixed point number.

### Compilation
This is a header only library. Set the include path to prec_ctrl/include.

The target language is C++17, but probably ["include/double_limit.h"](@ref include/double_limit.h) can be compiled in C++11.

### To Generate the Documents
Use doxygen. I tested the generation in doxygen 1.9.4.

```
% cd prec_ctrl
% doxygen prec_ctrl.doxygen
# Open prec_ctrl/html/index.html with your HTML browser.
```

### Unit Test
This library includes some unit test codes. If you want to run it, the following programs are needed:

- Catch2 (Tested in version 2.13.10)
- cmake  (Tested in Version 3.24.3)

```
% cd prec_ctrl/test
% cmake -DCMAKE_BUILD_TYPE=Release -B build .
% cd build
% make
% ./test_prec_ctrl
% ./test_prec_ctrl '[!benchmark]' # For benchmark
```

### Example for Some Results
The following code multiplies 0.1 and 1 000 000, and then subtracts 0.1 from the sum for 1 000 000 times.

#### Double without a Limitation of the Precision (No use this library)
Code:
```
#include <iostream>
#include <iomanip>

int main()
{
    const double INCR = 0.1;
    const int MULT = 1E+6;
    const double SUM = INCR * MULT;
    double remain = SUM;
    int count = 0;
    while (remain > 0.0) {
        count++;
        remain -= INCR;
    }
    std::cout << "      SUM = count * INCR + remain" << std::endl;
    std::cout << std::setprecision(16) << std::scientific;
    std::cout << "Dec: " << SUM << " = " << count << " * " << INCR << " + " << remain << std::endl;
    std::cout << std::hexfloat;
    std::cout << "Hex: " << SUM << " = " << count << " * " << INCR << " + " << remain << std::endl;
    return 0;
}
```
Result:
```
      SUM = count * INCR + remain
Dec: 1.0000000000000000e+05 = 1000000 * 1.0000000000000001e-01 + -1.3328811345469926e-06
Hex: 0x1.86ap+16 = 1000000 * 0x1.999999999999ap-4 + -0x1.65cae4e4ep-20
```
The `remain` is not exact 0, because `SUM` is not exact value; it should be 1000000 * 0x1.999999999999ap-4, but it cannot be expressed by the double precision. (`INCR` should be exact 0.1? The floating point uses the binary number, but some programmers seemingly tend to forget it when they handle a number after the decimal point. If you really need exact 0.1, you can change the unit to 0.1.)

`SUM` is the exact 1 000 000, but it's just lucky.

#### Double with a Limitation of the Precision
Code:
```
#include <iostream>
#include <iomanip>
#include "double_limit.h"

int main()
{
    const double INCR = prec_ctrl::limit_precision(0.1, 32, -24);
    const int MULT = 1E+6;
    const double SUM = INCR * MULT;
    double remain = SUM;
    int count = 0;
    while (remain > 0.0) {
        count++;
        remain -= INCR;
    }
    std::cout << "      SUM = count * INCR + remain" << std::endl;
    std::cout << std::setprecision(16) << std::scientific;
    std::cout << "Dec: " << SUM << " = " << count << " * " << INCR << " + " << remain << std::endl;
    std::cout << std::hexfloat;
    std::cout << "Hex: " << SUM << " = " << count << " * " << INCR << " + " << remain << std::endl;
    return 0;
}
```
Result:
```
      SUM = count * INCR + remain
Dec: 1.0000002384185791e+05 = 1000000 * 1.0000002384185791e-01 + 0.0000000000000000e+00
Hex: 0x1.86a0061a8p+16 = 1000000 * 0x1.9999ap-4 + 0x0p+0
```
The `remain` is the exact 0, because `INCR` is limited the precision and `SUM` is the exact value.

#### FixedPoint
Code:
```
#include <iostream>
#include <iomanip>
#include "FixedPoint.h"

int main()
{
    using namespace prec_ctrl;
    const FixedPoint<32, -24> INCR(0.1);
    const int MULT = 1E+6;
    const FixedPoint<52, -24> SUM(INCR * MULT);
    significand_t<52> remain = SUM.get_significand();
    int count = 0;
    while (remain > 0) {
        count++;
        remain -= INCR.get_significand();
    }
    FixedPoint<52, -24> remain_fixed;
    remain_fixed.set_significand(remain);
    std::cout << "      SUM = count * INCR + remain" << std::endl;
    std::cout << std::setprecision(16) << std::scientific;
    std::cout << "Dec: " << SUM << " = " << count << " * " << INCR << " + " << remain_fixed << std::endl;
    std::cout << std::hexfloat;
    std::cout << "Hex: " << SUM << " = " << count << " * " << INCR << " + " << remain_fixed << std::endl;
    return 0;
}
```
Result:
```
      SUM = count * INCR + remain
Dec: 1.0000002384185791e+05 = 1000000 * 1.0000002384185791e-01 + 0.0000000000000000e+00
Hex: 0x1.86a0061a8p+16 = 1000000 * 0x1.9999ap-4 + 0x0p+0
```
The result is the same as the double that is limited the same precision.

Please note that it handles directly the significand of `remain` when decreasing `INCR`. We know that `remain` is always a positive or zero and less than or equal to `SUM` and the same resolution as `SUM`, so we have no need to use a FixedPoint. If you would use a FixedPoint, you should subtract by a tournament method. It's very troublesome, so I recommend handling the significand when accumulating some FixedPoint values. I provide some helper functions in ["include/fixed_numeric.h"](@ref include/fixed_numeric.h) for the accumulation.

### A Comment on the benchmark in the Unit Test
multi_and_sum() calculates [the sum of multiplications of an integer and a double value](#multi_and_sum).

The FixedPoint version of multi_and_sum() is a bit faster than the double version on my PC. Possibly, some computers focusing on the floating point calculations can calculate the double numbers faster.

The setup codes consist of some copyings and clampings. Including the setup codes, the FixedPoint version is slower than the double version, because, probably, the FixedPoint version clamps the both operands but the double version clamps only an operand of double.

I don't recommend FixedPoint in the practical code because the double version is not much slower and fits the incremental development, and also if you can determine the bit-width and the bit-place of the operations in advance, you may consider using an integer type for the operations. I intend that FixedPoint is a learning and a design tool that tells you a sufficient bit-width and an accurate bit-place for an operation.

```
-------------------------------------------------------------------------------
FixedPoint and double Benchmark
-------------------------------------------------------------------------------
prec_ctrl/test/equivalence.cpp:171
...............................................................................

benchmark name                       samples       iterations    estimated
                                     mean          low mean      high mean
                                     std dev       low std dev   high std dev
-------------------------------------------------------------------------------
double multi_and_sum()                         100          8585    109.029 ms 
                                        127.481 ns    127.368 ns    127.627 ns 
                                       0.647396 ns   0.513698 ns    1.00688 ns 
                                                                               
double setup codes and                                                         
multi_and_sum()                                100          1941    109.084 ms 
                                        555.142 ns     554.42 ns    556.484 ns 
                                        4.84797 ns    3.09992 ns     9.1983 ns 
                                                                               
FixedPoint multi_and_sum()                     100         11443    108.709 ms 
                                          95.27 ns    95.2136 ns    95.3357 ns 
                                        0.30918 ns   0.268736 ns   0.384317 ns 
                                                                               
FixedPoint setup codes and                                                     
multi_and_sum()                                100          1915    109.155 ms 
                                        572.053 ns    571.557 ns    572.571 ns 
                                        2.58533 ns    2.30662 ns    2.95741 ns 
                                                                               

===============================================================================
```

## License
See copyright file for the copyright notice and the license details.

This program is published under GPL-3.0-or-later:

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
