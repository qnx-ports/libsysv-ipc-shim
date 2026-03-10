#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* These are for the temporary file we generate.  */
static char *name;
static int shmid;

#define SHM_MODE 0666
#define CHECK_EQ(v, k) \
  if ((v) != (k)) { printf("%d != %d", v, k); }

static void //
remove_shm ()
{
  /* Enforce message queue removal in case of early test failure.
     Ignore error since the shm may already have being removed.  */
  shmctl (shmid, IPC_RMID, NULL);
}


void //
main(int argc, char** argv)
{
    atexit(remove_shm);
    key_t key = getpid();

    shmid = shmget(key, 4096, IPC_CREAT | IPC_EXCL | SHM_MODE);
    if (shmid == -1)
    {
        printf("shmget fail %d", errno);
    }

    struct shmid_ds shminfo;
    if(shmctl (shmid, IPC_STAT, &shminfo) == -1)
    {
        printf("IPC_STAT fail %d", errno);
    }

    int *shmem = shmat (shmid, NULL, 0);
    if (shmem == (void*) -1)
    {
        printf("shmat fail %d", errno);
    }

    shmem[0]   = 0x55555555;
    shmem[32]  = 0x44444444;
    shmem[64]  = 0x33333333;
    shmem[128] = 0x22222222;

    if(shmdt (shmem) == -1)
    {
        printf("shmdt fail %d", errno);
    }

    shmem = shmat (shmid, NULL, SHM_RDONLY);
    if (shmem == (void*) -1)
    {
        printf("shmat readonly fail %d", errno);
    }

    CHECK_EQ (shmem[0],   0x55555555);
    CHECK_EQ (shmem[32],  0x44444444);
    CHECK_EQ (shmem[64],  0x33333333);
    CHECK_EQ (shmem[128], 0x22222222);

    if(shmdt (shmem) == -1)
    {
        printf("shmdt2 fail %d", errno);
    }

    if (shmctl (shmid, IPC_RMID, 0) == -1)
    {
        printf("IPC_RMID fail %d", errno);
    }
}