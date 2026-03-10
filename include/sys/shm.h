/*	$NetBSD: shm.h,v 1.15 1994/06/29 06:45:17 cgd Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 1994 Adam Glass
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Adam Glass.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * As defined+described in "X/Open System Interfaces and Headers"
 *                         Issue 4, p. XXX
 */

#ifndef _SYS_SHM_H_
#define _SYS_SHM_H_

#include <time.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/mman.h>

#ifdef _QNX_SYSV_SHM_INTERNAL
#define SHM_HEADER(shm) ((struct shmid_ds*)((char*)shm - sizeof(struct shmid_ds)))
#define SHM_FULL_SIZE(header) (header->shm_segsz + sizeof(struct shmid_ds))
#define SHM_BODY(shm) ((void*)((char*)shm + sizeof(struct shmid_ds)))
#endif

#define MAX_QNX_SYSV_SHM_NAME "/sysv-shm-4294967295"
#define MAX_QNX_SYSV_SHM_ANON_NAME "/sysv-shm-anon"
#ifndef __QNX__
#define __PAGESIZE 4096
#define SHM_ANON   ((char*)-1)
#endif

#define SHM_RDONLY 010000     /* non-op on QNX */ 
#define SHM_RND    020000     /* Round attach address to SHMLBA */
#define SHMLBA     __PAGESIZE /* Segment low boundary address multiple */
#define SHM_FAILED ((void*)(intptr_t)-1)

typedef unsigned int shmatt_t;

struct shmid_ds
{
    struct ipc_perm shm_perm; /* operation permission structure */
    size_t shm_segsz;         /* size of segment in bytes */
    pid_t shm_lpid;           /* process ID of last shared memory op */
    pid_t shm_cpid;           /* process ID of creator */
    shmatt_t shm_nattch;      /* number of current attaches */
    time_t shm_atime;         /* time of last shmat() */
    time_t shm_dtime;         /* time of last shmdt() */
    time_t shm_ctime;         /* time of last change by shmctl() */
    const char shm_name[sizeof(MAX_QNX_SYSV_SHM_NAME)];
};

__BEGIN_DECLS
void* shmat(int shmid, const void* shmaddr, int shmflg);
int shmget(key_t key, size_t size, int shmflg);
int shmctl(int shmid, int cmd, struct shmid_ds* buf);
int shmdt(const void* addr);
__END_DECLS

#endif /* !_SYS_SHM_H_ */
