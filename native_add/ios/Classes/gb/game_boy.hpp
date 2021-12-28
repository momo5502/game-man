#pragma once
#include "joypad.hpp"
#include "display.hpp"

#include "cpu.hpp"
#include "gpu.hpp"
#include "mmu.hpp"
#include "input.hpp"

struct gb_rom
{
	uint8_t padding[0x100];
	uint8_t entry_point[4];
	uint8_t logo[48];
	char title[16];
	uint16_t publisher;
	uint8_t sgb_flag;
	uint8_t cartridge_type;
	uint8_t rom_size;
	uint8_t ram_size;
	uint8_t destination;
	uint8_t old_publisher;
	uint8_t rom_version;
	uint8_t header_checksum;
	uint16_t global_checksum;
};

class game_boy : public serializable
{
public:
	game_boy(joypad* joypad, display* display);
	~game_boy();

	mmu* get_mmu() { return &this->mmu_; }
	gpu* get_gpu() { return &this->gpu_; }
	cpu* get_cpu() { return &this->cpu_; }
	input* get_input() { return &this->input_; }
	joypad* get_joypad() const { return this->joypad_; }
	display* get_display() const { return this->display_; }

	void load_rom(std::vector<uint8_t> data);
	void run();

	void skip_bios();

	void turn_off();

	void pause();
	void resume();
	bool is_paused();

	void serialize(utils::binary_buffer& buffer) override;

private:
	joypad* joypad_;
	display* display_;
	input input_;
	cpu cpu_;
	mmu mmu_;
	gpu gpu_;
	std::atomic<bool> off_{false};
	std::atomic<bool> paused_{false};

	bool frame();
};
