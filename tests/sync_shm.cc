#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#define SHM_MODE 0666

class SyncShmTest : public ::testing::Test {
public:
    /* These are for the temporary file we generate.  */
    char *name;
    int shmid;
    sem_t *startup_sem;

    ~SyncShmTest() override = default;

    void SetUp() override {
        name = NULL;
        shmid = -1;
        startup_sem = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
        if (startup_sem != MAP_FAILED) {
            sem_init(startup_sem, 1, 0);
        }
    }

    void TearDown() override {
        if (shmid != -1) {
            /* Enforce message queue removal in case of early test failure.
             Ignore error since the shm may already have being removed.  */
            shmctl(shmid, IPC_RMID, NULL);
            shmid = -1;
        }
        sem_destroy(startup_sem);
        munmap(startup_sem, sizeof(sem_t));
    }
};

TEST_F(SyncShmTest, dettach_on_normal_exit) {
    // Create shm in a child process, which then terminates normally. Check if
    // the shm was dettached.
    key_t key = getpid();
    int pid = fork();
    int *shmem;
    if (pid == 0) {
        // CHILD PROCESS
        shmid = shmget(key, 4096, IPC_CREAT | IPC_EXCL | SHM_MODE);
        ASSERT_NE(shmid, -1);

        shmem = (int *)shmat (shmid, NULL, 0);
        ASSERT_NE(shmem, (void*) -1);

        exit(0);
    }
    // PARENT PROCESS
    waitpid(pid, NULL, 0);

    shmid = shmget(key, 4096, SHM_MODE);
    ASSERT_NE(shmid, -1);

    shmem = (int *)shmat (shmid, NULL, 0);
    ASSERT_NE(shmem, (void*) -1);

    struct shmid_ds shminfo;
    ASSERT_NE(shmctl (shmid, IPC_STAT, &shminfo), -1);

    EXPECT_NONFATAL_FAILURE([&]() {
            EXPECT_EQ(shminfo.shm_nattch, 1) << "Test cleaning up attachments on normal terminations.";
        }(),
        "Test cleaning up attachments on normal terminations."
    );
}

TEST_F(SyncShmTest, dettach_on_abnormal_exit) {
    // Create shm in a child process, which then terminates abnormally. Check if
    // the shm was dettached.
    key_t key = getpid();
    int pid = fork();
    int *shmem;
    if (pid == 0) {
        // CHILD PROCESS
        shmid = shmget(key, 4096, IPC_CREAT | IPC_EXCL | SHM_MODE);
        ASSERT_NE(shmid, -1);

        shmem = (int *)shmat (shmid, NULL, 0);
        ASSERT_NE(shmem, (void*) -1);

        sem_post(startup_sem);
        sleep(5);
        // We should not get here...
        exit(-1);
    }
    // PARENT PROCESS
    while ((sem_wait(startup_sem) == -1) && (errno == EINTR)) ;
    ASSERT_NE(kill(pid, SIGTERM), -1);
    waitpid(pid, NULL, 0);

    shmid = shmget(key, 4096, SHM_MODE);
    ASSERT_NE(shmid, -1);

    shmem = (int *)shmat (shmid, NULL, 0);
    ASSERT_NE(shmem, (void*) -1);

    struct shmid_ds shminfo;
    ASSERT_NE(shmctl (shmid, IPC_STAT, &shminfo), -1);

    EXPECT_NONFATAL_FAILURE([&]() {
            EXPECT_EQ(shminfo.shm_nattch, 1) << "Test cleaning up attachments on abnormal terminations.";
        }(),
        "Test cleaning up attachments on abnormal terminations."
    );
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
