#include "cymux_engine.h"
#include <android/log.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#define LOG_TAG "CymuxPtyHandler"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern void parse_ansi_stream(const uint8_t* buffer, size_t len, ScreenMatrix* matrix);

void* pty_reader_thread(void* arg) {
    CymuxRuntime* runtime = (CymuxRuntime*)arg;
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(7, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);

    int master_fd;
    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);

    if (pid < 0) {
        LOGE("Forkpty execution failed");
        return NULL;
    }

    if (pid == 0) {
        setenv("TERM", "xterm-256color", 1);
        execl("/system/bin/sh", "-", NULL);
        exit(1);
    }

    runtime->pty_master = master_fd;
    runtime->child_pid = pid;

    long flags = fcntl(master_fd, F_GETFL, 0);
    fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);

    uint8_t read_buffer[2048];

    while (runtime->running) {
        ssize_t bytes_read = read(master_fd, read_buffer, sizeof(read_buffer));
        if (bytes_read > 0) {
            parse_ansi_stream(read_buffer, bytes_read, &runtime->matrix);
        } else if (bytes_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            usleep(1000);
        } else {
            break;
        }
    }
    return NULL;
}
