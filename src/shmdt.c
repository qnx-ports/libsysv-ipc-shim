#define _QNX_SYSV_SHM_INTERNAL
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include "../include/sys/shm.h"

int //
shmdt(const void* shmaddr)
{
    if (shmaddr == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    struct shmid_ds* header = SHM_HEADER(shmaddr);
    size_t full_size = SHM_FULL_SIZE(header);
    header->shm_nattch--;
    header->shm_dtime = time(NULL);
    header->shm_lpid = getpid();
    return munmap(header, full_size);
}
