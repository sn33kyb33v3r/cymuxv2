#define _GNU_SOURCE
#include "cymux_engine.h"
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <sched.h>
#include <unistd.h>

#define LOG_TAG "CymuxInputBridge"

void* input_poll_thread(void* arg) {
    CymuxRuntime* runtime = (CymuxRuntime*)arg;
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(7, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);

    while (runtime->running) {
        AInputQueue* queue = NULL;
        
        pthread_mutex_lock(&runtime->queue_mutex);
        queue = runtime->queue;
        pthread_mutex_unlock(&runtime->queue_mutex);

        if (!queue) {
            usleep(10000);
            continue;
        }

        AInputEvent* event = NULL;
        while (AInputQueue_getEvent(queue, &event) >= 0) {
            if (AInputQueue_preDispatchEvent(queue, event) == 0) {
                int32_t type = AInputEvent_getType(event);
                if (type == AINPUT_EVENT_TYPE_KEY) {
                    int32_t action = AKeyEvent_getAction(event);
                    int32_t code = AKeyEvent_getKeyCode(event);
                    
                    if (action == AKEY_EVENT_ACTION_DOWN) {
                        uint32_t head = atomic_load_explicit(&runtime->input_queue.head, memory_order_relaxed);
                        uint32_t next_head = (head + 1) % RING_BUFFER_SIZE;
                        uint32_t tail = atomic_load_explicit(&runtime->input_queue.tail, memory_order_acquire);
                        
                        if (next_head != tail) {
                            runtime->input_queue.buffer[head] = (uint8_t)code;
                            atomic_store_explicit(&runtime->input_queue.head, next_head, memory_order_release);
                            
                            write(runtime->pty_master, &runtime->input_queue.buffer[head], 1);
                        }
                    }
                }
                AInputQueue_finishEvent(queue, event, 1);
            }
        }
    }
    return NULL;
}
