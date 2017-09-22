#ifndef WARBLE_H
#define WARBLE_H
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "core_alloc.h"
#include "codecs.h"
#include "metadata.h"
#include "platform.h"
#include "load_code.h"
#include "i2s.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "dsp_core.h"


int dmain();
#endif
