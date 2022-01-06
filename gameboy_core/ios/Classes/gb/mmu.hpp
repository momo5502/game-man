#pragma once
#include "serializable.hpp"

struct gb_rom;
class cpu;
class game_boy;

struct mbc
{
	int32_t rom_bank = 1;
	int32_t ram_bank = 0;
	int32_t ram_on = 0;
	int32_t mode = 0;
};

class mmu : public serializable
{
public:
	mmu(game_boy* game_boy);
	~mmu();

	void load_rom(std::vector<uint8_t> data);

	uint8_t read_byte(uint16_t address);
	uint16_t read_word(uint16_t address);

	void write_byte(uint16_t address, uint8_t value);
	void write_word(uint16_t address, uint16_t value);

	uint8_t* get_memory_ptr(uint16_t address);

	void control_mbc(uint16_t address, uint8_t value);

	void mark_bios_pass();

	uint8_t vram[0x2000]{};
	uint8_t eram[0x8000]{};
	uint8_t wram[0x2000]{};
	uint8_t zram[0x7F]{};

	uint8_t i_f{};
	uint8_t i_e{};

	uint32_t rom_offset = 0x4000;
	uint32_t ram_offset = 0x0000;
	uint8_t cartridge_type = 0;

	mbc mbc[4]{};

	gb_rom* get_rom();

	void serialize(utils::binary_buffer& buffer) override;

private:
	static uint8_t bios_[256];

	std::vector<uint8_t> rom_{};
	uint8_t oam_[160]{};
	uint8_t zero_[2]{};
	bool passed_bios_{false};

	game_boy* gb_;
};
