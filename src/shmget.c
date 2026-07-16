/*
 * Copyright (c) 2026, BlackBerry Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _QNX_SYSV_SHM_INTERNAL
#include <errno.h>
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
        int save_errno = errno;
        close(fd);
        if (key != IPC_PRIVATE)
        {
            shm_unlink(name);
        }
        errno = save_errno;
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
        int save_errno = errno;
        close(fd);
        if (key != IPC_PRIVATE)
        {
            shm_unlink(name);
        }
        errno = save_errno;
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

    munmap(header, full_size);

    return fd;
}
