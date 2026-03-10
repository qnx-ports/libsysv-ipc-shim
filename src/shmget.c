#define _QNX_SYSV_SHM_INTERNAL
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include "../include/sys/shm.h"

// QNX only

int //
shmget(key_t key, size_t size, int shmflg)
{
    char name[sizeof(MAX_QNX_SYSV_SHM_NAME)] = {};
    int fd = 0;
    mode_t mode = 0;
    size_t full_size = size + sizeof(struct shmid_ds);

    int oflag = O_RDWR;
    if (shmflg & IPC_CREAT)
    {
        oflag |= O_CREAT;
        mode = shmflg & 0b111111111;
    }

    if (shmflg & IPC_EXCL)
    {
        oflag |= O_EXCL;
    }

    if (key == IPC_PRIVATE)
    {
        fd = shm_open(SHM_ANON, oflag | O_CREAT, shmflg & 0b111111111);
        sprintf(name, MAX_QNX_SYSV_SHM_ANON_NAME);
    }
    else
    {
        char shm_path[sizeof("/dev/shmem/") + sizeof(name)] = {};
        sprintf(name, "/sysv-shm-%u", key);
        sprintf(shm_path, "/dev/shmem%s", name);
        if (access(shm_path, F_OK) == 0)
        {
            return shm_open(name, oflag, 0);
        }
        fd = shm_open(name, oflag, mode);
    }

    if (fd < 0)
    {
        return fd;
    }

    // keeping on creating the new shared memory
    if (ftruncate(fd, full_size) < 0)
    {
        return -1;
    }

    struct shmid_ds* header = mmap(NULL,                   //
                                   full_size,              //
                                   PROT_READ | PROT_WRITE, //
                                   MAP_SHARED,             //
                                   fd,                     //
                                   0);
    if (header == MAP_FAILED)
    {
        close(fd);
        if (key != IPC_PRIVATE)
        {
            shm_unlink(name);
        }
        return -1;
    }

    memset(header, 0x0, full_size);

    header->shm_perm.cuid = getuid();
    header->shm_perm.cgid = getgid();
    header->shm_perm.uid = getuid();
    header->shm_perm.gid = getuid();
    header->shm_perm.mode = mode;
    strcpy((char*)header->shm_name, name);

    header->shm_segsz = size;
    header->shm_lpid = 0;
    header->shm_cpid = getpid();
    header->shm_nattch = 0;
    header->shm_atime = 0;
    header->shm_dtime = 0;
    header->shm_ctime = time(NULL);

    return fd;
}
