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

#include "compress.h"
#include <quicklz.h>
#include <cstdlib>
#include <cstring>
using namespace std;

size_t compress(const char *src, char *dest, size_t size)
{
    QuickLZ::qlz_state_compress *state = new QuickLZ::qlz_state_compress;
    size_t compressed_size = QuickLZ::qlz_compress(src, dest, size, state);
    delete state;
    return compressed_size;
}

size_t decompress(const char *src, char *dest)
{
    QuickLZ::qlz_state_decompress *state = new QuickLZ::qlz_state_decompress;
    size_t decompressed_size = QuickLZ::qlz_decompress(src, dest, state);
    delete state;
    return decompressed_size;
}
