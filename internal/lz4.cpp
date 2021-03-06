/*
 LZ4 - Fast LZ compression algorithm
 Copyright (C) 2011-2015, Yann Collet.

 BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above
 copyright notice, this list of conditions and the following disclaimer
 in the documentation and/or other materials provided with the
 distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 You can contact the author at :
 - LZ4 source repository : https://github.com/Cyan4973/lz4
 - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
 */

/**************************************
 *  Tuning parameters
 **************************************/
/*
 * HEAPMODE :
 * Select how default compression functions will allocate memory for their hash table,
 * in memory stack (0:default, fastest), or in memory heap (1:requires malloc()).
 */
#define HEAPMODE 0

/*
 * ACCELERATION_DEFAULT :
 * Select "acceleration" for LZ4_compress_fast() when parameter value <= 0
 */
#define ACCELERATION_DEFAULT 1

/**************************************
 *  CPU Feature Detection
 **************************************/
/*
 * LZ4_FORCE_SW_BITCOUNT
 * Define this parameter if your target system or compiler does not support hardware bit count
 */
#if defined(_MSC_VER) && defined(_WIN32_WCE)   /* Visual Studio for Windows CE does not support Hardware bit count */
#  define LZ4_FORCE_SW_BITCOUNT
#endif

/**************************************
 *  Includes
 **************************************/
#include "lz4.h"
#include "fundamental_structure.h"

/**************************************
 *  Compiler Options
 **************************************/
#ifdef _MSC_VER    /* Visual Studio */
#  define FORCE_INLINE static __forceinline
#  include <intrin.h>
#  pragma warning(disable : 4127)        /* disable: C4127: conditional expression is constant */
#  pragma warning(disable : 4293)        /* disable: C4293: too large shift (32-bits) */
#else
#  if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)   /* C99 */
#    if defined(__GNUC__) || defined(__clang__)
#      define FORCE_INLINE static inline __attribute__((always_inline))
#    else
#      define FORCE_INLINE static inline
#    endif
#  else
#    define FORCE_INLINE static
#  endif   /* __STDC_VERSION__ */
#endif  /* _MSC_VER */

/* LZ4_GCC_VERSION is defined into lz4.h */
#if (LZ4_GCC_VERSION >= 302) || (__INTEL_COMPILER >= 800) || defined(__clang__)
#  define expect(expr,value)    (__builtin_expect ((expr),(value)) )
#else
#  define expect(expr,value)    (expr)
#endif

#define likely(expr)     expect((expr) != 0, 1)
#define unlikely(expr)   expect((expr) != 0, 0)


/**************************************
 *  Memory routines
 **************************************/
#include <stdlib.h>   /* malloc, calloc, free */
#define ALLOCATOR(n,s) calloc(n,s)
#define FREEMEM        free
#include <string.h>   /* memset, memcpy */
#define MEM_INIT       memset

/**************************************
 *  Reading and writing into memory
 **************************************/
#define STEPSIZE sizeof(size_t)

namespace LZ4
{

    static unsigned LZ4_64bits(void)
    {
        return sizeof(void*)==8;
    }

    static unsigned LZ4_isLittleEndian(void)
    {
        const union
        {
            u32 i;
            u8 c[4];
        } one = { 1 };   /* don't use static : performance detrimental  */
        return one.c[0];
    }


    static u16 LZ4_read16(const void* memPtr)
    {
        u16 val16;
        memcpy(&val16, memPtr, 2);
        return val16;
    }

    static u16 LZ4_readLE16(const void* memPtr)
    {
        if (LZ4_isLittleEndian())
        {
            return LZ4_read16(memPtr);
        }
        else
        {
            const u8* p = (const u8*)memPtr;
            return (u16)((u16)p[0] + (p[1]<<8));
        }
    }

    static void LZ4_writeLE16(void* memPtr, u16 value)
    {
        if (LZ4_isLittleEndian())
        {
            memcpy(memPtr, &value, 2);
        }
        else
        {
            u8* p = (u8*)memPtr;
            p[0] = (u8) value;
            p[1] = (u8)(value>>8);
        }
    }

    static u32 LZ4_read32(const void* memPtr)
    {
        u32 val32;
        memcpy(&val32, memPtr, 4);
        return val32;
    }

    static u64 LZ4_read64(const void* memPtr)
    {
        u64 val64;
        memcpy(&val64, memPtr, 8);
        return val64;
    }

    static size_t LZ4_read_ARCH(const void* p)
    {
        if (LZ4_64bits())
            return (size_t)LZ4_read64(p);
        else
            return (size_t)LZ4_read32(p);
    }


    static void LZ4_copy4(void* dstPtr, const void* srcPtr)
    {
        memcpy(dstPtr, srcPtr, 4);
    }

    static void LZ4_copy8(void* dstPtr, const void* srcPtr)
    {
        memcpy(dstPtr, srcPtr, 8);
    }

    /* customized version of memcpy, which may overwrite up to 7 u8s beyond dstEnd */
    static void LZ4_wildCopy(void* dstPtr, const void* srcPtr, void* dstEnd)
    {
        u8* d = (u8*)dstPtr;
        const u8* s = (const u8*)srcPtr;
        u8* e = (u8*)dstEnd;
        do
        {
            LZ4_copy8(d,s);
            d+=8;
            s+=8;
        }
        while (d<e);
    }


    /**************************************
     *  Common Constants
     **************************************/
#define MINMATCH 4

#define COPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (COPYLENGTH+MINMATCH)
    static const int LZ4_minLength = (MFLIMIT+1);

#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)

#define MAXD_LOG 16
#define MAX_DISTANCE ((1 << MAXD_LOG) - 1)

#define ML_BITS  4
#define ML_MASK  ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)


    /**************************************
     *  Common Utils
     **************************************/
#define LZ4_STATIC_ASSERT(c)    { enum { LZ4_static_assert = 1/(int)(!!(c)) }; }   /* use only *after* variable declarations */


