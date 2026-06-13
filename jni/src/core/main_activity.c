#include <android/native_activity.h>
#include <android/log.h>
#include <pthread.h>
#include <stdlib.h>
#include "cymux_engine.h"

#define LOG_TAG "CymuxMainActivity"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void* io_thread_loop(void *arg);
void* input_poll_loop(void* arg);
int init_pty_subsystem(CymuxRuntime *runtime, int rows, int cols);

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    CymuxRuntime* runtime = (CymuxRuntime*)activity->instance;
    pthread_mutex_lock(&runtime->queue_mutex);
    runtime->queue = queue;
    pthread_mutex_unlock(&runtime->queue_mutex);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    CymuxRuntime* runtime = (CymuxRuntime*)activity->instance;
    pthread_mutex_lock(&runtime->queue_mutex);
    runtime->queue = NULL;
    pthread_mutex_unlock(&runtime->queue_mutex);
}

static void onDestroy(ANativeActivity* activity) {
    CymuxRuntime* runtime = (CymuxRuntime*)activity->instance;
    runtime->running = 0;
    free(runtime);
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    CymuxRuntime* runtime = calloc(1, sizeof(CymuxRuntime));
    activity->instance = runtime;

    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    activity->callbacks->onDestroy = onDestroy;

    runtime->activity = activity;
    runtime->running = 1;

    pthread_mutex_init(&runtime->matrix_mutex, NULL);
    pthread_mutex_init(&runtime->queue_mutex, NULL);

    init_pty_subsystem(runtime, SCREEN_ROWS, SCREEN_COLS);

    pthread_t io_thread;
    pthread_create(&io_thread, NULL, io_thread_loop, runtime);
    pthread_detach(io_thread);

    pthread_t input_thread;
    pthread_create(&input_thread, NULL, input_poll_loop, runtime);
    pthread_detach(input_thread);
}
