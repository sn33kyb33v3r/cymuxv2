#define _GNU_SOURCE
#include "transport.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "CymuxTransport"

void transport_pool_init(SharedMemoryPool* pool) {
    pool->capacity = SHARED_POOL_SIZE;
    pool->head = 0;
    pool->tail = 0;
    pool->pool_buffer = mmap(NULL, pool->capacity, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (pool->pool_buffer == MAP_FAILED) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Failed to map shared memory pool via mmap");
        pool->pool_buffer = NULL;
    }
}

void transport_bridge_splice(int pty_master_fd, SharedMemoryPool* pool) {
    if (!pool->pool_buffer || pty_master_fd < 0) return;

    int pipe_fds[2];
    if (pipe2(pipe_fds, O_NONBLOCK) < 0) {
        return;
    }

    struct iovec iov;
    iov.iov_base = pool->pool_buffer + pool->head;
    iov.iov_len = pool->capacity - pool->head;

    ssize_t spliced = vmsplice(pipe_fds[1], &iov, 1, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
    if (spliced > 0) {
        ssize_t bytes_read = read(pty_master_fd, pool->pool_buffer + pool->head, spliced);
        if (bytes_read > 0) {
            pool->head = (pool->head + bytes_read) % pool->capacity;
        }
    }

    close(pipe_fds[0]);
    close(pipe_fds[1]);
}

void transport_pool_cleanup(SharedMemoryPool* pool) {
    if (pool->pool_buffer) {
        munmap(pool->pool_buffer, pool->capacity);
        pool->pool_buffer = NULL;
    }
}