    /**************************************
     *  Common functions
     **************************************/
    static unsigned LZ4_NbCommonu8s (register size_t val)
    {
        if (LZ4_isLittleEndian())
        {
            if (LZ4_64bits())
            {
#       if defined(_MSC_VER) && defined(_WIN64) && !defined(LZ4_FORCE_SW_BITCOUNT)
                unsigned long r = 0;
                _BitScanForward64( &r, (u64)val );
                return (int)(r>>3);
#       elif (defined(__clang__) || (LZ4_GCC_VERSION >= 304)) && !defined(LZ4_FORCE_SW_BITCOUNT)
                return (__builtin_ctzll((u64)val) >> 3);
#       else
                static const int DeBruijnu8Pos[64] = { 0, 0, 0, 0, 0, 1, 1, 2, 0, 3, 1, 3, 1, 4, 2, 7, 0, 2, 3, 6, 1, 5, 3, 5, 1, 3, 4, 4, 2, 5, 6, 7, 7, 0, 1, 2, 3, 3, 4, 6, 2, 6, 5, 5, 3, 4, 5, 6, 7, 1, 2, 4, 6, 4, 4, 5, 7, 2, 6, 5, 7, 6, 7, 7 };
                return DeBruijnu8Pos[((u64)((val & -(long long)val) * 0x0218A392CDABBD3FULL)) >> 58];
#       endif
            }
            else /* 32 bits */
            {
#       if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
                unsigned long r;
                _BitScanForward( &r, (u32)val );
                return (int)(r>>3);
#       elif (defined(__clang__) || (LZ4_GCC_VERSION >= 304)) && !defined(LZ4_FORCE_SW_BITCOUNT)
                return (__builtin_ctz((u32)val) >> 3);
#       else
                static const int DeBruijnu8Pos[32] = { 0, 0, 3, 0, 3, 1, 3, 0, 3, 2, 2, 1, 3, 2, 0, 1, 3, 3, 1, 2, 2, 2, 2, 0, 3, 1, 2, 0, 1, 0, 1, 1 };
                return DeBruijnu8Pos[((u32)((val & -(s32)val) * 0x077CB531U)) >> 27];
#       endif
            }
        }
        else   /* Big Endian CPU */
        {
            if (LZ4_64bits())
            {
#       if defined(_MSC_VER) && defined(_WIN64) && !defined(LZ4_FORCE_SW_BITCOUNT)
                unsigned long r = 0;
                _BitScanReverse64( &r, val );
                return (unsigned)(r>>3);
#       elif (defined(__clang__) || (LZ4_GCC_VERSION >= 304)) && !defined(LZ4_FORCE_SW_BITCOUNT)
                return (__builtin_clzll((u64)val) >> 3);
#       else
                unsigned r;
                if (!(val>>32))
                {
                    r=4;
                }
                else
                {
                    r=0;
                    val>>=32;
                }
                if (!(val>>16))
                {
                    r+=2;
                    val>>=8;
                }
                else
                {
                    val>>=24;
                }
                r += (!val);
                return r;
#       endif
            }
            else /* 32 bits */
            {
#       if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
                unsigned long r = 0;
                _BitScanReverse( &r, (unsigned long)val );
                return (unsigned)(r>>3);
#       elif (defined(__clang__) || (LZ4_GCC_VERSION >= 304)) && !defined(LZ4_FORCE_SW_BITCOUNT)
                return (__builtin_clz((u32)val) >> 3);
#       else
                unsigned r;
                if (!(val>>16))
                {
                    r=2;
                    val>>=8;
                }
                else
                {
                    r=0;
                    val>>=24;
                }
                r += (!val);
                return r;
#       endif
            }
        }
    }

    static unsigned LZ4_count(const u8* pIn, const u8* pMatch, const u8* pInLimit)
    {
        const u8* const pStart = pIn;

        while (likely(pIn<pInLimit-(STEPSIZE-1)))
        {
            size_t diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
            if (!diff)
            {
                pIn+=STEPSIZE;
                pMatch+=STEPSIZE;
                continue;
            }
            pIn += LZ4_NbCommonu8s(diff);
            return (unsigned)(pIn - pStart);
        }

        if (LZ4_64bits()) if ((pIn<(pInLimit-3)) && (LZ4_read32(pMatch) == LZ4_read32(pIn)))
            {
                pIn+=4;
                pMatch+=4;
            }
        if ((pIn<(pInLimit-1)) && (LZ4_read16(pMatch) == LZ4_read16(pIn)))
        {
            pIn+=2;
            pMatch+=2;
        }
        if ((pIn<pInLimit) && (*pMatch == *pIn)) pIn++;
        return (unsigned)(pIn - pStart);
    }


#ifndef LZ4_COMMONDEFS_ONLY
    /**************************************
     *  Local Constants
     **************************************/
#define LZ4_HASHLOG   (LZ4_MEMORY_USAGE-2)
#define HASHTABLESIZE (1 << LZ4_MEMORY_USAGE)
#define HASH_SIZE_u32 (1 << LZ4_HASHLOG)       /* required as macro for static allocation */

    static const int LZ4_64Klimit = ((64 KB) + (MFLIMIT-1));
    static const u32 LZ4_skipTrigger = 6;  /* Increase this value ==> compression run slower on incompressible data */


    /**************************************
     *  Local Structures and types
     **************************************/
    typedef struct
    {
        u32 hashTable[HASH_SIZE_u32];
        u32 currentOffset;
        u32 initCheck;
        const u8* dictionary;
        u8* bufferStart;   /* obsolete, used for slideInputBuffer */
        u32 dictSize;
    } LZ4_stream_t_internal;

    typedef enum { notLimited = 0, limitedOutput = 1 } limitedOutput_directive;
    typedef enum { byPtr, byu32, byu16 } tableType_t;

    typedef enum { noDict = 0, withPrefix64k, usingExtDict } dict_directive;
    typedef enum { noDictIssue = 0, dictSmall } dictIssue_directive;

    typedef enum { endOnOutputSize = 0, endOnInputSize = 1 } endCondition_directive;
    typedef enum { full = 0, partial = 1 } earlyEnd_directive;


    /**************************************
     *  Local Utils
     **************************************/
    int LZ4_versionNumber (void)
    {
        return LZ4_VERSION_NUMBER;
    }
    int LZ4_compressBound(int isize)
    {
        return LZ4_COMPRESSBOUND(isize);
    }
    int LZ4_sizeofState()
    {
        return LZ4_STREAMSIZE;
    }



    /********************************
     *  Compression functions
     ********************************/

    static u32 LZ4_hashSequence(u32 sequence, tableType_t const tableType)
    {
        if (tableType == byu16)
            return (((sequence) * 2654435761U) >> ((MINMATCH*8)-(LZ4_HASHLOG+1)));
        else
            return (((sequence) * 2654435761U) >> ((MINMATCH*8)-LZ4_HASHLOG));
    }

    static const u64 prime5u8s = 889523592379ULL;
    static u32 LZ4_hashSequence64(size_t sequence, tableType_t const tableType)
    {
        const u32 hashLog = (tableType == byu16) ? LZ4_HASHLOG+1 : LZ4_HASHLOG;
        const u32 hashMask = (1<<hashLog) - 1;
        return ((sequence * prime5u8s) >> (40 - hashLog)) & hashMask;
    }

    static u32 LZ4_hashSequenceT(size_t sequence, tableType_t const tableType)
    {
        if (LZ4_64bits())
            return LZ4_hashSequence64(sequence, tableType);
        return LZ4_hashSequence((u32)sequence, tableType);
    }

    static u32 LZ4_hashPosition(const void* p, tableType_t tableType)
    {
        return LZ4_hashSequenceT(LZ4_read_ARCH(p), tableType);
    }

    static void LZ4_putPositionOnHash(const u8* p, u32 h, void* tableBase, tableType_t const tableType, const u8* srcBase)
    {
        switch (tableType)
        {
        case byPtr:
        {
            const u8** hashTable = (const u8**)tableBase;
            hashTable[h] = p;
            return;
        }
        case byu32:
        {
            u32* hashTable = (u32*) tableBase;
            hashTable[h] = (u32)(p-srcBase);
            return;
        }
        case byu16:
        {
            u16* hashTable = (u16*) tableBase;
            hashTable[h] = (u16)(p-srcBase);
            return;
        }
        }
    }

