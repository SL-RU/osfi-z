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

#include "stm32f4xx_hal.h"

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
#define ROCKBOX_HAS_LOGF

/* Define this if you want logf to output to the serial port */
#undef LOGF_SERIAL

/* Define this to record a chart with timings for the stages of boot */
#undef DO_BOOTCHART

#define HAVE_ASSERT_H




//#define CONFIG_PLATFORM PLATFORM_NATIVE

/* setup basic macros from capability masks */
#include "config_caps.h"

#if !defined(__ASSEMBLER__)
#include "system-arm.h"
#endif

#define ROCKBOX_STRICT_ALIGN 1


#endif /* __CONFIG_H__ */
