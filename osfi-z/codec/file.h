#undef MAX_PATH
#define MAX_PATH 260
#include <unistd.h>
#include <fcntl.h>
//#include "filesystem-native.h"

off_t filesize(int fd);
