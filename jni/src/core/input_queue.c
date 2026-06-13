#include "input_queue.h"
#include <android/log.h>

#define LOG_TAG "CymuxInput"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void ring_buffer_init(LockFreeRingBuffer* rb) {
    atomic_init(&rb->head, 0);
    atomic_init(&rb->tail, 0);
}

bool ring_buffer_push(LockFreeRingBuffer* rb, InputEventPacket event) {
    uint32_t current_head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    uint32_t current_tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    uint32_t next_head = (current_head + 1) % RING_BUFFER_SIZE;

    if (next_head == current_tail) {
        return false; 
    }

    rb->buffer[current_head] = event;
    atomic_store_explicit(&rb->head, next_head, memory_order_release);
    return true;
}

bool ring_buffer_pop(LockFreeRingBuffer* rb, InputEventPacket* event) {
    uint32_t current_tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    uint32_t current_head = atomic_load_explicit(&rb->head, memory_order_acquire);

    if (current_tail == current_head) {
        return false; 
    }

    *event = rb->buffer[current_tail];
    uint32_t next_tail = (current_tail + 1) % RING_BUFFER_SIZE;
    atomic_store_explicit(&rb->tail, next_tail, memory_order_release);
    return true;
}

void process_native_input_queue(AInputQueue* queue, LockFreeRingBuffer* rb) {
    AInputEvent* event = NULL;
    while (AInputQueue_getEvent(queue, &event) >= 0) {
        if (AInputQueue_preDispatchEvent(queue, event) != 0) {
            continue;
        }

        int32_t type = AInputEvent_getType(event);
        if (type == AINPUT_EVENT_TYPE_KEY) {
            InputEventPacket packet;
            packet.key_code = AKeyEvent_getKeyCode(event);
            packet.action = AKeyEvent_getAction(event);
            packet.meta_state = AKeyEvent_getMetaState(event);

            if (!ring_buffer_push(rb, packet)) {
                __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "Input ring buffer overflowed");
            }
        }

        AInputQueue_finishEvent(queue, event, 1);
    }
}
