#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

int 
main(int argc, char** argv)
{
        int id = shmget(atoi(argv[1]), 0, 0600);
        struct shmid_ds details = {};
        shmctl(id, IPC_STAT, &details);
        printf("shm_segsz: %zu\n", details.shm_segsz);
        printf("shm_lpid: %u\n", details.shm_lpid);
        printf("shm_cpid: %u\n", details.shm_cpid);
        printf("shm_nattch: %u\n", details.shm_nattch);
        printf("shm_atime: %ld\n", details.shm_atime);
        printf("shm_dtime: %ld\n", details.shm_dtime);
        printf("shm_ctime: %ld\n", details.shm_ctime);
        return 0;
}