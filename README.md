# libsysv-ipc

Wrapper on top of `shm_*` to provide old SysV style ipc

## Supports
- `shm*`: shared memory

## Guide
If you are using clang, put headers into `/usr/include/shim`

Build with `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`