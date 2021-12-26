#pragma once

#include <android/native_window_jni.h>
#include <EGL/egl.h>

class flutter_window
{
public:

    flutter_window() = default;
    flutter_window(JNIEnv* env, jobject surface);
    ~flutter_window();

    flutter_window(flutter_window&& obj) noexcept;
    flutter_window(const flutter_window& obj);

    flutter_window& operator=(flutter_window&& obj) noexcept;
    flutter_window& operator=(const flutter_window& obj);

    operator EGLNativeWindowType() const;
    operator bool() const;

private:
    ANativeWindow* window_{nullptr};
};
