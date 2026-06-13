#ifndef INPUT_QUEUE_H
#define INPUT_QUEUE_H

#include <android/input.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

#define RING_BUFFER_SIZE 1024

typedef struct {
    uint32_t key_code;
    uint32_t action;
    uint32_t meta_state;
} InputEventPacket;

typedef struct {
    InputEventPacket buffer[RING_BUFFER_SIZE];
    _Atomic uint32_t head;
    _Atomic uint32_t tail;
} LockFreeRingBuffer;

void ring_buffer_init(LockFreeRingBuffer* rb);
bool ring_buffer_push(LockFreeRingBuffer* rb, InputEventPacket event);
bool ring_buffer_pop(LockFreeRingBuffer* rb, InputEventPacket* event);

#endif
