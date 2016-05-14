/*
 * This file is part of NGWorld.
 * (C) Copyright 2016 DLaboratory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fundamental_algorithm.h"
#include <cmath>
using namespace std;

u64 bkdr_hash(const string &str, u64 magic_constant)
{
    // 此处采用BKDR哈希算法
    // 参考文献: The C Programming Language
    // -- Brian Kernighan & Dennis Ritchie
    u64 result = 0;
    for(size_t i = 0, size = str.size(); i < size; i++)
        result = result * magic_constant + str[i];
    return result;
}

u32 crc32(const void *buf, int len)
{
    u32 ret = 0xFFFFFFFF;
    const u8 *p = static_cast<const u8*>(buf);
    for(int i = 0; i < len; i++)
        ret = crc_32_tab[((ret & 0xFF) ^ *p++)] ^ (ret >> 8);
    return (ret ^ 0xFFFFFFFF);
}

#ifdef NGWORLD_USE_OWN_MATH_FX
double ngw_sin_fast(double x)
{
    /*
    另一种方法(但是比下面实际采用的方法慢1/3)
    constexpr double tp = 1./(2.0*3.1415926);
    x *= tp;
    x -= 0.25 + std::floor(x + 0.25);
    x *= 16.0 * (std::abs(x) - 0.5);
    //x += T(.225) * x * (std::abs(x) - T(1.));
    return x;
    */
    const double B = 1.2732395447;
    const double C = -0.4052847346;
    const double P = 0.2310792853;//0.225;
    double y = B * x + C * x * abs(x);
    y = P * (y * abs(y) - y) + y;
    return y;
}

double ngw_cos_fast(double x)
{
    const double Q = 1.5707963268;
    const double PI =3.1415926536;
    x += Q;
    if(x > PI)
        x -= 2 * PI;
    return(ngw_sin_fast(x));
}
#endif
