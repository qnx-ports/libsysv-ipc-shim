#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <functional>

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#define SHM_MODE 0666

class ShmTest : public ::testing::Test {
public:
    /* These are for the temporary file we generate.  */
    char *name;
    int shmid;

    ~ShmTest() override = default;

    void SetUp() override {
        name = NULL;
        shmid = -1;
    }

    void TearDown() override {
        if (shmid != -1) {
            /* Enforce message queue removal in case of early test failure.
             Ignore error since the shm may already have being removed.  */
            shmctl(shmid, IPC_RMID, NULL);
            shmid = -1;
        }
    }
};

TEST_F(ShmTest, read_write) {
    key_t key = getpid();

    shmid = shmget(key, 4096, IPC_CREAT | IPC_EXCL | SHM_MODE);
    ASSERT_NE(shmid, -1);

    struct shmid_ds shminfo;
    ASSERT_NE(shmctl (shmid, IPC_STAT, &shminfo), -1);

    int *shmem = (int *)shmat (shmid, NULL, 0);
    ASSERT_NE(shmem, (void*) -1);

    shmem[0]   = 0x55555555;
    shmem[32]  = 0x44444444;
    shmem[64]  = 0x33333333;
    shmem[128] = 0x22222222;

    ASSERT_NE(shmdt (shmem), -1);

    shmem = (int *)shmat (shmid, NULL, SHM_RDONLY);
    ASSERT_NE(shmem, (void*) -1);

    EXPECT_EQ (shmem[0],   0x55555555);
    EXPECT_EQ (shmem[32],  0x44444444);
    EXPECT_EQ (shmem[64],  0x33333333);
    EXPECT_EQ (shmem[128], 0x22222222);

    ASSERT_NE(shmdt (shmem), -1);

    EXPECT_NE(shmctl (shmid, IPC_RMID, 0), -1);
    shmid = -1;
}

TEST_F(ShmTest, rm_on_last_dettach) {
    key_t key = getpid();

    shmid = shmget(key, 4096, IPC_CREAT | IPC_EXCL | SHM_MODE);
    ASSERT_NE(shmid, -1);

    int *shmem = (int *)shmat (shmid, NULL, 0);
    ASSERT_NE(shmem, (void*) -1);

    EXPECT_NE(shmctl (shmid, IPC_RMID, 0), -1);

    struct shmid_ds shminfo;
    EXPECT_NONFATAL_FAILURE([&]() {
            EXPECT_NE(shmctl (shmid, IPC_STAT, &shminfo), -1) << "Test backing-file deletion after all instances are dettached.";

            // TODO: Unfortunately we'll start getting garbage in memory if this
            // continues...

            // shmem[0]   = 0x55555555;
            // shmem[32]  = 0x44444444;
            // shmem[64]  = 0x33333333;
            // shmem[128] = 0x22222222;

            // EXPECT_EQ (shminfo.shm_nattch, 1);

            // int *shmem2 = (int *)shmat (shmid, NULL, SHM_RDONLY);
            // EXPECT_NE(shmem2, (void*) -1);

            // EXPECT_EQ (shmem2[0],   0x55555555);
            // EXPECT_EQ (shmem2[32],  0x44444444);
            // EXPECT_EQ (shmem2[64],  0x33333333);
            // EXPECT_EQ (shmem2[128], 0x22222222);

            // ASSERT_NE(shmctl (shmid, IPC_STAT, &shminfo), -1);

            // EXPECT_EQ (shminfo.shm_nattch, 2);

            // // Dettach all.
            // ASSERT_NE(shmdt (shmem), -1);
            // ASSERT_NE(shmdt (shmem2), -1);
            // // Ensure we can't still stat shmid as normal.
            // EXPECT_EQ(shmctl (shmid, IPC_STAT, &shminfo), -1);

            // // Ensure the data is zeroed.
            // shmid = shmget(key, 4096, IPC_CREAT | SHM_MODE);
            // ASSERT_NE(shmid, -1);
            // shmem = (int *)shmat (shmid, NULL, 0);
            // ASSERT_NE(shmem, (void*) -1);

            // EXPECT_EQ (shmem[0],   0x0);
            // EXPECT_EQ (shmem[32],  0x0);
            // EXPECT_EQ (shmem[64],  0x0);
            // EXPECT_EQ (shmem[128], 0x0);

            // EXPECT_NE(shmctl (shmid, IPC_RMID, 0), -1);
            // shmid = -1;
        }(),
        "Test backing-file deletion after all instances are dettached."
    );
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}