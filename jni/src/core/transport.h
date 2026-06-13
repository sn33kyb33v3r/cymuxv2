#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SHARED_POOL_SIZE (1024 * 1024) 

typedef struct {
    uint8_t* pool_buffer;
    size_t capacity;
    uint32_t head;
    uint32_t tail;
} SharedMemoryPool;

void transport_pool_init(SharedMemoryPool* pool);
void transport_bridge_splice(int pty_master_fd, SharedMemoryPool* pool);
void transport_pool_cleanup(SharedMemoryPool* pool);

#endif
