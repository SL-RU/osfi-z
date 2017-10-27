#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

#include "rbcodecconfig.h"
#include "rbcodecplatform.h"
#include <unistd.h>


#ifndef ARRAYLEN
# define ARRAYLEN(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef MIN
# define MIN(x, y) ((x)<(y) ? (x) : (y))
#endif

#ifndef MAX
# define MAX(x, y) ((x)>(y) ? (x) : (y))
#endif

#ifndef BIT_N
# define BIT_N(n) (1U << (n))
#endif

#ifndef MASK_N
/* Make a mask of n contiguous bits, shifted left by 'shift' */
# define MASK_N(type, n, shift) \
     ((type)((((type)1 << (n)) - (type)1) << (shift)))
#endif


#define ICONST_ATTR
#define CACHEALIGN_SIZE 1
#define ICODE_ATTR
#define MEM_ALIGN_ATTR

#ifdef HAVE_PITCHCONTROL
/* precision of the pitch and speed variables */
/* One zero per decimal (100 means two decimal places */
#define PITCH_SPEED_PRECISION 100L
#define PITCH_SPEED_100 (100L * PITCH_SPEED_PRECISION)  /* 100% speed */
#endif /* HAVE_PITCHCONTROL */

int find_first_set_bit(uint32_t value);
#endif /* PLATFORM_H_INCLUDED */
