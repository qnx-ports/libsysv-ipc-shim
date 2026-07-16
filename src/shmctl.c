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
            munmap(header, sizeof(struct shmid_ds));
            return 0;
        }
        case IPC_SET:
        {
            header->shm_perm.uid = buf->shm_perm.uid;
            header->shm_perm.gid = buf->shm_perm.gid;
            header->shm_perm.mode &= (~0b111111111);
            header->shm_perm.mode |= (buf->shm_perm.mode & 0b111111111);
            fchown(shmid, buf->shm_perm.uid, buf->shm_perm.gid);
            break;
        }
        case IPC_RMID:
        {
            // FIXME: This isn't right. This should wait until the last detttach.
            // Again, would be solved by having a resmgr.
            const char *shm_name = header->shm_name;
            close(shmid);
            shm_unlink(shm_name);
            munmap(header, sizeof(struct shmid_ds));
            return 0;
        }
        default:
        break;
    }

    header->shm_lpid = getpid();
    header->shm_ctime = time(NULL);
    munmap(header, sizeof(struct shmid_ds));
    return 0;
}