/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 by Daniel Stenberg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__


/* symbolic names for multiple choice configurations: */

/* CONFIG_STORAGE (note these are combineable bit-flags) */

/* platforms
 * bit fields to allow PLATFORM_HOSTED to be OR'ed e.g. with a
 * possible future PLATFORM_ANDROID (some OSes might need totally different
 * handling to run on them than a stand-alone application) */
#define PLATFORM_NATIVE  (1<<0)
#define PLATFORM_HOSTED  (1<<1)

#define ROCKBOX_LITTLE_ENDIAN 1

/* Define the GCC version used for the build */
#define GCCNUM 701

/* Define this if you build rockbox to support the logf logging and display */
#undef ROCKBOX_HAS_LOGF

/* Define this if you want logf to output to the serial port */
#undef LOGF_SERIAL

/* Define this to record a chart with timings for the stages of boot */
#undef DO_BOOTCHART

#define HAVE_ASSERT_H


#include "sdlapp.h"

//#define CONFIG_PLATFORM PLATFORM_NATIVE

/* setup basic macros from capability masks */
#include "config_caps.h"

#if !defined(__ASSEMBLER__)
#include "system-arm.h"
#endif

#define CPU_ARM

#define ROCKBOX_STRICT_ALIGN 1

#if defined(CPU_ARM) && defined(__ASSEMBLER__)
/* ARMv4T doesn't switch the T bit when popping pc directly, we must use BX */
.macro ldmpc cond="", order="ia", regs
#if ARM_ARCH == 4 && defined(USE_THUMB)
    ldm\cond\order sp!, { \regs, lr }
    bx\cond lr
#else
    ldm\cond\order sp!, { \regs, pc }
#endif
.endm
.macro ldrpc cond=""
#if ARM_ARCH == 4 && defined(USE_THUMB)
    ldr\cond lr, [sp], #4
    bx\cond  lr
#else
    ldr\cond pc, [sp], #4
#endif
.endm
#endif

#define ICODE_ATTR
#define ICONST_ATTR
#define IDATA_ATTR
#define IBSS_ATTR
#define INIT_ATTR
#define INITDATA_ATTR

#define IF_COP(...)
#define IF_COP_VOID(...)    void
#define IF_COP_CORE(core)   CURRENT_CORE

#endif /* __CONFIG_H__ */