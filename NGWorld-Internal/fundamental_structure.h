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
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;

template<typename T = int>
class Vector3D
{
    public:
        T x, y, z;
        Vector3D<T>() : x(), y(), z() { }
        Vector3D<T>(T _x, T _y, T _z) : x(_x), y(_y), z(_z) { }
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

        // 按照x-y-z的顺序比较大小
        bool operator < (const Vector3D<T> &arg) const
        {
            return (x < arg.x) || (x == arg.x && y < arg.y) || (x == arg.x && y == arg.y && z < arg.z);
        }
};

template<typename T = int>
class Vector2D
{
    public:
        T x, y;
        Vector2D<T>() : x(), y() { }
        Vector2D<T>(T _x, T _y) : x(_x), y(_y) { }
        ~Vector2D<T>() { }

        // 向量和向量相加
        Vector2D<T> operator + (const Vector2D<T> &arg) const;
        Vector2D<T>& operator += (const Vector2D<T> &add);
        // 向量和标量相加
        Vector2D<T> operator + (const T &arg) const;
        Vector2D<T>& operator += (const T &arg);

        // 向量和向量相减
        Vector2D<T> operator - (const Vector2D<T> &arg) const;
        Vector2D<T>& operator -= (const Vector2D<T> &add);
        // 向量和标量相减
        Vector2D<T> operator - (const T &arg) const;
        Vector2D<T>& operator -= (const T &arg);

        // 点乘
        T dot(const Vector2D<T> &arg);

        // 叉乘 [注意:得到一个三维的向量]
        Vector3D<T> cross(const Vector2D<T> &arg);

        // 向量与标量相乘
        Vector2D<T> operator * (const T &arg) const;
        Vector2D<T>& operator *= (const T &arg);

        // 向量与标量相除
        Vector2D<T> operator / (const T &arg) const;
        Vector2D<T>& operator /= (const T &arg);
};

typedef Vector2D<s16> v2s16;
typedef Vector2D<s32> v2s32;
typedef Vector2D<s64> v2s64;
typedef Vector2D<float> v2f;
typedef Vector2D<double> v2d;

typedef Vector3D<s16> v3s16;
typedef Vector3D<s32> v3s32;
typedef Vector3D<s64> v3s64;
typedef Vector3D<float> v3f;
typedef Vector3D<double> v3d;

#endif
