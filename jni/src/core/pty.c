#define _GNU_SOURCE
#include "pty.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <android/log.h>

#define LOG_TAG "CymuxPty"

int pty_session_init(PtySession* session, const char* shell_path) {
    session->master_fd = posix_openpt(O_RDWR | O_CLOEXEC);
    if (session->master_fd < 0) return -1;

    if (grantpt(session->master_fd) < 0 || unlockpt(session->master_fd) < 0) {
        close(session->master_fd);
        return -2;
    }

    char slave_name[64];
    if (ptsname_r(session->master_fd, slave_name, sizeof(slave_name)) != 0) {
        close(session->master_fd);
        return -3;
    }

    session->slave_fd = open(slave_name, O_RDWR | O_CLOEXEC);
    if (session->slave_fd < 0) {
        close(session->master_fd);
        return -4;
    }

    struct termios tios;
    if (tcgetattr(session->slave_fd, &tios) == 0) {
        cfmakeraw(&tios);
        tcsetattr(session->slave_fd, TCSANOW, &tios);
    }

    struct winsize ws = { .ws_row = 24, .ws_col = 80, .ws_xpixel = 0, .ws_ypixel = 0 };
    ioctl(session->slave_fd, TIOCSWINSZ, &ws);

    session->child_pid = fork();
    if (session->child_pid < 0) {
        close(session->slave_fd);
        close(session->master_fd);
        return -5;
    }

    if (session->child_pid == 0) {
        close(session->master_fd);

        setsid();
        if (ioctl(session->slave_fd, TIOCSCTTY, NULL) < 0) {
            _exit(1);
        }

        dup2(session->slave_fd, STDIN_FILENO);
        dup2(session->slave_fd, STDOUT_FILENO);
        dup2(session->slave_fd, STDERR_FILENO);

        if (session->slave_fd > STDERR_FILENO) {
            close(session->slave_fd);
        }

        char* env[] = { "TERM=xterm-256color", "PATH=/system/bin:/system/xbin", NULL };
        execle(shell_path, shell_path, NULL, env);
        _exit(1);
    }

    close(session->slave_fd);
    return 0;
}

void pty_session_close(PtySession* session) {
    if (session->master_fd >= 0) {
        close(session->master_fd);
        session->master_fd = -1;
    }
}
