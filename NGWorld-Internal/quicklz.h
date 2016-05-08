/*
 * QuickLZ: a fast data compression library
 * (C) Copyright 2006-2011 Lasse Mikkel Reinhold <lar@quicklz.com>
 *
 * QuickLZ can be used for free under the GPL 1, 2 or 3 license (where anything
 * released into public must be open source) or under a commercial license if such
 * has been acquired (see http: *www.quicklz.com/order.html). The commercial license
 * does not cover derived or ported versions created by third parties under GPL.
 *
 * You can edit following user settings. Data must be decompressed with the same
 * setting of QLZ_COMPRESSION_LEVEL and QLZ_STREAMING_BUFFER as it was compressed
 * (see manual). If QLZ_STREAMING_BUFFER > 0, scratch buffers must be initially
 * zeroed out (see manual). First #ifndef makes it possible to define settings from
 * the outside like the compiler command line.
 *
 * This file is modified to fit the NGWorld project, and is part of NGWorld.
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
#ifndef QLZ_HEADER
#define QLZ_HEADER

#include "fundamental_structure.h"
#include <cstdlib>

#ifndef QLZ_COMPRESSION_LEVEL

// 1 gives fastest compression speed. 3 gives fastest decompression speed and best
// compression ratio.
#define QLZ_COMPRESSION_LEVEL 1
//#define QLZ_COMPRESSION_LEVEL 2
//#define QLZ_COMPRESSION_LEVEL 3

// If > 0, zero out both states prior to first call to qlz_compress() or qlz_decompress()
// and decompress packets in the same order as they were compressed
#define QLZ_STREAMING_BUFFER 0
//#define QLZ_STREAMING_BUFFER 100000
//#define QLZ_STREAMING_BUFFER 1000000

// Guarantees that decompression of corrupted data cannot crash. Decreases decompression
// speed 10-20%. Compression speed not affected.
//#define QLZ_MEMORY_SAFE
#endif

#define QLZ_VERSION_MAJOR 1
#define QLZ_VERSION_MINOR 5
#define QLZ_VERSION_REVISION 0

// Decrease QLZ_POINTERS for level 3 to increase compression speed. Do not touch any other values!
#if QLZ_COMPRESSION_LEVEL == 1
#define QLZ_POINTERS 1
#define QLZ_HASH_VALUES 4096
#elif QLZ_COMPRESSION_LEVEL == 2
#define QLZ_POINTERS 4
#define QLZ_HASH_VALUES 2048
#elif QLZ_COMPRESSION_LEVEL == 3
#define QLZ_POINTERS 16
#define QLZ_HASH_VALUES 4096
#endif

// Detect if pointer size is 64-bit. It's not fatal if some 64-bit target is not detected because this is only for adding an optional 64-bit optimization.
#if defined _LP64 || defined __LP64__ || defined __64BIT__ || _ADDR64 || defined _WIN64 || defined __arch64__ || __WORDSIZE == 64 || (defined __sparc && defined __sparcv9) || defined __x86_64 || defined __amd64 || defined __x86_64__ || defined _M_X64 || defined _M_IA64 || defined __ia64 || defined __IA64__
#define QLZ_PTR_64
#endif

namespace QuickLZ
{
    
    // hash entry
    struct qlz_hash_compress
    {
#if QLZ_COMPRESSION_LEVEL == 1
        u32 cache;
#if defined QLZ_PTR_64 && QLZ_STREAMING_BUFFER == 0
        unsigned int offset;
#else
        const unsigned char *offset;
#endif
#else
        const unsigned char *offset[QLZ_POINTERS];
#endif
        
    };
    
    struct qlz_hash_decompress
    {
#if QLZ_COMPRESSION_LEVEL == 1
        const unsigned char *offset;
#else
        const unsigned char *offset[QLZ_POINTERS];
#endif
    };
    
    
    // states
    struct qlz_state_compress
    {
#if QLZ_STREAMING_BUFFER > 0
        unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
#endif
        size_t stream_counter;
        qlz_hash_compress hash[QLZ_HASH_VALUES];
        unsigned char hash_counter[QLZ_HASH_VALUES];
    };
    
    
#if QLZ_COMPRESSION_LEVEL == 1 || QLZ_COMPRESSION_LEVEL == 2
    struct qlz_state_decompress
    {
#if QLZ_STREAMING_BUFFER > 0
        unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
#endif
        qlz_hash_decompress hash[QLZ_HASH_VALUES];
        unsigned char hash_counter[QLZ_HASH_VALUES];
        size_t stream_counter;
    };
#elif QLZ_COMPRESSION_LEVEL == 3
    struct qlz_state_decompress
    {
#if QLZ_STREAMING_BUFFER > 0
        unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
#endif
#if QLZ_COMPRESSION_LEVEL <= 2
        qlz_hash_decompress hash[QLZ_HASH_VALUES];
#endif
        size_t stream_counter;
    };
#endif
    
    extern "C"
    {
        size_t qlz_size_decompressed(const char *source);
        size_t qlz_size_compressed(const char *source);
        size_t qlz_compress(const void *source, char *destination, size_t size, qlz_state_compress *state);
        size_t qlz_decompress(const char *source, void *destination, qlz_state_decompress *state);
        int qlz_get_setting(int setting);
    }
    
}

#endif

