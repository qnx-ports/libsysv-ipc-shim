#define _QNX_SYSV_SHM_INTERNAL
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/sys/shm.h"

void* //
shmat(int shmid, const void* shmaddr, int shmflg)
{
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_SHARED;

    const void* addr = shmaddr;
    if (addr != NULL && //
        shmflg & SHM_RND)
    {
        addr = ((char*)shmaddr - ((uintptr_t)shmaddr % SHMLBA) - sizeof(struct shmid_ds));
    }

    struct shmid_ds* header = mmap(NULL,                    //
                                   sizeof(struct shmid_ds), //
                                   PROT_READ | PROT_WRITE,  //
                                   MAP_SHARED,              //
                                   shmid,                   //
                                   0);
    if (header == MAP_FAILED)
    {
        return SHM_FAILED;
    }

    char* shm = mmap((void*)addr,           //
                     SHM_FULL_SIZE(header), //
                     prot,                  //
                     flags,                 //
                     shmid,                 //
                     0);
    if (shm == MAP_FAILED)
    {
        return SHM_FAILED;
    }

    header->shm_atime = time(NULL);
    header->shm_lpid = getpid();
    header->shm_nattch++;
    munmap(header, sizeof(struct shmid_ds));
    return SHM_BODY(shm);
}