cmake_minimum_required(VERSION 3.5)

project(libsysv-ipc CXX C)

set(CMAKE_CXX_STANDARD 17)

option(LIBSYSV_IPC_ENABLE_TESTING "Enable testing of the libsysv-ipc library." ON)
option(LIBSYSV_IPC_ENABLE_INSTALL "Enable install of the libsysv-ipc library." ON)

set(LIBSYSV_IPC_SOURCES src/ipc.c
    src/shmat.c
    src/shmctl.c
    src/shmdt.c
    src/shmget.c)

add_library(sysv-ipc ${LIBSYSV_IPC_SOURCES})
target_include_directories(sysv-ipc PUBLIC include)

if(LIBSYSV_IPC_ENABLE_INSTALL)
    install(TARGETS sysv-ipc)
endif()

if(LIBSYSV_IPC_ENABLE_TESTING)
    find_package(GTest REQUIRED)

    enable_testing()

    add_executable(shm_test tests/shm.cc)
    target_link_libraries(shm_test sysv-ipc GTest::gtest)
    add_test(NAME shm_test COMMAND shm_test)

    add_executable(sync_shm_test tests/sync_shm.cc)
    target_link_libraries(sync_shm_test sysv-ipc GTest::gtest)
    add_test(NAME sync_shm_test COMMAND sync_shm_test)
endif()
