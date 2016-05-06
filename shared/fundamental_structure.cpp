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

#include "fundamental_structure.h"

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

