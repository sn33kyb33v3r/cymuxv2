#define _GNU_SOURCE
#include "cymux_engine.h"
#include <string.h>

void init_cymux_runtime_state(CymuxRuntime* runtime) {
    if (!runtime) return;
    memset(&runtime->matrix.grid, 0, sizeof(runtime->matrix.grid));
    atomic_store_explicit(&runtime->matrix.revision, 0, memory_order_release);
}
