#include <stdint.h>
#include <thread>
#include <string>
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include "gb/game_boy.hpp"

#include <android/log.h>


#include "gb/utils/utils.hpp"
#include "gb/utils/finally.hpp"
#include "gb/utils/binary_buffer.hpp"
#include "egl_thread_surface.h"

#include <dlfcn.h>
#include "flutter_window.h"
#include "texture_2d.h"

std::mutex window_mutex;
flutter_window window{};

class joypad_mobile : public joypad {
public:
    virtual bool is_up_pressed() override {
        return states[0];
    }

    virtual bool is_down_pressed() override {
        return states[1];
    }

    virtual bool is_left_pressed() override
    {
        return states[2];
    }

    virtual bool is_right_pressed() override
    {
        return states[3];
    }

    virtual bool is_a_pressed()override
    {
        return states[4];
    }

    virtual bool is_b_pressed()override
    {
        return states[5];
    }

    virtual bool is_start_pressed()override
    {
        return states[6];
    }

    virtual bool is_select_pressed()override
    {
        return states[7];
    }

    void set_state(int id, bool state) {
        states[id] = state;
    }

private:
    std::atomic<bool> states[8]{};
};

class game_boy_mobile_egl : public display
{
public:
    std::unique_ptr<egl_thread_surface> thread_surface_{};
    std::unique_ptr<texture_2d> texture_;
    
    joypad_mobile joypad{};
    game_boy gb;
    std::thread runner;

    std::atomic<bool> request_dump{false};
    std::string dumped_state;

    game_boy_mobile_egl()
        : gb(&joypad, this)
    {

    }

    ~game_boy_mobile_egl() {
        gb.turn_off();
        if (runner.joinable()) {
            runner.join();
        }
    }

    void draw_frame(const color* buffer/* GB_WIDTH * GB_HEIGHT */) override {
        if (!this->thread_surface_ || !this->texture_) {
            if(request_dump) request_dump = false;
            return;
        }

        glViewport(0, 0, GB_WIDTH * 2, GB_HEIGHT * 2);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->texture_->update(buffer);
        this->texture_->draw();

        this->thread_surface_->swap_buffers();

        dump_state();
    }

    void dump_state()
    {
        if(request_dump)
        {
            utils::binary_buffer data{};
            gb.serialize(data);
            dumped_state = data.get_buffer();

            request_dump = false;
        }
    }

    void set_title(std::string title) override {
        __android_log_write(ANDROID_LOG_INFO, "gbegl", title.data());
    }

    void run(void* data, uint64_t size, bool is_save)
    {
        uint8_t* start = reinterpret_cast<uint8_t*>(data);

        std::vector<uint8_t> buffer{};
        buffer.assign(start, start + size);

        this->runner = std::thread([this, is_save, b = std::move(buffer)]()
        {
            while (true)
            {
                {
                    std::lock_guard<std::mutex> _(window_mutex);
                    if(window)
                    {
                        break;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            this->work(std::move(b), is_save);
        });
    }

private:
    void work(std::vector<uint8_t> buffer, bool is_save)
    {
        const auto _ = utils::finally([this]()
        {
            this->texture_ = {};
            this->thread_surface_ = {};
        });

        this->thread_surface_ = std::make_unique<egl_thread_surface>(window);
        this->texture_ = std::make_unique<texture_2d>(GB_WIDTH, GB_HEIGHT);

        if(is_save) {
            utils::binary_buffer data{buffer};
            gb.serialize(data);
        }
        else{
            gb.load_rom(std::move(buffer));
            gb.skip_bios();
        }

        gb.run();

        dump_state();
    }
};

std::mutex m;
std::unique_ptr<game_boy_mobile_egl> mobile_gb_egl{};

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void
press_button(int id, bool value)
{
    std::lock_guard<std::mutex> _(m);
    if (mobile_gb_egl) {
        mobile_gb_egl->joypad.set_state(id, value);
    }
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void
load_rom(void* data, uint64_t size, bool is_save)
{
    std::lock_guard<std::mutex> _(m);
    mobile_gb_egl = std::make_unique<game_boy_mobile_egl>();
    mobile_gb_egl->run(data, size, is_save);
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void
get_state(void(*callback)(const void* data, uint64_t size))
{
    std::unique_lock<std::mutex> lock(m);
    if(!mobile_gb_egl) return;

    bool was_paused = mobile_gb_egl->gb.is_paused();
    mobile_gb_egl->gb.resume();

    mobile_gb_egl->request_dump = true;
    while(mobile_gb_egl && mobile_gb_egl->request_dump) {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        lock.lock();
    }

    if(!mobile_gb_egl) return;

    auto data = mobile_gb_egl->dumped_state;

    if(was_paused) {
        mobile_gb_egl->gb.pause();
    }

    lock.unlock();

    callback(data.data(), data.size());
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void
set_paused(bool paused)
{
    std::lock_guard<std::mutex> _(m);
    if (mobile_gb_egl) {
        if(paused) {
            mobile_gb_egl->gb.pause();
        }else {
            mobile_gb_egl->gb.resume();
        }
    }
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void Java_com_example_native_1add_OpenglTexturePlugin_nativeSetSurface(JNIEnv * jenv, jobject obj, jobject surface) {
    __android_log_write(ANDROID_LOG_INFO, "gbegl", "Set surface");
    std::lock_guard<std::mutex> _(window_mutex);
    window = flutter_window(jenv, surface);
}
