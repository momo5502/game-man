#include "egl_thread_surface.h"
#include "gb/utils/finally.hpp"
#include <stdexcept>

egl_thread_surface::egl_thread_surface(flutter_window window)
    : window_(std::move(window))
{
    auto cleanup = utils::finally([this]()
    {
        this->release();
    });

    this->display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (this->display_ == EGL_NO_DISPLAY)
    {
        throw std::runtime_error("eglGetDisplay failed");
    }

    EGLint version[2];
    if (!eglInitialize(this->display_, &version[0], &version[1]))
    {
        throw std::runtime_error("eglInitialize failed");
    }

    EGLConfig config{};
    EGLint num_configs{};
    EGLint config_attributes[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    //Request opengl ES2.0
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    eglChooseConfig(this->display_, config_attributes, &config, 1, &num_configs);
    if (!num_configs)
    {
        throw std::runtime_error("eglChooseConfig failed");
    }

    EGLint context_attributes[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
    };

    this->context_ = eglCreateContext(this->display_, config, EGL_NO_CONTEXT, context_attributes);
    if (this->context_ == EGL_NO_CONTEXT)
    {
        throw std::runtime_error("eglCreateContext failed");
    }

    this->surface_ = eglCreateWindowSurface(this->display_, config, this->window_, nullptr);
    if (!this->surface_ || this->surface_ == EGL_NO_SURFACE)
    {
        throw std::runtime_error("eglCreateWindowSurface failed");
    }

    if (!eglMakeCurrent(this->display_, this->surface_, this->surface_, this->context_))
    {
        throw std::runtime_error("eglMakeCurrent failed");
    }

    cleanup.cancel();
}

egl_thread_surface::~egl_thread_surface()
{
    this->release();
}

void egl_thread_surface::release()
{
    auto display = this->display_;
    auto surface = this->surface_;
    auto context = this->context_;

    this->display_ = EGL_NO_DISPLAY;
    this->surface_ = EGL_NO_SURFACE;
    this->context_ = EGL_NO_CONTEXT;

    if (display == EGL_NO_DISPLAY)
    {
        return;
    }

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (surface != EGL_NO_SURFACE)
    {
        eglDestroySurface(display, surface);
    }

    if (context != EGL_NO_CONTEXT)
    {
        eglDestroyContext(display, context);
    }

    eglTerminate(display);
}

void egl_thread_surface::swap_buffers() const
{
    eglSwapBuffers(this->display_, this->surface_);
}
