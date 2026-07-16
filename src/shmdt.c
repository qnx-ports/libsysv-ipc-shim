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
