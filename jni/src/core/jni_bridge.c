#include <jni.h>
#include <android/log.h>
#include "topology.h"
#include "input_queue.h"
#include "transport.h"
#include "parser.h"
#include "renderer.h"
#include "pty.h"

#define LOG_TAG "CymuxJniBridge"

static PtySession g_session;
static GlesRenderer g_renderer;
static SharedMemoryPool g_pool;

JNIEXPORT jint JNICALL
Java_com_cymux_TerminalCore_nativeInit(JNIEnv* env, jobject thiz, jstring shell_path) {
    const char* path = (*env)->GetStringUTFChars(env, shell_path, NULL);
    if (!path) return -1;

    topology_pin_thread(7);

    transport_pool_init(&g_pool);
    renderer_init(&g_renderer);

    int res = pty_session_init(&g_session, path);
    (*env)->ReleaseStringUTFChars(env, shell_path, path);

    return res;
}

JNIEXPORT void JNICALL
Java_com_cymux_TerminalCore_nativeTriggerRenderLoop(JNIEnv* env, jobject thiz, jbyteArray buffer) {
    jbyte* bytes = (*env)->GetByteArrayElements(env, buffer, NULL);
    jsize len = (*env)->GetArrayLength(env, buffer);

    if (bytes) {
        transport_bridge_splice(g_session.master_fd, &g_pool);
        parser_neon_scan((const uint8_t*)bytes, (size_t)len);
        renderer_update_grid(&g_renderer, (const uint8_t*)g_pool.pool_buffer);
        renderer_render_frame(&g_renderer);

        (*env)->ReleaseByteArrayElements(env, buffer, bytes, JNI_ABORT);
    }
}

JNIEXPORT void JNICALL
Java_com_cymux_TerminalCore_nativeShutdown(JNIEnv* env, jobject thiz) {
    pty_session_close(&g_session);
    transport_pool_cleanup(&g_pool);
}
