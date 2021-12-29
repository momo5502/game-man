#include "std_include.hpp"
#include "gpu.hpp"
#include "game_boy.hpp"
#include "utils/utils.hpp"

gpu::gpu(game_boy* game_boy) : gb_(game_boy), mode_(mode_hblank), clock_(0)
{
	zero_object(this->mem_);
	zero_object(this->tiles_);
	zero_object(this->screen_buffer_);
	zero_object(this->objects_);
}

gpu::~gpu() = default;

void gpu::serialize(utils::binary_buffer& buffer)
{
	buffer.handle(this->mode_);
	buffer.handle(this->mem_);
	buffer.handle(this->clock_);
	buffer.handle(this->last_time_);
	buffer.handle(this->is_color_gb_);
	buffer.handle(this->screen_buffer_);
	buffer.handle(this->tiles_);
	buffer.handle(this->objects_);
}

void gpu::render_texture() const
{
	this->gb_->get_display()->draw_frame(this->screen_buffer_);
}

void gpu::render_screen()
{
	uint8_t scanrow[GB_WIDTH] = {0};

	if (this->mem_.flags & flag_background_on)
	{
		const uint16_t linebase = GB_WIDTH * this->mem_.curline;
		const uint16_t mapaddress = ((this->mem_.flags & flag_alt_tile_map) ? 0x1C00 : 0x1800);
		const uint16_t mapbase = mapaddress + ((((this->mem_.curline +
			this->mem_.yscrl) & 255) >> 3) << 5);
		const uint8_t y = (this->mem_.curline + this->mem_.yscrl) & 7;
		uint8_t x = this->mem_.xscrl & 7;
		uint8_t t = (this->mem_.xscrl >> 3) & 31;

		uint16_t tile = this->gb_->get_mmu()->vram[mapbase + t];
		for (int32_t i = 0; i < GB_WIDTH; ++i)
		{
			if (this->mem_.flags & flag_alt_tile_set)
			{
			}
			else tile = 0x100 + static_cast<int8_t>(tile);

			scanrow[i] = this->tiles_[tile][y][x];
			this->screen_buffer_[linebase + i] = this->get_color_from_palette(0, scanrow[i]);

			x++;
			if (x == 8)
			{
				t = (t + 1) & 31;
				x = 0;
				tile = this->gb_->get_mmu()->vram[mapbase + t];
			}
		}
	}
	if (this->mem_.flags & flag_sprites_on)
	{
		for (int32_t i = 0; i < 40; i++)
		{
			const auto obj = this->objects_[i];

			// Check if this sprite falls on this scanline
			if (obj.y <= this->mem_.curline && (obj.y + 8) > this->mem_.curline)
			{
				// Where to render on the canvas
				auto canvasoffs = (this->mem_.curline * 160 + obj.x);

				// If the sprite is Y-flipped,
				// use the opposite side of the tile
				uint8_t* tilerow;
				if (obj.y_flip)
				{
					tilerow = this->tiles_[obj.tile][7 - (this->mem_.curline - obj.y)];
				}
				else
				{
					tilerow = this->tiles_[obj.tile][this->mem_.curline - obj.y];
				}

				for (int32_t x = 0; x < 8; x++)
				{
					// If this pixel is still on-screen, AND
					// if it's not colour 0 (transparent), AND
					// if this sprite has priority OR shows under the bg
					// then render the pixel
					if ((obj.x + x) >= 0 && (obj.x + x) < 160 && (!obj.priority || !
						scanrow[obj.x + x]))
					{
						const unsigned index = tilerow[obj.x_flip ? (7 - x) : x];
						if (index)
						{
							// If the sprite is X-flipped,
							// write pixels in reverse order
							const auto color = this->get_color_from_palette(1 + (obj.palette != 0),
							                                                index);
							this->screen_buffer_[canvasoffs] = color;
						}
					}

					canvasoffs++;
				}
			}
		}
	}
}

