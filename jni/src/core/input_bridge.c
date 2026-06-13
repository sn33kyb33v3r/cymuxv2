#define _GNU_SOURCE
#include <android/input.h>
#include <android/keycodes.h>
#include <unistd.h>
#include <pthread.h>
#include "cymux_engine.h"

void write_input_to_pty(CymuxRuntime *runtime, int keycode) {
    char c = 0;
    if (keycode >= AKEYCODE_A && keycode <= AKEYCODE_Z) {
        c = 'a' + (keycode - AKEYCODE_A);
    } else if (keycode >= AKEYCODE_0 && keycode <= AKEYCODE_9) {
        c = '0' + (keycode - AKEYCODE_0);
    } else if (keycode == AKEYCODE_ENTER) {
        c = '\r';
    } else if (keycode == AKEYCODE_SPACE) {
        c = ' ';
    }
    if (c != 0 && runtime->pty_master > 0) {
        write(runtime->pty_master, &c, 1);
    }
}

void* input_poll_loop(void* arg) {
    CymuxRuntime* runtime = (CymuxRuntime*)arg;
    while (runtime->running) {
        pthread_mutex_lock(&runtime->queue_mutex);
        if (runtime->queue != NULL) {
            AInputEvent* event = NULL;
            while (AInputQueue_getEvent(runtime->queue, &event) >= 0) {
                if (AInputQueue_preDispatchEvent(runtime->queue, event) == 0) {
                    int32_t handled = 0;
                    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
                        int32_t action = AKeyEvent_getAction(event);
                        if (action == AKEY_EVENT_ACTION_DOWN) {
                            int32_t keycode = AKeyEvent_getKeyCode(event);
                            write_input_to_pty(runtime, keycode);
                            handled = 1;
                        }
                    }
                    AInputQueue_finishEvent(runtime->queue, event, handled);
                }
            }
        }
        pthread_mutex_unlock(&runtime->queue_mutex);
        usleep(10000);
    }
    return NULL;
}
