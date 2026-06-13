#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

int execute_atomic_move(const char* src_absolute_path, const char* dest_absolute_path) {
    if (!src_absolute_path || !dest_absolute_path) {
        return -1;
    }
    return rename(src_absolute_path, dest_absolute_path);
}