void gpu::frame()
{
	const int32_t time = this->gb_->get_cpu()->registers.m;
	this->clock_ += time - this->last_time_;
	this->last_time_ = time;

	switch (this->mode_)
	{
	case mode_hblank:
		{
			if (this->clock_ >= 51)
			{
				this->clock_ -= 51;
				this->mem_.curline++;

				if (this->mem_.curline == GB_HEIGHT)
				{
					this->mode_ = mode_vblank;
					this->render_texture();
					if (this->gb_->get_mmu()->i_e & 1) this->gb_->get_mmu()->i_f |= 1;
					if (this->mem_.lcd_status & (1 << 4) && this->gb_->get_mmu()->i_e & 2)
						this->gb_->get_mmu()->i_f |=
							2;
				}
				else
				{
					this->mode_ = mode_oam;
					if (this->mem_.lcd_status & (1 << 5) && this->gb_->get_mmu()->i_e & 2)
						this->gb_->get_mmu()->i_f |=
							2;
				}
			}
			break;
		}

	case mode_vblank:
		{
			if (this->clock_ >= 114)
			{
				this->clock_ -= 114;
				this->mem_.curline++;

				if (this->mem_.curline > 153)
				{
					this->mode_ = mode_oam;
					this->mem_.curline = 0;

					if (this->mem_.lcd_status & (1 << 5) && this->gb_->get_mmu()->i_e & 2)
						this->gb_->get_mmu()->i_f |=
							2;
				}
			}
			break;
		}

	case mode_oam:
		{
			if (this->clock_ >= 20)
			{
				this->clock_ -= 20;
				this->mode_ = mode_vram;
			}
			break;
		}

	case mode_vram:
		{
			if (this->clock_ >= 43)
			{
				this->clock_ -= 43;
				this->mode_ = mode_hblank;
				if (this->mem_.lcd_status & (1 << 3) && this->gb_->get_mmu()->i_e & 2) this->gb_->get_mmu()->i_f |= 2;

				if (this->mem_.flags & flag_display_on)
				{
					this->render_screen();
				}
			}
			break;
		}
	}
}

uint8_t* gpu::get_memory_ptr(uint16_t address)
{
	address -= 0xFF40;

	if (address < sizeof(this->mem_))
	{
		const auto pointer = reinterpret_cast<uint8_t*>(&this->mem_) + address;

		if (pointer == &this->mem_.lcd_status)
		{
			this->mem_.lcd_status = static_cast<uint8_t>(this->mode_ | (this->mem_.curline == this->mem_.raster
				                                                            ? 0
				                                                            : 0));
		}

		return pointer;
	}

	return nullptr;
}

void gpu::update_object(const uint16_t address, const uint8_t value)
{
	const int32_t obj = (address - 0xFE00) >> 2;
	if (obj < 40)
	{
		switch (address & 3)
		{
			// Y-coordinate
		case 0: this->objects_[obj].y = value - 16;
			break;

			// X-coordinate
		case 1: this->objects_[obj].x = value - 8;
			break;

			// Data tile
		case 2: this->objects_[obj].tile = value;
			break;

			// Options
		case 3:
			this->objects_[obj].palette = (value & 0x10) ? 1 : 0;
			this->objects_[obj].x_flip = (value & 0x20) ? 1 : 0;
			this->objects_[obj].y_flip = (value & 0x40) ? 1 : 0;
			this->objects_[obj].priority = (value & 0x80) ? 1 : 0;
			break;
		}
	}
}

void gpu::set_is_color_gb(const bool value)
{
	this->is_color_gb_ = value;
}

void gpu::update_tile(uint16_t addr)
{
	addr &= 0x1ffe;
	const uint16_t tile = (addr >> 4) & 511;
	const uint16_t y = (addr >> 1) & 7;
	for (uint16_t x = 0; x < 8; x++)
	{
		const uint16_t sx = 1 << (7 - x);

		uint8_t var = (this->gb_->get_mmu()->vram[addr] & sx) ? 1 : 0;
		var |= (this->gb_->get_mmu()->vram[addr + 1] & sx) ? 2 : 0;
		var &= 3;

		this->tiles_[tile][y][x] = var;
	}
}

color gpu::get_color_from_palette(const uint32_t palette, const uint32_t index)
{
	if (palette > 3 || index > 4) return color{0, 0, 0, 0};

	auto* quad = reinterpret_cast<gpu::gbc_pixel_quad*>(&this->mem_.palette[palette]);

	gpu::gb_color color{};
	switch (index)
	{
	case 0: color = quad->_1;
		break;
	case 1: color = quad->_2;
		break;
	case 2: color = quad->_3;
		break;
	case 3: color = quad->_4;
		break;
	default: color = gbc_white;
		break;
	}

	return gpu::get_gb_color(color);
}

color gpu::get_gb_color(const gpu::gb_color pixel)
{
	switch (pixel)
	{
	case gbc_black: return color{0, 0, 0, 255};
	case gbc_dark_gray: return color{192, 192, 192, 255};
	case gbc_light_gray: return color{96, 96, 96, 255};
	case gbc_white: return color{255, 255, 255, 255};
	}
	return color{0, 0, 0, 0};
}

color gpu::get_gb_color(uint8_t pixel)
{
	return gpu::get_gb_color(*reinterpret_cast<gpu::gb_color*>(&pixel));
}
