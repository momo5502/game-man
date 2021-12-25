#include "std_include.hpp"

#include "mmu.hpp"
#include "game_boy.hpp"

#include "utils/utils.hpp"


uint8_t mmu::bios_[256] =
{
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
	0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
	0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
	0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
	0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
	0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
	0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
	0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

mmu::mmu(game_boy* game_boy) : gb_(game_boy)
{
	zero_object(this->vram);
	zero_object(this->eram);
	zero_object(this->wram);
	zero_object(this->zram);

	zero_object(this->oam_);
	zero_object(this->zero_);
}

gb_rom* mmu::get_rom()
{
	return reinterpret_cast<gb_rom*>(this->rom_.data());
}

void mmu::load_rom(std::vector<uint8_t> data)
{
	this->rom_ = std::move(data);
	this->cartridge_type = this->get_rom()->cartridge_type;
}

uint8_t mmu::read_byte(const uint16_t address)
{
	if (const auto mem = this->get_memory_ptr(address))
	{
		if (address == 0xFF00)
		{
			return this->gb_->get_input()->read();
		}

		return *mem;
	}

	throw std::runtime_error("Nullptr dereferenced!");
}

uint16_t mmu::read_word(const uint16_t address)
{
	const uint16_t low = this->read_byte(address);
	const uint16_t high = this->read_byte(address + 1);
	return low | (high << 8);
}

void mmu::write_byte(const uint16_t address, const uint8_t value)
{
	this->control_mbc(address, value);
	if (address < 0x8000) return;

	if (const auto mem = this->get_memory_ptr(address))
	{
		*mem = value;

		if (address == 0xFF46) // OAM DMA
		{
			for (uint16_t i = 0; i < 160; ++i)
			{
				const auto val = *this->get_memory_ptr((value << 8) + i);
				this->oam_[i] = val;
				this->gb_->get_gpu()->update_object(0xFE00 + i, val);
			}
		}

		if (mem >= this->vram && mem < &this->vram[sizeof this->vram])
		{
			this->gb_->get_gpu()->update_tile(address);
		}
		else if (address >= 0xFE00 && address <= 0xFEFF)
		{
			this->gb_->get_gpu()->update_object(address, value);
		}
		else if (static_cast<void*>(mem) == &this->gb_->get_cpu()->timer.div)
		{
			this->gb_->get_cpu()->timer.div = 0;
		}
		else if (static_cast<void*>(mem) == &this->gb_->get_cpu()->timer.tac)
		{
			this->gb_->get_cpu()->timer.tac &= 7;
		}
		else if (address == 0xFF00)
		{
			this->gb_->get_input()->write(value);
		}
	}
	else throw std::runtime_error("Nullptr dereferenced!");
}

void mmu::write_word(const uint16_t address, const uint16_t value)
{
	this->write_byte(address, value & 0xFF);
	this->write_byte(address + 1, value >> 8);
}

void mmu::mark_bios_pass()
{
	this->passed_bios_ = true;
}

void mmu::control_mbc(const uint16_t address, uint8_t value)
{
	switch (address & 0xF000)
	{
		// MBC1: External RAM switch
	case 0x0000:
	case 0x1000:
		switch (this->cartridge_type)
		{
		case 2:
		case 3:
			this->mbc[1].ram_on = ((value & 0x0F) == 0x0A) ? 1 : 0;
			break;
		}
		break;

		// MBC1: ROM bank
	case 0x2000:
	case 0x3000:
		switch (this->cartridge_type)
		{
		case 1:
		case 2:
		case 3:
			// Set lower 5 bits of ROM bank (skipping #0)
			value &= 0x1F;
			if (!value) value = 1;
			this->mbc[1].rom_bank = (this->mbc[1].rom_bank & 0x60) | value;

			// Calculate ROM offset from bank
			this->rom_offset = this->mbc[1].rom_bank * 0x4000;
			break;
		}
		break;

		// MBC1: RAM bank
	case 0x4000:
	case 0x5000:
		switch (this->cartridge_type)
		{
		case 1:
		case 2:
		case 3:
			if (this->mbc[1].mode)
			{
				// RAM mode: Set bank
				this->mbc[1].ram_bank = value & 3;
				this->ram_offset = this->mbc[1].ram_bank * 0x2000;
			}
			else
			{
				// ROM mode: Set high bits of bank
				this->mbc[1].rom_bank = (this->mbc[1].rom_bank & 0x1F) | ((value & 3) << 5);
				this->rom_offset = this->mbc[1].rom_bank * 0x4000;
			}
			break;
		}
		break;

		// MBC1: Mode switch
	case 0x6000:
	case 0x7000:
		switch (this->cartridge_type)
		{
		case 2:
		case 3:
			this->mbc[1].mode = value & 1;
			break;
		}
		break;
	}
}

uint8_t* mmu::get_memory_ptr(const uint16_t address)
{
	switch (address & 0xF000)
	{
		// BIOS / ROM0
	case 0x0000:
		{
			if (!this->passed_bios_)
			{
				if (address < 0x100)
				{
					return &mmu::bios_[address];
				}
			}

			if (address >= this->rom_.size()) throw std::runtime_error("Rom not loaded!");
			return &const_cast<uint8_t*>(this->rom_.data())[address];
		}

		// ROM0
	case 0x1000:
	case 0x2000:
	case 0x3000:
		{
			if (address >= this->rom_.size()) throw std::runtime_error("Rom not loaded!");
			return &const_cast<uint8_t*>(this->rom_.data())[address];
		}

		// ROM1
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		{
			const size_t new_address = this->rom_offset + (address - 0x4000);
			if (new_address >= this->rom_.size()) throw std::runtime_error("Rom not loaded!");
			return &const_cast<uint8_t*>(this->rom_.data())[new_address];
		}

		// VRAM
	case 0x8000:
	case 0x9000:
		{
			return &this->vram[address & 0x1FFF];
		}

		// ERAM
	case 0xA000:
	case 0xB000:
		{
			return &this->eram[this->ram_offset + (address - 0xA000)];
		}

		// WRAM
	case 0xC000:
	case 0xD000:
	case 0xE000: // Shadow
		{
			return &this->wram[address & 0x1FFF];
		}

	case 0xF000:
		{
			const uint16_t lh_addr = address & 0x0F00;
			if (lh_addr == 0x0F00) // Zero page
			{
				if (address == 0xFFFF) return &this->i_e;

				if (address >= 0xFF80)
				{
					return &this->zram[address & 0x7F];
				}
				else // TODO: Implement
				{
					if (address & 0x30)
					{
						this->zero_[0] = 0;
						this->zero_[1] = 0;
						return &this->zero_[0];
					}
					if ((address & 0xF0) >= 0x40 && (address & 0xF0) <= 0x70)
					{
						return this->gb_->get_gpu()->get_memory_ptr(address);
					}

					// Joypad
					if (address >= 0xFF00 && address <= 0xFF03)
					{
						this->zero_[0] = 0;
						this->zero_[1] = 0;
						return &this->zero_[0];
					}

					// Timer
					if (address == 0xFF04) return reinterpret_cast<uint8_t*>(&this->gb_->get_cpu()->timer.div);
					if (address == 0xFF05) return reinterpret_cast<uint8_t*>(&this->gb_->get_cpu()->timer.tima);
					if (address == 0xFF06) return reinterpret_cast<uint8_t*>(&this->gb_->get_cpu()->timer.tma);
					if (address == 0xFF07) return reinterpret_cast<uint8_t*>(&this->gb_->get_cpu()->timer.tac);

					// Others
					if (address >= 0xFF08 && address <= 0xFF0E)
					{
						this->zero_[0] = 0;
						this->zero_[1] = 0;
						return &this->zero_[0];
					}

					if (address == 0xFF0F)
					{
						return &this->i_f;
					}

					throw std::runtime_error(utils::va("Not implemented (%X)!", address));
					/*
					this->zero[0] = 0;
					this->zero[1] = 0;
					return &this->zero[0];
					*/
				}
			}
			else if (lh_addr == 0x0E00) // OAM
			{
				if (address < 0xFEA0)
				{
					return &this->oam_[address & 0xFF]; // GPU memory
				}
				else
				{
					this->zero_[0] = 0;
					this->zero_[1] = 0;
					return &this->zero_[0];
				}
			}
			else
			{
				return &this->wram[address & 0x1FFF];
			}
		}

	default:
		{
			return nullptr;
		}
	}
}

mmu::~mmu()
{
}