    static void LZ4_putPosition(const u8* p, void* tableBase, tableType_t tableType, const u8* srcBase)
    {
        u32 h = LZ4_hashPosition(p, tableType);
        LZ4_putPositionOnHash(p, h, tableBase, tableType, srcBase);
    }

    static const u8* LZ4_getPositionOnHash(u32 h, void* tableBase, tableType_t tableType, const u8* srcBase)
    {
        if (tableType == byPtr)
        {
            const u8** hashTable = (const u8**) tableBase;
            return hashTable[h];
        }
        if (tableType == byu32)
        {
            u32* hashTable = (u32*) tableBase;
            return hashTable[h] + srcBase;
        }
        {
            u16* hashTable = (u16*) tableBase;    /* default, to ensure a return */
            return hashTable[h] + srcBase;
        }
    }

    static const u8* LZ4_getPosition(const u8* p, void* tableBase, tableType_t tableType, const u8* srcBase)
    {
        u32 h = LZ4_hashPosition(p, tableType);
        return LZ4_getPositionOnHash(h, tableBase, tableType, srcBase);
    }

    FORCE_INLINE int LZ4_compress_generic(
        void* const ctx,
        const char* const source,
        char* const dest,
        const int inputSize,
        const int maxOutputSize,
        const limitedOutput_directive outputLimited,
        const tableType_t tableType,
        const dict_directive dict,
        const dictIssue_directive dictIssue,
        const u32 acceleration)
    {
        LZ4_stream_t_internal* const dictPtr = (LZ4_stream_t_internal*)ctx;

        const u8* ip = (const u8*) source;
        const u8* base;
        const u8* lowLimit;
        const u8* const lowRefLimit = ip - dictPtr->dictSize;
        const u8* const dictionary = dictPtr->dictionary;
        const u8* const dictEnd = dictionary + dictPtr->dictSize;
        const size_t dictDelta = dictEnd - (const u8*)source;
        const u8* anchor = (const u8*) source;
        const u8* const iend = ip + inputSize;
        const u8* const mflimit = iend - MFLIMIT;
        const u8* const matchlimit = iend - LASTLITERALS;

        u8* op = (u8*) dest;
        u8* const olimit = op + maxOutputSize;

        u32 forwardH;
        size_t refDelta=0;

        /* Init conditions */
        if ((u32)inputSize > (u32)LZ4_MAX_INPUT_SIZE) return 0;   /* Unsupported input size, too large (or negative) */
        switch(dict)
        {
        case noDict:
        default:
            base = (const u8*)source;
            lowLimit = (const u8*)source;
            break;
        case withPrefix64k:
            base = (const u8*)source - dictPtr->currentOffset;
            lowLimit = (const u8*)source - dictPtr->dictSize;
            break;
        case usingExtDict:
            base = (const u8*)source - dictPtr->currentOffset;
            lowLimit = (const u8*)source;
            break;
        }
        if ((tableType == byu16) && (inputSize>=LZ4_64Klimit)) return 0;   /* Size too large (not within 64K limit) */
        if (inputSize<LZ4_minLength) goto _last_literals;                  /* Input too small, no compression (all literals) */

        /* First u8 */
        LZ4_putPosition(ip, ctx, tableType, base);
        ip++;
        forwardH = LZ4_hashPosition(ip, tableType);

        /* Main Loop */
        for ( ; ; )
        {
            const u8* match;
            u8* token;
            {
                const u8* forwardIp = ip;
                unsigned step = 1;
                unsigned searchMatchNb = acceleration << LZ4_skipTrigger;

                /* Find a match */
                do
                {
                    u32 h = forwardH;
                    ip = forwardIp;
                    forwardIp += step;
                    step = (searchMatchNb++ >> LZ4_skipTrigger);

                    if (unlikely(forwardIp > mflimit)) goto _last_literals;

                    match = LZ4_getPositionOnHash(h, ctx, tableType, base);
                    if (dict==usingExtDict)
                    {
                        if (match<(const u8*)source)
                        {
                            refDelta = dictDelta;
                            lowLimit = dictionary;
                        }
                        else
                        {
                            refDelta = 0;
                            lowLimit = (const u8*)source;
                        }
                    }
                    forwardH = LZ4_hashPosition(forwardIp, tableType);
                    LZ4_putPositionOnHash(ip, h, ctx, tableType, base);

                }
                while ( ((dictIssue==dictSmall) ? (match < lowRefLimit) : 0)
                        || ((tableType==byu16) ? 0 : (match + MAX_DISTANCE < ip))
                        || (LZ4_read32(match+refDelta) != LZ4_read32(ip)) );
            }

            /* Catch up */
            while ((ip>anchor) && (match+refDelta > lowLimit) && (unlikely(ip[-1]==match[refDelta-1])))
            {
                ip--;
                match--;
            }

            {
                /* Encode Literal length */
                unsigned litLength = (unsigned)(ip - anchor);
                token = op++;
                if ((outputLimited) && (unlikely(op + litLength + (2 + 1 + LASTLITERALS) + (litLength/255) > olimit)))
                    return 0;   /* Check output limit */
                if (litLength>=RUN_MASK)
                {
                    int len = (int)litLength-RUN_MASK;
                    *token=(RUN_MASK<<ML_BITS);
                    for(; len >= 255 ; len-=255) *op++ = 255;
                    *op++ = (u8)len;
                }
                else *token = (u8)(litLength<<ML_BITS);

                /* Copy Literals */
                LZ4_wildCopy(op, anchor, op+litLength);
                op+=litLength;
            }

_next_match:
            /* Encode Offset */
            LZ4_writeLE16(op, (u16)(ip-match));
            op+=2;

            /* Encode MatchLength */
            {
                unsigned matchLength;

                if ((dict==usingExtDict) && (lowLimit==dictionary))
                {
                    const u8* limit;
                    match += refDelta;
                    limit = ip + (dictEnd-match);
                    if (limit > matchlimit) limit = matchlimit;
                    matchLength = LZ4_count(ip+MINMATCH, match+MINMATCH, limit);
                    ip += MINMATCH + matchLength;
                    if (ip==limit)
                    {
                        unsigned more = LZ4_count(ip, (const u8*)source, matchlimit);
                        matchLength += more;
                        ip += more;
                    }
                }
                else
                {
                    matchLength = LZ4_count(ip+MINMATCH, match+MINMATCH, matchlimit);
                    ip += MINMATCH + matchLength;
                }

                if ((outputLimited) && (unlikely(op + (1 + LASTLITERALS) + (matchLength>>8) > olimit)))
                    return 0;    /* Check output limit */
                if (matchLength>=ML_MASK)
                {
                    *token += ML_MASK;
                    matchLength -= ML_MASK;
                    for (; matchLength >= 510 ; matchLength-=510)
                    {
                        *op++ = 255;
                        *op++ = 255;
                    }
                    if (matchLength >= 255)
                    {
                        matchLength-=255;
                        *op++ = 255;
                    }
                    *op++ = (u8)matchLength;
                }
                else *token += (u8)(matchLength);
            }

            anchor = ip;

            /* Test end of chunk */
            if (ip > mflimit) break;

            /* Fill table */
            LZ4_putPosition(ip-2, ctx, tableType, base);

            /* Test next position */
            match = LZ4_getPosition(ip, ctx, tableType, base);
            if (dict==usingExtDict)
            {
                if (match<(const u8*)source)
                {
                    refDelta = dictDelta;
                    lowLimit = dictionary;
                }
                else
                {
                    refDelta = 0;
                    lowLimit = (const u8*)source;
                }
            }
            LZ4_putPosition(ip, ctx, tableType, base);
            if ( ((dictIssue==dictSmall) ? (match>=lowRefLimit) : 1)
                    && (match+MAX_DISTANCE>=ip)
                    && (LZ4_read32(match+refDelta)==LZ4_read32(ip)) )
            {
                token=op++;
                *token=0;
                goto _next_match;
            }

            /* Prepare next loop */
            forwardH = LZ4_hashPosition(++ip, tableType);
        }

_last_literals:
        /* Encode Last Literals */
        {
            const size_t lastRun = (size_t)(iend - anchor);
            if ((outputLimited) && ((op - (u8*)dest) + lastRun + 1 + ((lastRun+255-RUN_MASK)/255) > (u32)maxOutputSize))
                return 0;   /* Check output limit */
            if (lastRun >= RUN_MASK)
            {
                size_t accumulator = lastRun - RUN_MASK;
                *op++ = RUN_MASK << ML_BITS;
                for(; accumulator >= 255 ; accumulator-=255) *op++ = 255;
                *op++ = (u8) accumulator;
            }
            else
            {
                *op++ = (u8)(lastRun<<ML_BITS);
            }
            memcpy(op, anchor, lastRun);
            op += lastRun;
        }

        /* End */
        return (int) (((char*)op)-dest);
    }


