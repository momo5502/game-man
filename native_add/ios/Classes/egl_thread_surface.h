#pragma once

#include <EGL/egl.h>
#include "flutter_window.h"

class egl_thread_surface
{
public:
    egl_thread_surface(flutter_window window);
    ~egl_thread_surface();

    egl_thread_surface(const egl_thread_surface&) = delete;
    egl_thread_surface(egl_thread_surface&&) noexcept = delete;

    egl_thread_surface& operator=(const egl_thread_surface&) = delete;
    egl_thread_surface& operator=(egl_thread_surface&&) noexcept = delete;

    void swap_buffers() const;

private:
    flutter_window window_{};
    EGLDisplay display_{EGL_NO_DISPLAY};
    EGLSurface surface_{EGL_NO_SURFACE};
    EGLContext context_{EGL_NO_CONTEXT};

    void release();
};