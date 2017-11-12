#ifndef STDIO_FATFS_H
#define STDIO_FATFS_H
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ff.h"
#include "rbcodecconfig.h"
#include <stdio.h>

#define SZ_TBL 1024

/**
 * Set buffer for cluster link map table. It is required for FATFS fast seek functionality
 *
 * @param fd file
 * @return 
 */
void fseek_init(int fd);

#endif