    int LZ4_compress_fast_extState(void* state, const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
    {
        LZ4_resetStream((LZ4_stream_t*)state);
        if (acceleration < 1) acceleration = ACCELERATION_DEFAULT;

        if (maxOutputSize >= LZ4_compressBound(inputSize))
        {
            if (inputSize < LZ4_64Klimit)
                return LZ4_compress_generic(state, source, dest, inputSize, 0, notLimited, byu16,                        noDict, noDictIssue, acceleration);
            else
                return LZ4_compress_generic(state, source, dest, inputSize, 0, notLimited, LZ4_64bits() ? byu32 : byPtr, noDict, noDictIssue, acceleration);
        }
        else
        {
            if (inputSize < LZ4_64Klimit)
                return LZ4_compress_generic(state, source, dest, inputSize, maxOutputSize, limitedOutput, byu16,                        noDict, noDictIssue, acceleration);
            else
                return LZ4_compress_generic(state, source, dest, inputSize, maxOutputSize, limitedOutput, LZ4_64bits() ? byu32 : byPtr, noDict, noDictIssue, acceleration);
        }
    }


    int LZ4_compress_fast(const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
    {
#if (HEAPMODE)
        void* ctxPtr = ALLOCATOR(1, sizeof(LZ4_stream_t));   /* malloc-calloc always properly aligned */
#else
        LZ4_stream_t ctx;
        void* ctxPtr = &ctx;
#endif

        int result = LZ4_compress_fast_extState(ctxPtr, source, dest, inputSize, maxOutputSize, acceleration);

#if (HEAPMODE)
        FREEMEM(ctxPtr);
#endif
        return result;
    }


    int LZ4_compress_default(const char* source, char* dest, int inputSize, int maxOutputSize)
    {
        return LZ4_compress_fast(source, dest, inputSize, maxOutputSize, 1);
    }


    /* hidden debug function */
    /* strangely enough, gcc generates faster code when this function is uncommented, even if unused */
    int LZ4_compress_fast_force(const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
    {
        LZ4_stream_t ctx;

        LZ4_resetStream(&ctx);

        if (inputSize < LZ4_64Klimit)
            return LZ4_compress_generic(&ctx, source, dest, inputSize, maxOutputSize, limitedOutput, byu16,                        noDict, noDictIssue, acceleration);
        else
            return LZ4_compress_generic(&ctx, source, dest, inputSize, maxOutputSize, limitedOutput, LZ4_64bits() ? byu32 : byPtr, noDict, noDictIssue, acceleration);
    }


    /********************************
     *  destSize variant
     ********************************/

    static int LZ4_compress_destSize_generic(
        void* const ctx,
        const char* const src,
        char* const dst,
        int*  const srcSizePtr,
        const int targetDstSize,
        const tableType_t tableType)
    {
        const u8* ip = (const u8*) src;
        const u8* base = (const u8*) src;
        const u8* lowLimit = (const u8*) src;
        const u8* anchor = ip;
        const u8* const iend = ip + *srcSizePtr;
        const u8* const mflimit = iend - MFLIMIT;
        const u8* const matchlimit = iend - LASTLITERALS;

        u8* op = (u8*) dst;
        u8* const oend = op + targetDstSize;
        u8* const oMaxLit = op + targetDstSize - 2 /* offset */ - 8 /* because 8+MINMATCH==MFLIMIT */ - 1 /* token */;
        u8* const oMaxMatch = op + targetDstSize - (LASTLITERALS + 1 /* token */);
        u8* const oMaxSeq = oMaxLit - 1 /* token */;

        u32 forwardH;


        /* Init conditions */
        if (targetDstSize < 1) return 0;                                     /* Impossible to store anything */
        if ((u32)*srcSizePtr > (u32)LZ4_MAX_INPUT_SIZE) return 0;            /* Unsupported input size, too large (or negative) */
        if ((tableType == byu16) && (*srcSizePtr>=LZ4_64Klimit)) return 0;   /* Size too large (not within 64K limit) */
        if (*srcSizePtr<LZ4_minLength) goto _last_literals;                  /* Input too small, no compression (all literals) */

        /* First u8 */
        *srcSizePtr = 0;
        LZ4_putPosition(ip, ctx, tableType, base);
        ip++;
        forwardH = LZ4_hashPosition(ip, tableType);

        /* Main Loop */
        for ( ; ; )
        {
            const u8* match;
            u8* token;
            {
                const u8* forwardIp = ip;
                unsigned step = 1;
                unsigned searchMatchNb = 1 << LZ4_skipTrigger;

                /* Find a match */
                do
                {
                    u32 h = forwardH;
                    ip = forwardIp;
                    forwardIp += step;
                    step = (searchMatchNb++ >> LZ4_skipTrigger);

                    if (unlikely(forwardIp > mflimit))
                        goto _last_literals;

                    match = LZ4_getPositionOnHash(h, ctx, tableType, base);
                    forwardH = LZ4_hashPosition(forwardIp, tableType);
                    LZ4_putPositionOnHash(ip, h, ctx, tableType, base);

                }
                while ( ((tableType==byu16) ? 0 : (match + MAX_DISTANCE < ip))
                        || (LZ4_read32(match) != LZ4_read32(ip)) );
            }

            /* Catch up */
            while ((ip>anchor) && (match > lowLimit) && (unlikely(ip[-1]==match[-1])))
            {
                ip--;
                match--;
            }

            {
                /* Encode Literal length */
                unsigned litLength = (unsigned)(ip - anchor);
                token = op++;
                if (op + ((litLength+240)/255) + litLength > oMaxLit)
                {
                    /* Not enough space for a last match */
                    op--;
                    goto _last_literals;
                }
                if (litLength>=RUN_MASK)
                {
                    unsigned len = litLength - RUN_MASK;
                    *token=(RUN_MASK<<ML_BITS);
                    for(; len >= 255 ; len-=255) *op++ = 255;
                    *op++ = (u8)len;
                }
                else *token = (u8)(litLength<<ML_BITS);

                /* Copy Literals */
                LZ4_wildCopy(op, anchor, op+litLength);
                op += litLength;
            }

_next_match:
            /* Encode Offset */
            LZ4_writeLE16(op, (u16)(ip-match));
            op+=2;

            /* Encode MatchLength */
            {
                size_t matchLength;

                matchLength = LZ4_count(ip+MINMATCH, match+MINMATCH, matchlimit);

                if (op + ((matchLength+240)/255) > oMaxMatch)
                {
                    /* Match description too long : reduce it */
                    matchLength = (15-1) + (oMaxMatch-op) * 255;
                }
                //printf("offset %5i, matchLength%5i \n", (int)(ip-match), matchLength + MINMATCH);
                ip += MINMATCH + matchLength;

                if (matchLength>=ML_MASK)
                {
                    *token += ML_MASK;
                    matchLength -= ML_MASK;
                    while (matchLength >= 255)
                    {
                        matchLength-=255;
                        *op++ = 255;
                    }
                    *op++ = (u8)matchLength;
                }
                else *token += (u8)(matchLength);
            }

            anchor = ip;

            /* Test end of block */
            if (ip > mflimit) break;
            if (op > oMaxSeq) break;

            /* Fill table */
            LZ4_putPosition(ip-2, ctx, tableType, base);

            /* Test next position */
            match = LZ4_getPosition(ip, ctx, tableType, base);
            LZ4_putPosition(ip, ctx, tableType, base);
            if ( (match+MAX_DISTANCE>=ip)
                    && (LZ4_read32(match)==LZ4_read32(ip)) )
            {
                token=op++;
                *token=0;
                goto _next_match;
            }

            /* Prepare next loop */
            forwardH = LZ4_hashPosition(++ip, tableType);
        }

_last_literals:
        /* Encode Last Literals */
        {
            size_t lastRunSize = (size_t)(iend - anchor);
            if (op + 1 /* token */ + ((lastRunSize+240)/255) /* litLength */ + lastRunSize /* literals */ > oend)
            {
                /* adapt lastRunSize to fill 'dst' */
                lastRunSize  = (oend-op) - 1;
                lastRunSize -= (lastRunSize+240)/255;
            }
            ip = anchor + lastRunSize;

            if (lastRunSize >= RUN_MASK)
            {
                size_t accumulator = lastRunSize - RUN_MASK;
                *op++ = RUN_MASK << ML_BITS;
                for(; accumulator >= 255 ; accumulator-=255) *op++ = 255;
                *op++ = (u8) accumulator;
            }
            else
            {
                *op++ = (u8)(lastRunSize<<ML_BITS);
            }
            memcpy(op, anchor, lastRunSize);
            op += lastRunSize;
        }

        /* End */
        *srcSizePtr = (int) (((const char*)ip)-src);
        return (int) (((char*)op)-dst);
    }


    static int LZ4_compress_destSize_extState (void* state, const char* src, char* dst, int* srcSizePtr, int targetDstSize)
    {
        LZ4_resetStream((LZ4_stream_t*)state);

        if (targetDstSize >= LZ4_compressBound(*srcSizePtr))   /* compression success is guaranteed */
        {
            return LZ4_compress_fast_extState(state, src, dst, *srcSizePtr, targetDstSize, 1);
        }
        else
        {
            if (*srcSizePtr < LZ4_64Klimit)
                return LZ4_compress_destSize_generic(state, src, dst, srcSizePtr, targetDstSize, byu16);
            else
                return LZ4_compress_destSize_generic(state, src, dst, srcSizePtr, targetDstSize, LZ4_64bits() ? byu32 : byPtr);
        }
    }


    int LZ4_compress_destSize(const char* src, char* dst, int* srcSizePtr, int targetDstSize)
    {
#if (HEAPMODE)
        void* ctx = ALLOCATOR(1, sizeof(LZ4_stream_t));   /* malloc-calloc always properly aligned */
#else
        LZ4_stream_t ctxBody;
        void* ctx = &ctxBody;
#endif

        int result = LZ4_compress_destSize_extState(ctx, src, dst, srcSizePtr, targetDstSize);

#if (HEAPMODE)
        FREEMEM(ctx);
#endif
        return result;
    }

    /********************************
     *  Streaming functions
     ********************************/

    LZ4_stream_t* LZ4_createStream(void)
    {
        LZ4_stream_t* lz4s = (LZ4_stream_t*)ALLOCATOR(8, LZ4_STREAMSIZE_U64);
        LZ4_STATIC_ASSERT(LZ4_STREAMSIZE >= sizeof(LZ4_stream_t_internal));    /* A compilation error here means LZ4_STREAMSIZE is not large enough */
        LZ4_resetStream(lz4s);
        return lz4s;
    }

    void LZ4_resetStream (LZ4_stream_t* LZ4_stream)
    {
        MEM_INIT(LZ4_stream, 0, sizeof(LZ4_stream_t));
    }

    int LZ4_freeStream (LZ4_stream_t* LZ4_stream)
    {
        FREEMEM(LZ4_stream);
        return (0);
    }


#define HASH_UNIT sizeof(size_t)
    int LZ4_loadDict (LZ4_stream_t* LZ4_dict, const char* dictionary, int dictSize)
    {
        LZ4_stream_t_internal* dict = (LZ4_stream_t_internal*) LZ4_dict;
        const u8* p = (const u8*)dictionary;
        const u8* const dictEnd = p + dictSize;
        const u8* base;

        if ((dict->initCheck) || (dict->currentOffset > 1 GB))  /* Uninitialized structure, or reuse overflow */
            LZ4_resetStream(LZ4_dict);

        if (dictSize < (int)HASH_UNIT)
        {
            dict->dictionary = NULL;
            dict->dictSize = 0;
            return 0;
        }

        if ((dictEnd - p) > 64 KB) p = dictEnd - 64 KB;
        dict->currentOffset += 64 KB;
        base = p - dict->currentOffset;
        dict->dictionary = p;
        dict->dictSize = (u32)(dictEnd - p);
        dict->currentOffset += dict->dictSize;

        while (p <= dictEnd-HASH_UNIT)
        {
            LZ4_putPosition(p, dict->hashTable, byu32, base);
            p+=3;
        }

        return dict->dictSize;
    }


    static void LZ4_renormDictT(LZ4_stream_t_internal* LZ4_dict, const u8* src)
    {
        if ((LZ4_dict->currentOffset > 0x80000000) ||
                ((size_t)LZ4_dict->currentOffset > (size_t)src))   /* address space overflow */
        {
            /* rescale hash table */
            u32 delta = LZ4_dict->currentOffset - 64 KB;
            const u8* dictEnd = LZ4_dict->dictionary + LZ4_dict->dictSize;
            int i;
            for (i=0; i<HASH_SIZE_u32; i++)
            {
                if (LZ4_dict->hashTable[i] < delta) LZ4_dict->hashTable[i]=0;
                else LZ4_dict->hashTable[i] -= delta;
            }
            LZ4_dict->currentOffset = 64 KB;
            if (LZ4_dict->dictSize > 64 KB) LZ4_dict->dictSize = 64 KB;
            LZ4_dict->dictionary = dictEnd - LZ4_dict->dictSize;
        }
    }


    int LZ4_compress_fast_continue (LZ4_stream_t* LZ4_stream, const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
    {
        LZ4_stream_t_internal* streamPtr = (LZ4_stream_t_internal*)LZ4_stream;
        const u8* const dictEnd = streamPtr->dictionary + streamPtr->dictSize;

        const u8* smallest = (const u8*) source;
        if (streamPtr->initCheck) return 0;   /* Uninitialized structure detected */
        if ((streamPtr->dictSize>0) && (smallest>dictEnd)) smallest = dictEnd;
        LZ4_renormDictT(streamPtr, smallest);
        if (acceleration < 1) acceleration = ACCELERATION_DEFAULT;

        /* Check overlapping input/dictionary space */
        {
            const u8* sourceEnd = (const u8*) source + inputSize;
            if ((sourceEnd > streamPtr->dictionary) && (sourceEnd < dictEnd))
            {
                streamPtr->dictSize = (u32)(dictEnd - sourceEnd);
                if (streamPtr->dictSize > 64 KB) streamPtr->dictSize = 64 KB;
                if (streamPtr->dictSize < 4) streamPtr->dictSize = 0;
                streamPtr->dictionary = dictEnd - streamPtr->dictSize;
            }
        }

        /* prefix mode : source data follows dictionary */
        if (dictEnd == (const u8*)source)
        {
            int result;
            if ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset))
                result = LZ4_compress_generic(LZ4_stream, source, dest, inputSize, maxOutputSize, limitedOutput, byu32, withPrefix64k, dictSmall, acceleration);
            else
                result = LZ4_compress_generic(LZ4_stream, source, dest, inputSize, maxOutputSize, limitedOutput, byu32, withPrefix64k, noDictIssue, acceleration);
            streamPtr->dictSize += (u32)inputSize;
            streamPtr->currentOffset += (u32)inputSize;
            return result;
        }

        /* external dictionary mode */
        {
            int result;
            if ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset))
                result = LZ4_compress_generic(LZ4_stream, source, dest, inputSize, maxOutputSize, limitedOutput, byu32, usingExtDict, dictSmall, acceleration);
            else
                result = LZ4_compress_generic(LZ4_stream, source, dest, inputSize, maxOutputSize, limitedOutput, byu32, usingExtDict, noDictIssue, acceleration);
            streamPtr->dictionary = (const u8*)source;
            streamPtr->dictSize = (u32)inputSize;
            streamPtr->currentOffset += (u32)inputSize;
            return result;
        }
    }


