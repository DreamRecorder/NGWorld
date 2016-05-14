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
using namespace std;

template <typename T>
bool compare_vector_2d(const Vector2D<T> &left, const Vector2D<T> &right)
{
    return (left.x < right.x) || (left.x == right.x && left.y < right.y);
}

template <typename T>
bool compare_vector_3d(const Vector3D<T> &left, const Vector3D<T> &right)
{
    return (left.x < right.x) || ((left.x == right.x) && (left.y < right.y)) ||
           ((left.x == right.x) && (left.y == right.y) && (left.z < right.z));
}

template <typename T>
bool compare_vector_3d_xzy(const Vector3D<T> &left, const Vector3D<T> &right)
{
    return (left.x < right.x) || ((left.x == right.x) && (left.z < right.z)) ||
           ((left.x == right.x) && (left.z == right.z) && (left.y < right.y));
}