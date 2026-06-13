#ifndef CYMUX_ENGINE_H
#define CYMUX_ENGINE_H

#include <android/native_activity.h>
#include <android/input.h>
#include <pthread.h>
#include <stdatomic.h>

#define SCREEN_COLS 160
#define SCREEN_ROWS 45
#define RING_BUFFER_SIZE 4096

typedef struct {
    uint8_t character;
    uint8_t fg_color;
    uint8_t bg_color;
    uint8_t attrib;
} __attribute__((packed)) GridCell;

typedef struct {
    GridCell grid[SCREEN_ROWS * SCREEN_COLS];
    _Atomic uint32_t revision;
} ScreenMatrix;

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    _Atomic uint32_t head;
    _Atomic uint32_t tail;
} InputRingBuffer;

typedef struct {
    ANativeActivity* activity;
    AInputQueue* queue;
    int pty_master;
    pid_t child_pid;
    InputRingBuffer input_queue;
    ScreenMatrix matrix;
    volatile int running;
    pthread_mutex_t queue_mutex;
} CymuxRuntime;

#endif
