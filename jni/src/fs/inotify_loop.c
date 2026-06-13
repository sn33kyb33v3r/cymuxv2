#include "../core/cymux_engine.h"
#include <android/log.h>
#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define LOG_TAG "CymuxInotify"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void* inotify_event_loop(void* arg) {
    CymuxRuntime* runtime = (CymuxRuntime*)arg;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(4, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        return NULL;
    }

    int wd = inotify_add_watch(fd, "/sdcard", IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM);
    if (wd < 0) {
        close(fd);
        return NULL;
    }

    uint8_t buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    
    while (runtime->running) {
        ssize_t len = read(fd, buffer, sizeof(buffer));
        if (len < 0 && errno == EAGAIN) {
            usleep(50000);
            continue;
        }
        
        if (len > 0) {
            size_t i = 0;
            while (i < (size_t)len) {
                struct inotify_event* event = (struct inotify_event*)&buffer[i];
                if (event->len > 0) {
                    atomic_fetch_add_explicit(&runtime->matrix.revision, 1, memory_order_release);
                }
                i += sizeof(struct inotify_event) + event->len;
            }
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return NULL;
}
