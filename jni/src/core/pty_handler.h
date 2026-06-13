#ifndef PTY_HANDLER_H
#define PTY_HANDLER_H

#include "cymux_engine.h"

int init_pty_subsystem(CymuxRuntime *runtime, int rows, int cols);

#endif