    /* Hidden debug function, to force external dictionary mode */
    int LZ4_compress_forceExtDict (LZ4_stream_t* LZ4_dict, const char* source, char* dest, int inputSize)
    {
        LZ4_stream_t_internal* streamPtr = (LZ4_stream_t_internal*)LZ4_dict;
        int result;
        const u8* const dictEnd = streamPtr->dictionary + streamPtr->dictSize;

        const u8* smallest = dictEnd;
        if (smallest > (const u8*) source) smallest = (const u8*) source;
        LZ4_renormDictT((LZ4_stream_t_internal*)LZ4_dict, smallest);

        result = LZ4_compress_generic(LZ4_dict, source, dest, inputSize, 0, notLimited, byu32, usingExtDict, noDictIssue, 1);

        streamPtr->dictionary = (const u8*)source;
        streamPtr->dictSize = (u32)inputSize;
        streamPtr->currentOffset += (u32)inputSize;

        return result;
    }


    int LZ4_saveDict (LZ4_stream_t* LZ4_dict, char* safeBuffer, int dictSize)
    {
        LZ4_stream_t_internal* dict = (LZ4_stream_t_internal*) LZ4_dict;
        const u8* previousDictEnd = dict->dictionary + dict->dictSize;

        if ((u32)dictSize > 64 KB) dictSize = 64 KB;   /* useless to define a dictionary > 64 KB */
        if ((u32)dictSize > dict->dictSize) dictSize = dict->dictSize;

        memmove(safeBuffer, previousDictEnd - dictSize, dictSize);

        dict->dictionary = (const u8*)safeBuffer;
        dict->dictSize = (u32)dictSize;

        return dictSize;
    }



