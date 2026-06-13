#ifndef CYMUX_ENGINE_H
#define CYMUX_ENGINE_H

#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <android/native_activity.h>
#include <android/input.h>

#define SCREEN_ROWS 24
#define SCREEN_COLS 80
#define INPUT_BUFFER_SIZE 1024

typedef struct {
    uint8_t character;
    uint8_t fg_color;
    uint8_t bg_color;
    uint8_t mode;
} GridCell;

typedef struct {
    GridCell grid[SCREEN_ROWS * SCREEN_COLS];
} ScreenMatrix;

typedef struct {
    uint8_t data[INPUT_BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
} LockFreeInputBuffer;

typedef struct {
    int pty_master;
    pid_t shell_pid;
    union {
        uint8_t screen_matrix[SCREEN_ROWS * SCREEN_COLS];
        ScreenMatrix matrix_layout;
    };
    uint32_t cursor_row;
    uint32_t cursor_col;
    LockFreeInputBuffer input_buffer;
    LockFreeInputBuffer input_queue;
    struct {
        volatile uint32_t revision;
    } matrix;
    pthread_mutex_t matrix_mutex;
    pthread_mutex_t queue_mutex;
    AInputQueue* queue;
    ANativeActivity* activity;
    volatile int running;
} CymuxRuntime;

#endif
