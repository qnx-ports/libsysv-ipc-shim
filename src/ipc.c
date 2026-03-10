#include "../include/sys/ipc.h"
#include <stdio.h>

key_t //
ftok(const char* path, int proj_id)
{
    fprintf(stderr, "ftok is not available on QNX");
    return IPC_PRIVATE;
}