    /*******************************
     *  Decompression functions
     *******************************/
    /*
     * This generic decompression function cover all use cases.
     * It shall be instantiated several times, using different sets of directives
     * Note that it is essential this generic function is really inlined,
     * in order to remove useless branches during compilation optimization.
     */
    FORCE_INLINE int LZ4_decompress_generic(
        const char* const source,
        char* const dest,
        int inputSize,
        int outputSize,         /* If endOnInput==endOnInputSize, this value is the max size of Output Buffer. */

        int endOnInput,         /* endOnOutputSize, endOnInputSize */
        int partialDecoding,    /* full, partial */
        int targetOutputSize,   /* only used if partialDecoding==partial */
        int dict,               /* noDict, withPrefix64k, usingExtDict */
        const u8* const lowPrefix,  /* == dest if dict == noDict */
        const u8* const dictStart,  /* only if dict==usingExtDict */
        const size_t dictSize         /* note : = 0 if noDict */
    )
    {
        /* Local Variables */
        const u8* ip = (const u8*) source;
        const u8* const iend = ip + inputSize;

        u8* op = (u8*) dest;
        u8* const oend = op + outputSize;
        u8* cpy;
        u8* oexit = op + targetOutputSize;
        const u8* const lowLimit = lowPrefix - dictSize;

        const u8* const dictEnd = (const u8*)dictStart + dictSize;
        const size_t dec32table[] = {4, 1, 2, 1, 4, 4, 4, 4};
        const size_t dec64table[] = {0, 0, 0, (size_t)-1, 0, 1, 2, 3};

        const int safeDecode = (endOnInput==endOnInputSize);
        const int checkOffset = ((safeDecode) && (dictSize < (int)(64 KB)));


        /* Special cases */
        if ((partialDecoding) && (oexit> oend-MFLIMIT)) oexit = oend-MFLIMIT;                         /* targetOutputSize too high => decode everything */
        if ((endOnInput) && (unlikely(outputSize==0))) return ((inputSize==1) && (*ip==0)) ? 0 : -1;  /* Empty output buffer */
        if ((!endOnInput) && (unlikely(outputSize==0))) return (*ip==0?1:-1);


        /* Main Loop */
        while (1)
        {
            unsigned token;
            size_t length;
            const u8* match;

            /* get literal length */
            token = *ip++;
            if ((length=(token>>ML_BITS)) == RUN_MASK)
            {
                unsigned s;
                do
                {
                    s = *ip++;
                    length += s;
                }
                while (likely((endOnInput)?ip<iend-RUN_MASK:1) && (s==255));
                if ((safeDecode) && unlikely((size_t)(op+length)<(size_t)(op))) goto _output_error;   /* overflow detection */
                if ((safeDecode) && unlikely((size_t)(ip+length)<(size_t)(ip))) goto _output_error;   /* overflow detection */
            }

            /* copy literals */
            cpy = op+length;
            if (((endOnInput) && ((cpy>(partialDecoding?oexit:oend-MFLIMIT)) || (ip+length>iend-(2+1+LASTLITERALS))) )
                    || ((!endOnInput) && (cpy>oend-COPYLENGTH)))
            {
                if (partialDecoding)
                {
                    if (cpy > oend) goto _output_error;                           /* Error : write attempt beyond end of output buffer */
                    if ((endOnInput) && (ip+length > iend)) goto _output_error;   /* Error : read attempt beyond end of input buffer */
                }
                else
                {
                    if ((!endOnInput) && (cpy != oend)) goto _output_error;       /* Error : block decoding must stop exactly there */
                    if ((endOnInput) && ((ip+length != iend) || (cpy > oend))) goto _output_error;   /* Error : input must be consumed */
                }
                memcpy(op, ip, length);
                ip += length;
                op += length;
                break;     /* Necessarily EOF, due to parsing restrictions */
            }
            LZ4_wildCopy(op, ip, cpy);
            ip += length;
            op = cpy;

            /* get offset */
            match = cpy - LZ4_readLE16(ip);
            ip+=2;
            if ((checkOffset) && (unlikely(match < lowLimit))) goto _output_error;   /* Error : offset outside destination buffer */

            /* get matchlength */
            length = token & ML_MASK;
            if (length == ML_MASK)
            {
                unsigned s;
                do
                {
                    if ((endOnInput) && (ip > iend-LASTLITERALS)) goto _output_error;
                    s = *ip++;
                    length += s;
                }
                while (s==255);
                if ((safeDecode) && unlikely((size_t)(op+length)<(size_t)op)) goto _output_error;   /* overflow detection */
            }
            length += MINMATCH;

            /* check external dictionary */
            if ((dict==usingExtDict) && (match < lowPrefix))
            {
                if (unlikely(op+length > oend-LASTLITERALS)) goto _output_error;   /* doesn't respect parsing restriction */

                if (length <= (size_t)(lowPrefix-match))
                {
                    /* match can be copied as a single segment from external dictionary */
                    match = dictEnd - (lowPrefix-match);
                    memmove(op, match, length);
                    op += length;
                }
                else
                {
                    /* match encompass external dictionary and current segment */
                    size_t copySize = (size_t)(lowPrefix-match);
                    memcpy(op, dictEnd - copySize, copySize);
                    op += copySize;
                    copySize = length - copySize;
                    if (copySize > (size_t)(op-lowPrefix))   /* overlap within current segment */
                    {
                        u8* const endOfMatch = op + copySize;
                        const u8* copyFrom = lowPrefix;
                        while (op < endOfMatch) *op++ = *copyFrom++;
                    }
                    else
                    {
                        memcpy(op, lowPrefix, copySize);
                        op += copySize;
                    }
                }
                continue;
            }

            /* copy repeated sequence */
            cpy = op + length;
            if (unlikely((op-match)<8))
            {
                const size_t dec64 = dec64table[op-match];
                op[0] = match[0];
                op[1] = match[1];
                op[2] = match[2];
                op[3] = match[3];
                match += dec32table[op-match];
                LZ4_copy4(op+4, match);
                op += 8;
                match -= dec64;
            }
            else
            {
                LZ4_copy8(op, match);
                op+=8;
                match+=8;
            }

            if (unlikely(cpy>oend-12))
            {
                if (cpy > oend-LASTLITERALS) goto _output_error;    /* Error : last LASTLITERALS u8s must be literals */
                if (op < oend-8)
                {
                    LZ4_wildCopy(op, match, oend-8);
                    match += (oend-8) - op;
                    op = oend-8;
                }
                while (op<cpy) *op++ = *match++;
            }
            else
                LZ4_wildCopy(op, match, cpy);
            op=cpy;   /* correction */
        }

        /* end of decoding */
        if (endOnInput)
            return (int) (((char*)op)-dest);     /* Nb of output u8s decoded */
        else
            return (int) (((const char*)ip)-source);   /* Nb of input u8s read */

        /* Overflow error detected */
_output_error:
        return (int) (-(((const char*)ip)-source))-1;
    }


