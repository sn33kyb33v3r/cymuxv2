#ifndef PTY_H
#define PTY_H

#include <sys/types.h>

typedef struct {
    int master_fd;
    int slave_fd;
    pid_t child_pid;
} PtySession;

int pty_session_init(PtySession* session, const char* shell_path);
void pty_session_close(PtySession* session);

#endif
