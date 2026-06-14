#define _GNU_SOURCE
#include "topology.h"
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <android/log.h>

#define LOG_TAG "CymuxTopology"

void topology_pin_thread(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pid_t tid = gettid();
    if (sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Failed to pin thread to core %d", core_id);
    } else {
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Successfully pinned thread to core %d", core_id);
    }
}
