#define _GNU_SOURCE
#include "cymux_engine.h"
#include <android/log.h>
#include <android/looper.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_TAG "CymuxMainActivity"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern void* pty_reader_thread(void* arg);
extern void* input_poll_thread(void* arg);

static void bind_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    LOGI("Input Queue Attaching");
    CymuxRuntime* runtime = (CymuxRuntime*)activity->instance;
    pthread_mutex_lock(&runtime->queue_mutex);
    runtime->queue = queue;
    pthread_mutex_unlock(&runtime->queue_mutex);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    LOGI("Input Queue Detaching");
    CymuxRuntime* runtime = (CymuxRuntime*)activity->instance;
    pthread_mutex_lock(&runtime->queue_mutex);
    runtime->queue = NULL;
    pthread_mutex_unlock(&runtime->queue_mutex);
}

static void onStart(ANativeActivity* activity) {
    LOGI("Cymux Runtime Primary Control Plane Starting");
}

static void onResume(ANativeActivity* activity) {
    LOGI("Cymux Runtime Focused");
}

static void* init_runtime(void* arg) {
    CymuxRuntime* runtime = (CymuxRuntime*)arg;
    bind_core(7);

    atomic_init(&runtime->input_queue.head, 0);
    atomic_init(&runtime->input_queue.tail, 0);
    atomic_init(&runtime->matrix.revision, 0);
    pthread_mutex_init(&runtime->queue_mutex, NULL);
    runtime->queue = NULL;
    runtime->running = 1;

    pthread_t pty_thread, input_thread;
    pthread_create(&pty_thread, NULL, pty_reader_thread, runtime);
    pthread_create(&input_thread, NULL, input_poll_thread, runtime);

    pthread_detach(pty_thread);
    pthread_detach(input_thread);
    return NULL;
}

static void onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    CymuxRuntime* runtime = malloc(sizeof(CymuxRuntime));
    runtime->activity = activity;
    activity->instance = runtime;

    pthread_t init_thread;
    pthread_create(&init_thread, NULL, init_runtime, runtime);
    pthread_detach(init_thread);
}

static void onDestroy(ANativeActivity* activity) {
    CymuxRuntime* runtime = (CymuxRuntime*)activity->instance;
    runtime->running = 0;
    close(runtime->pty_master);
    pthread_mutex_destroy(&runtime->queue_mutex);
    free(runtime);
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    onCreate(activity, savedState, savedStateSize);
}
