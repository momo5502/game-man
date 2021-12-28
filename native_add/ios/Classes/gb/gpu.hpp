#pragma once
#include "display.hpp"
#include "serializable.hpp"

class game_boy;

class gpu : public serializable
{
public:
	gpu(game_boy* game_boy);
	~gpu();

	void frame();

	uint8_t* get_memory_ptr(uint16_t address);
	void update_tile(uint16_t address);
	void update_object(uint16_t address, uint8_t value);
	void set_is_color_gb(bool value);
	
        void serialize(utils::binary_buffer& buffer) override;

private:
	struct memory
	{
		uint8_t flags;
		uint8_t lcd_status;
		uint8_t yscrl;
		uint8_t xscrl;
		uint8_t curline;
		uint8_t raster;
		uint8_t unk;
		uint8_t palette[3];

		uint8_t reg[0xFF];
	};

	struct object
	{
		int32_t y;
		int32_t x;
		int32_t tile;
		int32_t palette;
		int32_t x_flip;
		int32_t y_flip;
		int32_t priority;
	};

	enum gb_color : uint8_t
	{
		gbc_white = 0,
		gbc_light_gray = 1,
		gbc_dark_gray = 2,
		gbc_black = 3,
	};

	enum mode
	{
		mode_hblank = 0,
		mode_vblank = 1,
		mode_oam = 2,
		mode_vram = 3
	};

	struct gbc_pixel_quad
	{
		gb_color _1 : 2;
		gb_color _2 : 2;
		gb_color _3 : 2;
		gb_color _4 : 2;
	};

	enum flags
	{
		flag_background_on = (1 << 0),
		flag_sprites_on = (1 << 1),
		flag_sprites_size = (1 << 2),
		flag_alt_tile_map = (1 << 3),
		flag_alt_tile_set = (1 << 4),
		flag_window_on = (1 << 5),
		flag_alt_window_tile_map = (1 << 6),
		flag_display_on = (1 << 7)
	};

	game_boy* gb_;

	mode mode_;
	memory mem_{};
	uint32_t clock_;
	uint32_t last_time_ = 0;
	bool is_color_gb {false};

	color screen_buffer_[GB_WIDTH * GB_HEIGHT]{};
	uint8_t tiles_[512][8][8]{};
	object objects_[40]{};

	inline color get_color_from_palette(uint32_t palette, uint32_t index);

	static inline color get_gb_color(gb_color pixel);
	static inline color get_gb_color(uint8_t pixel);

	void render_screen();
	void render_texture() const;
};
