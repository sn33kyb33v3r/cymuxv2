#define _GNU_SOURCE
#include <pty.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "cymux_engine.h"

int init_pty_subsystem(CymuxRuntime *runtime, int rows, int cols) {
    struct winsize ws = {
        .ws_row = rows,
        .ws_col = cols,
        .ws_xpixel = 0,
        .ws_ypixel = 0
    };

    pid_t pid = forkpty(&runtime->pty_master, NULL, NULL, &ws);
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        setenv("TERM", "xterm-256color", 1);
        char *shell = getenv("SHELL");
        if (!shell) shell = "/system/bin/sh";
        execl(shell, shell, "-", NULL);
        _exit(127);
    }

    int flags = fcntl(runtime->pty_master, F_GETFL, 0);
    fcntl(runtime->pty_master, F_SETFL, flags | O_NONBLOCK);
    runtime->shell_pid = pid;
    return 0;
}
