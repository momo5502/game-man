#include <stdint.h>
#include <thread>
#include <string>
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include "gb/game_boy.hpp"

#include <android/log.h>

class joypad_mobile : public joypad {
public:
	virtual bool is_up_pressed() override{
        return states[0];
    }

	virtual bool is_down_pressed() override{
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

class display_mobile : public display {
public:
    void draw_frame(const color* buffer/* GB_WIDTH * GB_HEIGHT */) override {
        //__android_log_write(ANDROID_LOG_INFO, "drawing", "Drawing!");
        memcpy(data, buffer, sizeof(data));
        available.store(true);
    }

    void* get_data() {
        if(available.load()) {
            available.store(false);
        }
        return data;
    }

    void set_title(std::string title) override {
        __android_log_write(ANDROID_LOG_INFO, "gb", title.data());
    }

private:
    static color data[GB_WIDTH * GB_HEIGHT];
    std::atomic<bool> available{false};
};

color display_mobile::data[GB_WIDTH * GB_HEIGHT]{};

class MobileGameBoy
{
public:
    display_mobile display{};
    joypad_mobile joypad{};
    game_boy gb;
    std::thread runner;

    MobileGameBoy()
        : gb(&joypad, &display)
    {

    }

    ~MobileGameBoy() {
        __android_log_write(ANDROID_LOG_INFO, "destroying", "destroying!");
        gb.turn_off();
        if(runner.joinable()) {
            runner.join();
        }
    }

    void run(void* data, uint64_t size)
    {
        std::atomic<bool> running{false};
        runner = std::thread([&running, this, data, size]() {
            uint8_t* start = reinterpret_cast<uint8_t*>(data); 
            std::vector<uint8_t> data;
            data.assign(start, start + size);

            gb.load_rom(std::move(data));
            gb.skip_bios();
            running = true;
            gb.run();
            __android_log_write(ANDROID_LOG_INFO, "term", "Terminating!");
        });

        while(!running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};

std::mutex m;
std::unique_ptr<MobileGameBoy> mobileGb{};

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void
press_button(int id, bool value)
{
    std::lock_guard<std::mutex> _(m);
    if(mobileGb) {
        mobileGb->joypad.set_state(id, value);
    }
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void*
get_buffer()
{
    std::lock_guard<std::mutex> _(m);
    if(!mobileGb) {
        return nullptr;
    }

    return mobileGb->display.get_data();
}

uint32_t jenkins_one_at_a_time_hash(char *key, size_t len)
{
    uint32_t hash, i;
    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

unsigned long reg32 = 0xffffffff;     // Schieberegister
 
unsigned long crc32_bytecalc(unsigned char byte)
{
  int i;
  unsigned long polynom = 0xEDB88320;    // Generatorpolynom
    for (i=0; i<8; ++i)
  {
        if ((reg32&1) != (byte&1))
             reg32 = (reg32>>1)^polynom; 
        else 
             reg32 >>= 1;
    byte >>= 1;
  }
  return reg32 ^ 0xffffffff;       // inverses Ergebnis, MSB zuerst
}
unsigned long crc32_messagecalc(unsigned char *data, int len)
{
  int i;
  for(i=0; i<len; i++) 
  {
    crc32_bytecalc(data[i]);    // Berechne fuer jeweils 8 Bit der Nachricht
  }
  return reg32 ^ 0xffffffff;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void
load_rom(void* data, uint64_t size)
{
    __android_log_write(ANDROID_LOG_INFO, "gb", ("Loading rom... " + std::to_string(crc32_messagecalc((unsigned char*)data, size)) + " " + std::to_string(size)).data());
    std::lock_guard<std::mutex> _(m);
    mobileGb = std::make_unique<MobileGameBoy>();
    mobileGb->run(data, size);
}
