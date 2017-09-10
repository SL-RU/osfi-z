#ifndef ROCKBOX_VOICE_ENCODER
#include "codeclib.h" 
#include "autoconf.h"
#else
#define ICODE_ATTR
#define IDATA_ATTR
#define IBSS_ATTR
#define ICONST_ATTR
#endif
/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

#ifndef ROCKBOX_VOICE_ENCODER

#define DISABLE_FLOAT_API
#define DISABLE_VBR

/* Make use of ARM4E assembly optimizations */
#if defined(CPU_ARM)
#define ARM4_ASM
#endif

/* Make use of Coldfire assembly optimizations */
#if defined(CPU_COLDFIRE)
#define COLDFIRE_ASM
#endif

/* Make use of Blackfin assembly optimizations */
/* #undef BFIN_ASM */
#endif /* ROCKBOX_VOICE_ENCODER */

/* Disable wideband codec */
/* #undef DISABLE_WIDEBAND */

/* Enable valgrind extra checks */
/* #undef ENABLE_VALGRIND */

/* Debug fixed-point implementation */
/* #undef FIXED_DEBUG */

#ifndef ROCKBOX_VOICE_ENCODER
/* Compile target codec as fixed point */
#define FIXED_POINT 
#else
/* Compile voice clip encoder as floating point */
#define FLOATING_POINT 
#endif

#ifndef ROCKBOX_VOICE_CODEC
#define EXC_ICONST_ATTR ICONST_ATTR
#define GAIN_ICONST_ATTR ICONST_ATTR
#define HEXC_ICONST_ATTR ICONST_ATTR
#define LSP_ICONST_ATTR ICONST_ATTR
#else
#define EXC_ICONST_ATTR
#define GAIN_ICONST_ATTR
#define HEXC_ICONST_ATTR
#define LSP_ICONST_ATTR
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the `getopt_long' function. */
#define HAVE_GETOPT_LONG 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `m' library (-lm). */
/* #undef HAVE_LIBM */

/* Define to 1 if you have the `winmm' library (-lwinmm). */
/* #undef HAVE_LIBWINMM */

/* Define to 1 if you have the <memory.h> header file. */
/* #define HAVE_MEMORY_H 1 */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/audioio.h> header file. */
/* #undef HAVE_SYS_AUDIOIO_H */

/* Define to 1 if you have the <sys/soundcard.h> header file. */
#define HAVE_SYS_SOUNDCARD_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Reduce precision to 16 bits (EXPERIMENTAL) */
/* #undef PRECISION16 */

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* Version extra */
#define SPEEX_EXTRA_VERSION "-git"

/* Version major */
#define SPEEX_MAJOR_VERSION 1

/* Version micro */
#define SPEEX_MICRO_VERSION 15

/* Version minor */
#define SPEEX_MINOR_VERSION 1

/* Complete version string */
#define SPEEX_VERSION "1.2beta3"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable support for TI C55X DSP */
/* #undef TI_C55X */

/* Make use of alloca */
/* #undef USE_ALLOCA */

/* Use C99 variable-size arrays */
#define VAR_ARRAYS 

/* Enable Vorbis-style psychoacoustics (EXPERIMENTAL) */
/* #undef VORBIS_PSYCHO */

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#ifdef ROCKBOX_BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif

/* Enable SSE support */
/* #undef _USE_SSE */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to equivalent of C99 restrict keyword, or to nothing if this is not
   supported. Do not define if restrict is supported directly. */
#define restrict __restrict

#define RELEASE 1

