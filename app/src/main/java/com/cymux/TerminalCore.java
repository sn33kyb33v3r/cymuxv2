package com.cymux;

public class TerminalCore {
    static {
        System.loadLibrary("cymux");
    }

    public native int nativeInit(String shellPath);
    public native void nativeTriggerRenderLoop(byte[] buffer);
    public native void nativeShutdown();
}
