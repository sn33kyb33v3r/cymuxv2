#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>

int affinity_bind_prime(void) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(7, &cpuset);
    return sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

int affinity_bind_middle(void) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(4, &cpuset);
    CPU_SET(5, &cpuset);
    CPU_SET(6, &cpuset);
    return sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}
