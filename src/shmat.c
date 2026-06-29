#define _QNX_SYSV_SHM_INTERNAL
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/sys/shm.h"

// FIXME: No way to automatically decrement shm_nattch on abnormal program exit.
// This should be easy to achieve using a resmgr.
void* //
shmat(int shmid, const void* shmaddr, int shmflg)
{
    int flags = MAP_SHARED;

    const void* addr = shmaddr;
    if (addr != NULL && //
        shmflg & SHM_RND)
    {
        addr = ((char*)shmaddr - ((uintptr_t)shmaddr % SHMLBA));
    }

    // Attach the header before the attach-point of the address if possible.
    //
    // FIXME: I think this will cause some unexpected outcomes around
    // deliberately choosing a certain address to attach at. Not sure what the
    // right fix is...
    if ((size_t) addr >= sizeof(struct shmid_ds)) // Implying addr != NULL
    {
        addr -= sizeof(struct shmid_ds);
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
                     PROT_READ | PROT_WRITE,//
                     flags,                 //
                     shmid,                 //
                     0);
    if (shm == MAP_FAILED)
    {
        munmap(header, sizeof(struct shmid_ds));
        return SHM_FAILED;
    }

    // FIXME: These must be two separate mapped addresses to have different
    // protections.
#if 0
    // The header must be read/write otherwise we can't touch shm_* attribs.
    if (shmflg == SHM_RDONLY) {
        if (mprotect(SHM_BODY(shm), shm->shm_segsz, PROT_READ) == -1) {
            munmap(shm, SHM_FULL_SIZE(header));
            munmap(header, sizeof(struct shmid_ds));
            return SHM_FAILED;
        }
    }
#endif

    header->shm_atime = time(NULL);
    header->shm_lpid = getpid();
    header->shm_nattch++;
    munmap(header, sizeof(struct shmid_ds));
    return SHM_BODY(shm);
}