    int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize)
    {
        return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize, endOnInputSize, full, 0, noDict, (u8*)dest, NULL, 0);
    }

    int LZ4_decompress_safe_partial(const char* source, char* dest, int compressedSize, int targetOutputSize, int maxDecompressedSize)
    {
        return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize, endOnInputSize, partial, targetOutputSize, noDict, (u8*)dest, NULL, 0);
    }

    int LZ4_decompress_fast(const char* source, char* dest, int originalSize)
    {
        return LZ4_decompress_generic(source, dest, 0, originalSize, endOnOutputSize, full, 0, withPrefix64k, (u8*)(dest - 64 KB), NULL, 64 KB);
    }


    /* streaming decompression functions */

    typedef struct
    {
        const u8* externalDict;
        size_t extDictSize;
        const u8* prefixEnd;
        size_t prefixSize;
    } LZ4_streamDecode_t_internal;

    /*
     * If you prefer dynamic allocation methods,
     * LZ4_createStreamDecode()
     * provides a pointer (void*) towards an initialized LZ4_streamDecode_t structure.
     */
    LZ4_streamDecode_t* LZ4_createStreamDecode(void)
    {
        LZ4_streamDecode_t* lz4s = (LZ4_streamDecode_t*) ALLOCATOR(1, sizeof(LZ4_streamDecode_t));
        return lz4s;
    }

    int LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream)
    {
        FREEMEM(LZ4_stream);
        return 0;
    }

    /*
     * LZ4_setStreamDecode
     * Use this function to instruct where to find the dictionary
     * This function is not necessary if previous data is still available where it was decoded.
     * Loading a size of 0 is allowed (same effect as no dictionary).
     * Return : 1 if OK, 0 if error
     */
    int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize)
    {
        LZ4_streamDecode_t_internal* lz4sd = (LZ4_streamDecode_t_internal*) LZ4_streamDecode;
        lz4sd->prefixSize = (size_t) dictSize;
        lz4sd->prefixEnd = (const u8*) dictionary + dictSize;
        lz4sd->externalDict = NULL;
        lz4sd->extDictSize  = 0;
        return 1;
    }

    /*
     *_continue() :
     These decoding functions allow decompression of multiple blocks in "streaming" mode.
     Previously decoded blocks must still be available at the memory position where they were decoded.
     If it's not possible, save the relevant part of decoded data into a safe buffer,
     and indicate where it stands using LZ4_setStreamDecode()
     */
    int LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int compressedSize, int maxOutputSize)
    {
        LZ4_streamDecode_t_internal* lz4sd = (LZ4_streamDecode_t_internal*) LZ4_streamDecode;
        int result;

        if (lz4sd->prefixEnd == (u8*)dest)
        {
            result = LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                            endOnInputSize, full, 0,
                                            usingExtDict, lz4sd->prefixEnd - lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
            if (result <= 0) return result;
            lz4sd->prefixSize += result;
            lz4sd->prefixEnd  += result;
        }
        else
        {
            lz4sd->extDictSize = lz4sd->prefixSize;
            lz4sd->externalDict = lz4sd->prefixEnd - lz4sd->extDictSize;
            result = LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                            endOnInputSize, full, 0,
                                            usingExtDict, (u8*)dest, lz4sd->externalDict, lz4sd->extDictSize);
            if (result <= 0) return result;
            lz4sd->prefixSize = result;
            lz4sd->prefixEnd  = (u8*)dest + result;
        }

        return result;
    }

    int LZ4_decompress_fast_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int originalSize)
    {
        LZ4_streamDecode_t_internal* lz4sd = (LZ4_streamDecode_t_internal*) LZ4_streamDecode;
        int result;

        if (lz4sd->prefixEnd == (u8*)dest)
        {
            result = LZ4_decompress_generic(source, dest, 0, originalSize,
                                            endOnOutputSize, full, 0,
                                            usingExtDict, lz4sd->prefixEnd - lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
            if (result <= 0) return result;
            lz4sd->prefixSize += originalSize;
            lz4sd->prefixEnd  += originalSize;
        }
        else
        {
            lz4sd->extDictSize = lz4sd->prefixSize;
            lz4sd->externalDict = (u8*)dest - lz4sd->extDictSize;
            result = LZ4_decompress_generic(source, dest, 0, originalSize,
                                            endOnOutputSize, full, 0,
                                            usingExtDict, (u8*)dest, lz4sd->externalDict, lz4sd->extDictSize);
            if (result <= 0) return result;
            lz4sd->prefixSize = originalSize;
            lz4sd->prefixEnd  = (u8*)dest + originalSize;
        }

        return result;
    }


    /*
     Advanced decoding functions :
     *_usingDict() :
     These decoding functions work the same as "_continue" ones,
     the dictionary must be explicitly provided within parameters
     */

    FORCE_INLINE int LZ4_decompress_usingDict_generic(const char* source, char* dest, int compressedSize, int maxOutputSize, int safe, const char* dictStart, int dictSize)
    {
        if (dictSize==0)
            return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, noDict, (u8*)dest, NULL, 0);
        if (dictStart+dictSize == dest)
        {
            if (dictSize >= (int)(64 KB - 1))
                return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, withPrefix64k, (u8*)dest-64 KB, NULL, 0);
            return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, noDict, (u8*)dest-dictSize, NULL, 0);
        }
        return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, usingExtDict, (u8*)dest, (const u8*)dictStart, dictSize);
    }

    int LZ4_decompress_safe_usingDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize)
    {
        return LZ4_decompress_usingDict_generic(source, dest, compressedSize, maxOutputSize, 1, dictStart, dictSize);
    }

    int LZ4_decompress_fast_usingDict(const char* source, char* dest, int originalSize, const char* dictStart, int dictSize)
    {
        return LZ4_decompress_usingDict_generic(source, dest, 0, originalSize, 0, dictStart, dictSize);
    }

    /* debug function */
    int LZ4_decompress_safe_forceExtDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize)
    {
        return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, endOnInputSize, full, 0, usingExtDict, (u8*)dest, (const u8*)dictStart, dictSize);
    }


    /***************************************************
     *  Obsolete Functions
     ***************************************************/
    /* obsolete compression functions */
    int LZ4_compress_limitedOutput(const char* source, char* dest, int inputSize, int maxOutputSize)
    {
        return LZ4_compress_default(source, dest, inputSize, maxOutputSize);
    }
    int LZ4_compress(const char* source, char* dest, int inputSize)
    {
        return LZ4_compress_default(source, dest, inputSize, LZ4_compressBound(inputSize));
    }
    int LZ4_compress_limitedOutput_withState (void* state, const char* src, char* dst, int srcSize, int dstSize)
    {
        return LZ4_compress_fast_extState(state, src, dst, srcSize, dstSize, 1);
    }
    int LZ4_compress_withState (void* state, const char* src, char* dst, int srcSize)
    {
        return LZ4_compress_fast_extState(state, src, dst, srcSize, LZ4_compressBound(srcSize), 1);
    }
    int LZ4_compress_limitedOutput_continue (LZ4_stream_t* LZ4_stream, const char* src, char* dst, int srcSize, int maxDstSize)
    {
        return LZ4_compress_fast_continue(LZ4_stream, src, dst, srcSize, maxDstSize, 1);
    }
    int LZ4_compress_continue (LZ4_stream_t* LZ4_stream, const char* source, char* dest, int inputSize)
    {
        return LZ4_compress_fast_continue(LZ4_stream, source, dest, inputSize, LZ4_compressBound(inputSize), 1);
    }

    /*
     These function names are deprecated and should no longer be used.
     They are only provided here for compatibility with older user programs.
     - LZ4_uncompress is totally equivalent to LZ4_decompress_fast
     - LZ4_uncompress_unknownOutputSize is totally equivalent to LZ4_decompress_safe
     */
    int LZ4_uncompress (const char* source, char* dest, int outputSize)
    {
        return LZ4_decompress_fast(source, dest, outputSize);
    }
    int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize)
    {
        return LZ4_decompress_safe(source, dest, isize, maxOutputSize);
    }


    /* Obsolete Streaming functions */

    int LZ4_sizeofStreamState()
    {
        return LZ4_STREAMSIZE;
    }

    static void LZ4_init(LZ4_stream_t_internal* lz4ds, u8* base)
    {
        MEM_INIT(lz4ds, 0, LZ4_STREAMSIZE);
        lz4ds->bufferStart = base;
    }

    int LZ4_resetStreamState(void* state, char* inputBuffer)
    {
        if ((((size_t)state) & 3) != 0) return 1;   /* Error : pointer is not aligned on 4-u8s boundary */
        LZ4_init((LZ4_stream_t_internal*)state, (u8*)inputBuffer);
        return 0;
    }

    void* LZ4_create (char* inputBuffer)
    {
        void* lz4ds = ALLOCATOR(8, LZ4_STREAMSIZE_U64);
        LZ4_init ((LZ4_stream_t_internal*)lz4ds, (u8*)inputBuffer);
        return lz4ds;
    }

    char* LZ4_slideInputBuffer (void* LZ4_Data)
    {
        LZ4_stream_t_internal* ctx = (LZ4_stream_t_internal*)LZ4_Data;
        int dictSize = LZ4_saveDict((LZ4_stream_t*)LZ4_Data, (char*)ctx->bufferStart, 64 KB);
        return (char*)(ctx->bufferStart + dictSize);
    }

    /* Obsolete streaming decompression functions */

    int LZ4_decompress_safe_withPrefix64k(const char* source, char* dest, int compressedSize, int maxOutputSize)
    {
        return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, endOnInputSize, full, 0, withPrefix64k, (u8*)dest - 64 KB, NULL, 64 KB);
    }

    int LZ4_decompress_fast_withPrefix64k(const char* source, char* dest, int originalSize)
    {
        return LZ4_decompress_generic(source, dest, 0, originalSize, endOnOutputSize, full, 0, withPrefix64k, (u8*)dest - 64 KB, NULL, 64 KB);
    }

}

#endif   /* LZ4_COMMONDEFS_ONLY */

