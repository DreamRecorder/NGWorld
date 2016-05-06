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

#ifndef _FUNDAMENTAL_STRUCTURE_H_
#define _FUNDAMENTAL_STRUCTURE_H_

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long s32;
typedef unsigned long u32;
typedef long long s64;
typedef unsigned long long u64;

template<typename T = int>
class Vector3D
{
    public:
        T x, y, z;
        Vector3D<T>() : x(), y(), z() { }
        Vector3D<T>(T ix, T iy, T iz) : x(ix), y(iy), z(iz) { }
        ~Vector3D<T>() { }

        // 向量和向量相加
        Vector3D<T> operator + (const Vector3D<T> &arg) const;
        Vector3D<T>& operator += (const Vector3D<T> &add);
        // 向量和标量相加
        Vector3D<T> operator + (const T &arg) const;
        Vector3D<T>& operator += (const T &arg);
        
        // 向量和向量相减
        Vector3D<T> operator - (const Vector3D<T> &arg) const;
        Vector3D<T>& operator -= (const Vector3D<T> &add);
        // 向量和标量相减
        Vector3D<T> operator - (const T &arg) const;
        Vector3D<T>& operator -= (const T &arg);

        // 点乘
        T dot(const Vector3D<T> &arg);

        // 叉乘
        Vector3D<T> cross(const Vector3D<T> &arg);
        
        // 向量与标量相乘
        Vector3D<T> operator * (const T &arg) const;
        Vector3D<T>& operator *= (const T &arg);

        // 向量与标量相除
        Vector3D<T> operator / (const T &arg) const;
        Vector3D<T>& operator /= (const T &arg);
};

typedef Vector3D<s16> v3s16;
typedef Vector3D<s32> v3s32;
typedef Vector3D<float> v3f;
typedef Vector3D<double> v3d;

#endif
