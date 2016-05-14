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

#include <cmath>
#include "fundamental_algorithm.h"

template <typename T = int>
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
};

template <typename T = int>
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

    // 将向量沿着逆时针方向旋转angle(弧度制)度
    void rotate(const double &angle);
};

template<typename T>
Vector3D<T> Vector3D<T>::operator + (const Vector3D<T> &arg) const
{
    return Vector3D<T>(x + arg.x, y + arg.y, z + arg.z);
}

template<typename T>
Vector3D<T>& Vector3D<T>::operator += (const Vector3D<T> &arg)
{
    x += arg.x;
    y += arg.y;
    z += arg.z;
    return *this;
}

template<typename T>
Vector3D<T> Vector3D<T>::operator + (const T &arg) const
{
    return Vector3D<T>(x + arg, y + arg, z + arg);
}

template<typename T>
Vector3D<T>& Vector3D<T>::operator += (const T &arg)
{
    x += arg;
    y += arg;
    z += arg;
    return *this;
}

template<typename T>
Vector3D<T> Vector3D<T>::operator - (const Vector3D<T> &arg) const
{
    return Vector3D<T>(x - arg.x, y - arg.y, z - arg.z);
}

template<typename T>
Vector3D<T>& Vector3D<T>::operator -= (const Vector3D<T> &arg)
{
    x -= arg.x;
    y -= arg.y;
    z -= arg.z;
    return *this;
}

template<typename T>
Vector3D<T> Vector3D<T>::operator - (const T &arg) const
{
    return Vector3D<T>(x - arg, y - arg, z - arg);
}

template<typename T>
Vector3D<T>& Vector3D<T>::operator -= (const T &arg)
{
    x -= arg;
    y -= arg;
    z -= arg;
    return *this;
}

template<typename T>
T Vector3D<T>::dot(const Vector3D<T> &arg)
{
    return x * arg.x + y * arg.y + z * arg.z;
}

template<typename T>
Vector3D<T> Vector3D<T>::cross(const Vector3D<T> &arg)
{
    return Vector3D(y * arg.z - z * arg.y,
                    z * arg.x - x * arg.z,
                    x * arg.y - y * arg.x);
}

template<typename T>
Vector3D<T> Vector3D<T>::operator * (const T &arg) const
{
    return Vector3D<T>(x * arg, y * arg, z * arg);
}

template<typename T>
Vector3D<T>& Vector3D<T>::operator *= (const T &arg)
{
    x *= arg;
    y *= arg;
    z *= arg;
    return *this;
}

template<typename T>
Vector3D<T> Vector3D<T>::operator / (const T &arg) const
{
    return Vector3D<T>(x / arg, y / arg, z / arg);
}

template<typename T>
Vector3D<T>& Vector3D<T>::operator /= (const T &arg)
{
    x /= arg;
    y /= arg;
    z /= arg;
    return *this;
}

template<typename T>
Vector2D<T> Vector2D<T>::operator + (const Vector2D<T> &arg) const
{
    return Vector2D<T>(x + arg.x, y + arg.y);
}

template<typename T>
Vector2D<T>& Vector2D<T>::operator += (const Vector2D<T> &arg)
{
    x += arg.x;
    y += arg.y;
    return *this;
}

template<typename T>
Vector2D<T> Vector2D<T>::operator + (const T &arg) const
{
    return Vector2D<T>(x + arg, y + arg);
}

template<typename T>
Vector2D<T>& Vector2D<T>::operator += (const T &arg)
{
    x += arg;
    y += arg;
    return *this;
}

template<typename T>
Vector2D<T> Vector2D<T>::operator - (const Vector2D<T> &arg) const
{
    return Vector2D<T>(x - arg.x, y - arg.y);
}

template<typename T>
Vector2D<T>& Vector2D<T>::operator -= (const Vector2D<T> &arg)
{
    x -= arg.x;
    y -= arg.y;
    return *this;
}

template<typename T>
Vector2D<T> Vector2D<T>::operator - (const T &arg) const
{
    return Vector2D<T>(x - arg, y - arg);
}

template<typename T>
Vector2D<T>& Vector2D<T>::operator -= (const T &arg)
{
    x -= arg;
    y -= arg;
    return *this;
}

template<typename T>
T Vector2D<T>::dot(const Vector2D<T> &arg)
{
    return x * arg.x + y * arg.y;
}

template<typename T>
Vector3D<T> Vector2D<T>::cross(const Vector2D<T> &arg)
{
    return Vector3D<T>(0, 0, x * arg.y - arg.x * y);
}

template<typename T>
Vector2D<T> Vector2D<T>::operator * (const T &arg) const
{
    return Vector2D<T>(x * arg, y * arg);
}

template<typename T>
Vector2D<T>& Vector2D<T>::operator *= (const T &arg)
{
    x *= arg;
    y *= arg;
    return *this;
}

template<typename T>
Vector2D<T> Vector2D<T>::operator / (const T &arg) const
{
    return Vector2D<T>(x / arg, y / arg);
}

template<typename T>
Vector2D<T>& Vector2D<T>::operator /= (const T &arg)
{
    x /= arg;
    y /= arg;
    return *this;
}

template<typename T>
void Vector2D<T>::rotate(const double &angle)
{
    T orig_x = x, orig_y = y;
#ifdef NGWORLD_USE_OWN_MATH_FX
    x = orig_x * ngw_cos_fast(angle) - orig_y * ngw_sin_fast(angle);
    y = orig_x * ngw_sin_fast(angle) - orig_y * ngw_cos_fast(angle);
#else
    x = orig_x * cos(angle) - orig_y * sin(angle);
    y = orig_x * sin(angle) - orig_y * cos(angle);
#endif
}

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

// 按照x-y的顺序比较二维向量
template <typename T>
bool compare_vector_2d(const Vector2D<T> &left, const Vector2D<T> &right);

// 按照x-y-z的顺序比较三维向量
template <typename T>
bool compare_vector_3d(const Vector3D<T> &left, const Vector3D<T> &right);

// 按照x-z-y的顺序比较三维向量
template <typename T>
bool compare_vector_3d_xzy(const Vector3D<T> &left, const Vector3D<T> &right);

#endif
