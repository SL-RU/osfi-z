/*
  Copyright (c) 2005, The Musepack Development Team
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided
  with the distribution.

  * Neither the name of the The Musepack Development Team nor the
  names of its contributors may be used to endorse or promote
  products derived from this software without specific prior
  written permission.

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
*/

/// \file math.h
/// Libmpcdec internal math routines.  

#ifndef _mpcdec_math_h_
#define _mpcdec_math_h_

#include "mpc_types.h"

#define MPC_FIXED_POINT_SHIFT 16

#ifdef MPC_FIXED_POINT

   #ifdef _WIN32_WCE
      #include <cmnintrin.h>
      #define MPC_HAVE_MULHIGH
   #endif

   typedef mpc_int64_t MPC_SAMPLE_FORMAT_MULTIPLY;

   #define MAKE_MPC_SAMPLE(X)      (MPC_SAMPLE_FORMAT)((double)(X) * (double)(((mpc_int64_t)1)<<MPC_FIXED_POINT_FRACTPART))
   #define MAKE_MPC_SAMPLE_EX(X,Y) (MPC_SAMPLE_FORMAT)((double)(X) * (double)(((mpc_int64_t)1)<<(Y)))
   
   #define MPC_SHR_RND(X, Y)       ((X+(1<<(Y-1)))>>Y)

