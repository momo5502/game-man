#include "flutter_window.h"
#include <type_traits>

flutter_window::flutter_window(JNIEnv* env, jobject surface)
{
    this->window_ = ANativeWindow_fromSurface(env, surface);
}

flutter_window::~flutter_window()
{
    if (this->window_)
    {
        ANativeWindow_release(this->window_);
        this->window_ = nullptr;
    }
}

flutter_window::flutter_window(flutter_window&& obj) noexcept
{
    this->operator=(std::move(obj));
}

flutter_window::flutter_window(const flutter_window& obj)
{
    this->operator=(obj);
}

flutter_window& flutter_window::operator=(flutter_window&& obj) noexcept
{
    if (this != &obj)
    {
        this->~flutter_window();
        this->window_ = obj.window_;
        obj.window_ = nullptr;
    }

    return *this;
}

flutter_window& flutter_window::operator=(const flutter_window& obj)
{
    if (this != &obj)
    {
        this->~flutter_window();

        this->window_ = obj.window_;
        if (this->window_)
        {
            ANativeWindow_acquire(this->window_);
        }
    }

    return *this;
}

flutter_window::operator EGLNativeWindowType() const
{
    return this->window_;
}

flutter_window::operator bool() const
{
    return this->window_ != nullptr;
}
