#define _QNX_SYSV_SHM_INTERNAL
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "../include/sys/shm.h"

int //
shmctl(int shmid, int cmd, struct shmid_ds* buf)
{
    int prot = PROT_READ;
    if (cmd != IPC_STAT)
    {
        prot |= PROT_WRITE;
    }

    struct shmid_ds* header = mmap(NULL,                    //
                                   sizeof(struct shmid_ds), //
                                   prot,                    //
                                   MAP_SHARED,              //
                                   shmid,                   //
                                   0);
    if (header == MAP_FAILED)
    {
        return -1;
    }

    switch (cmd)
    {
        case IPC_STAT:
        {
            memcpy(buf, header, sizeof(struct shmid_ds));
            return 0;
        }
        case IPC_SET:
        {
            header->shm_perm.uid = buf->shm_perm.uid;
            header->shm_perm.gid = buf->shm_perm.gid;
            header->shm_perm.mode &= (~0b111111111);
            header->shm_perm.mode |= (buf->shm_perm.mode & 0b111111111);
            fchown(shmid, buf->shm_perm.uid, buf->shm_perm.gid);
            return 0;
        }
        case IPC_RMID:
        {
            close(shmid);
            shm_unlink(header->shm_name);
            return 0;
        }
    }

    munmap(header, sizeof(struct shmid_ds));
    header->shm_lpid = getpid();
    header->shm_ctime = time(NULL);
    return 0;
}