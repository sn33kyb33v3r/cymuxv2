#define _GNU_SOURCE
#include <unistd.h>
#include <sys/select.h>
#include <stdint.h>
#include "cymux_engine.h"

void extern parse_ansi_stream(CymuxRuntime *runtime, const uint8_t *buf, size_t len);

void *io_thread_loop(void *arg) {
    CymuxRuntime *runtime = (CymuxRuntime *)arg;
    uint8_t read_buf[4096];
    fd_set read_fds;

    while (runtime->shell_pid > 0) {
        FD_ZERO(&read_fds);
        FD_SET(runtime->pty_master, &read_fds);

        struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
        int ret = select(runtime->pty_master + 1, &read_fds, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(runtime->pty_master, &read_fds)) {
            ssize_t n = read(runtime->pty_master, read_buf, sizeof(read_buf));
            if (n > 0) {
                pthread_mutex_lock(&runtime->matrix_mutex);
                parse_ansi_stream(runtime, read_buf, n);
                pthread_mutex_unlock(&runtime->matrix_mutex);
            } else if (n < 0) {
                break;
            }
        }
    }
    return NULL;
}
