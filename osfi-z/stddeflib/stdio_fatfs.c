#include "stdio_fatfs.h"

//for FATFS fast seek functionality
static DWORD clmt[SZ_TBL];
typedef struct
{
    uint8_t used;
    FIL file;
    off_t offset;
    DWORD *clmt; //for FATFS fast seek functionality
} FD_descr;

#define descr_count 5
#define STDIO_FATFS_DEBUG 0
static FD_descr descrs[descr_count];

int open(const char *pathname, int flags, ...)
{
    for (uint32_t i = 0; i < descr_count; i++)
    {
	if(!descrs[i].used)
	{
	    descrs[i].used = 1;
	    uint32_t flag = FA_READ;
	    switch(flags)
	    {
	    case O_RDONLY: flag = FA_READ; break;
	    case O_WRONLY: flag = FA_WRITE; break;
	    case O_RDWR: flag = FA_READ | FA_WRITE;
	    }
	    descrs[i].offset = 0;
	    descrs[i].clmt = 0;
	    FRESULT res = f_open(&descrs[i].file, (TCHAR*)pathname, flag);
	    //if(STDIO_FATFS_DEBUG)
		printf("open %s : %d\n", pathname, res);
	    if(res != FR_OK)
	    {
		descrs[i].used = 0;
		return -1;		
	    }
	    return i;
	}
    }
    return -1;
}
ssize_t read(int fd, void *buf, size_t count)
{
    if(fd < 0 || fd >= descr_count || descrs[fd].used == 0)
	return -1;

    UINT br = 0;
    FRESULT res = f_read(&descrs[fd].file, buf, count, &br);
    descrs[fd].offset += (off_t)br;
    if(res != FR_OK)
    {
	printf("r %db f%d:%d\n", br, fd, res);
	return 0;
    }

    if(STDIO_FATFS_DEBUG)
	printf("r %d/%ldb f%d:%d\n", br, count, fd, res);

    return br;
}
void fseek_init(int fd)
{
    if(fd < 0 || fd >= descr_count || descrs[fd].used == 0)
	return;

    if(clmt[0] != 0)
    {
	printf("clmt already used!\n");
	return;
    }
    descrs[fd].clmt = clmt;
    
    descrs[fd].file.cltbl = clmt;
    clmt[0] = SZ_TBL;
    FRESULT res = f_lseek(&descrs[fd].file, CREATE_LINKMAP);     /* Create CLMT */
    if(res == FR_NOT_ENOUGH_CORE)
	printf("clmt not enough\n");

}
off_t lseek(int fd, off_t offset, int whence)
{
    if(fd < 0 || fd >= descr_count || descrs[fd].used == 0)
	return -1;

    if(STDIO_FATFS_DEBUG)
	printf("l f%d s%ld d%ld w%d ", fd,
	       descrs[fd].offset,
	       offset, whence);
    
    switch (whence) {
    case SEEK_CUR:
	offset = offset + descrs[fd].offset;
	break;
    case SEEK_SET:
	
	break;
    case SEEK_END:
	offset = f_size(&descrs[fd].file) + offset;
	break;
    default:
	break;
    }

    descrs[fd].offset = offset;
    FRESULT res = f_lseek(&descrs[fd].file, (UINT)offset);
    if(STDIO_FATFS_DEBUG)
	printf("o%ld:%d\n", offset, res);

    return 0;
}
int close(int fd)
{
    if(fd < 0 || fd >= descr_count || descrs[fd].used == 0)
	return -1;

    //printf("close\n");

    if(descrs[fd].clmt != 0)
    {
	//printf("close clmt\n");
	descrs[fd].clmt[0] = 0;
	clmt[0] = 0;
	descrs[fd].clmt = 0;
    }
    f_close(&descrs[fd].file);
    descrs[fd].used = 0;
    return 0;
}

int fstat(int fd, struct stat *buf)
{
    if(fd < 0 || fd >= descr_count || descrs[fd].used == 0)
	return -1;

    buf->st_size = f_size(&descrs[fd].file);
    return 0;
}


int find_first_set_bit(uint32_t value)
{
    if (value == 0)
        return 32;
    return __builtin_ctz(value);
}

off_t filesize(int fd)
{
    struct stat st;
    fstat(fd, &st);
    return st.st_size;
}