#if defined(CPU_COLDFIRE)

      #define MPC_MULTIPLY(X,Y)      mpc_multiply((X), (Y))
      #define MPC_MULTIPLY_EX(X,Y,Z) mpc_multiply_ex((X), (Y), (Z))
      
      static inline MPC_SAMPLE_FORMAT mpc_multiply(MPC_SAMPLE_FORMAT x,
                                                   MPC_SAMPLE_FORMAT y)
      {
          MPC_SAMPLE_FORMAT t1, t2;
          asm volatile (
              "mac.l   %[x],%[y],%%acc0\n" /* multiply */
              "mulu.l  %[y],%[x]   \n"     /* get lower half, avoid emac stall */
              "movclr.l %%acc0,%[t1]   \n" /* get higher half */
              "moveq.l #17,%[t2]   \n"
              "asl.l   %[t2],%[t1] \n"     /* hi <<= 17, plus one free */
              "moveq.l #14,%[t2]   \n"
              "lsr.l   %[t2],%[x]  \n"     /* (unsigned)lo >>= 14 */
              "or.l    %[x],%[t1]  \n"     /* combine result */
              : /* outputs */
              [t1]"=&d"(t1),
              [t2]"=&d"(t2),
              [x] "+d" (x)
              : /* inputs */
              [y] "d"  (y)
          );
          return t1;
      }

      static inline MPC_SAMPLE_FORMAT mpc_multiply_ex(MPC_SAMPLE_FORMAT x,
                                                      MPC_SAMPLE_FORMAT y,
                                                      unsigned shift)
      {
          MPC_SAMPLE_FORMAT t1, t2;
          asm volatile (
              "mac.l   %[x],%[y],%%acc0\n" /* multiply */
              "mulu.l  %[y],%[x]   \n"     /* get lower half, avoid emac stall */
              "movclr.l %%acc0,%[t1]   \n" /* get higher half */
              "moveq.l #31,%[t2]   \n"
              "sub.l   %[sh],%[t2] \n"     /* t2 = 31 - shift */
              "ble.s   1f          \n"
              "asl.l   %[t2],%[t1] \n"     /* hi <<= 31 - shift */
              "lsr.l   %[sh],%[x]  \n"     /* (unsigned)lo >>= shift */
              "or.l    %[x],%[t1]  \n"     /* combine result */
              "bra.s   2f          \n"
          "1:                      \n"
              "neg.l   %[t2]       \n"     /* t2 = shift - 31 */
              "asr.l   %[t2],%[t1] \n"     /* hi >>= t2 */
          "2:                      \n"
              : /* outputs */
              [t1]"=&d"(t1),
              [t2]"=&d"(t2),
              [x] "+d" (x)
              : /* inputs */
              [y] "d"  (y),
              [sh]"d"  (shift)
          );
          return t1;
      }
   #elif defined(CPU_ARM)
      /* Calculate: result = (X*Y)>>14 */
      #define MPC_MULTIPLY(X,Y) \
         ({ \
            MPC_SAMPLE_FORMAT lo; \
            MPC_SAMPLE_FORMAT hi; \
            asm volatile ( \
               "smull %[lo], %[hi], %[x], %[y] \n\t" /* multiply */ \
               "mov   %[lo], %[lo], lsr #14    \n\t" /* lo >>= 14 */ \
               "orr   %[lo], %[lo], %[hi], lsl #18"  /* lo |= (hi << 18) */ \
               : [lo]"=&r"(lo), [hi]"=&r"(hi) \
               : [x]"r"(X), [y]"r"(Y)); \
            lo; \
         })
      
      /* Calculate: result = (X*Y)>>Z */
      #define MPC_MULTIPLY_EX(X,Y,Z) \
         ({ \
            MPC_SAMPLE_FORMAT lo; \
            MPC_SAMPLE_FORMAT hi; \
            asm volatile ( \
               "smull %[lo], %[hi], %[x], %[y] \n\t"   /* multiply */ \
               "mov   %[lo], %[lo], lsr %[shr] \n\t"   /* lo >>= Z */ \
               "orr   %[lo], %[lo], %[hi], lsl %[shl]" /* lo |= (hi << (32-Z)) */ \
               : [lo]"=&r"(lo), [hi]"=&r"(hi) \
               : [x]"r"(X), [y]"r"(Y), [shr]"r"(Z), [shl]"r"(32-Z)); \
            lo; \
         })
   #else /* libmusepack standard */

      #define MPC_MULTIPLY_NOTRUNCATE(X,Y) \
         (((MPC_SAMPLE_FORMAT_MULTIPLY)(X) * (MPC_SAMPLE_FORMAT_MULTIPLY)(Y)) >> MPC_FIXED_POINT_FRACTPART)

      #define MPC_MULTIPLY_EX_NOTRUNCATE(X,Y,Z) \
         (((MPC_SAMPLE_FORMAT_MULTIPLY)(X) * (MPC_SAMPLE_FORMAT_MULTIPLY)(Y)) >> (Z))

      #ifdef _DEBUG
         static inline MPC_SAMPLE_FORMAT MPC_MULTIPLY(MPC_SAMPLE_FORMAT item1,MPC_SAMPLE_FORMAT item2)
         {
            MPC_SAMPLE_FORMAT_MULTIPLY temp = MPC_MULTIPLY_NOTRUNCATE(item1,item2);
            assert(temp == (MPC_SAMPLE_FORMAT_MULTIPLY)(MPC_SAMPLE_FORMAT)temp);
            return (MPC_SAMPLE_FORMAT)temp;
         }
         
         static inline MPC_SAMPLE_FORMAT MPC_MULTIPLY_EX(MPC_SAMPLE_FORMAT item1,MPC_SAMPLE_FORMAT item2,unsigned shift)
         {
            MPC_SAMPLE_FORMAT_MULTIPLY temp = MPC_MULTIPLY_EX_NOTRUNCATE(item1,item2,shift);
            assert(temp == (MPC_SAMPLE_FORMAT_MULTIPLY)(MPC_SAMPLE_FORMAT)temp);
            return (MPC_SAMPLE_FORMAT)temp;
         }
      #else
         #define MPC_MULTIPLY(X,Y) ((MPC_SAMPLE_FORMAT)MPC_MULTIPLY_NOTRUNCATE(X,Y))
         #define MPC_MULTIPLY_EX(X,Y,Z) ((MPC_SAMPLE_FORMAT)MPC_MULTIPLY_EX_NOTRUNCATE(X,Y,Z))
      #endif

   #endif

   #ifdef MPC_HAVE_MULHIGH
      #define MPC_MULTIPLY_FRACT(X,Y) _MulHigh(X,Y)
   #else
      #if defined(CPU_COLDFIRE)
         /* loses one bit of accuracy. The rest of the macros won't be as easy as this... */
         #define MPC_MULTIPLY_FRACT(X,Y) \
            ({ \
               MPC_SAMPLE_FORMAT t; \
               asm volatile ( \
                  "mac.l %[A], %[B], %%acc0\n\t" \
                  "movclr.l %%acc0, %[t]   \n\t" \
                  "asr.l #1, %[t]          \n\t" \
                  : [t] "=d" (t) \
                  : [A] "r" ((X)), [B] "r" ((Y))); \
               t; \
            })
      #elif defined(CPU_ARM)
         /* Calculate: result = (X*Y)>>32, without need for >>32 */
         #define MPC_MULTIPLY_FRACT(X,Y) \
            ({ \
               MPC_SAMPLE_FORMAT lo; \
               MPC_SAMPLE_FORMAT hi; \
               asm volatile ( \
                  "smull %[lo], %[hi], %[x], %[y]" /* hi = result */ \
                  : [lo]"=&r"(lo), [hi]"=&r"(hi) \
                  : [x]"r"(X), [y]"r"(Y)); \
               hi; \
            })
      #else
         #define MPC_MULTIPLY_FRACT(X,Y) MPC_MULTIPLY_EX(X,Y,32)
      #endif
   #endif

   #define MPC_MAKE_FRACT_CONST(X) (MPC_SAMPLE_FORMAT)((X) * (double)(((mpc_int64_t)1)<<32) )
   
   #define MPC_MULTIPLY_FLOAT_INT(X,Y) ((X)*(Y))

#else
   //in floating-point mode, decoded samples are in -1...1 range

   typedef float MPC_SAMPLE_FORMAT;
   
   #define MAKE_MPC_SAMPLE(X)      ((MPC_SAMPLE_FORMAT)(X))
   #define MAKE_MPC_SAMPLE_EX(X,Y) ((MPC_SAMPLE_FORMAT)(X))
   
   #define MPC_MULTIPLY_FRACT(X,Y) ((X)*(Y))
   #define MPC_MAKE_FRACT_CONST(X) (X)
   
   #define MPC_MULTIPLY_FLOAT_INT(X,Y) ((X)*(Y))
   #define MPC_MULTIPLY(X,Y)           ((X)*(Y))
   #define MPC_MULTIPLY_EX(X,Y,Z)      ((X)*(Y))
   
   #define MPC_SHR_RND(X, Y) (X)

#endif

#endif // _mpcdec_math_h_

