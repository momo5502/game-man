#pragma once

#include <EGL/egl.h>

class egl_thread_surface
{
public:
    egl_thread_surface(EGLNativeWindowType window);
    ~egl_thread_surface();

    void swap_buffers() const;

private:
    EGLDisplay display_{EGL_NO_DISPLAY};
    EGLSurface surface_{EGL_NO_SURFACE};
    EGLContext context_{EGL_NO_CONTEXT};

    void release();
};