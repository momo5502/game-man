#include "std_include.hpp"
#include "game_boy.hpp"

game_boy::game_boy(joypad* joypad, display* display)
	: joypad_(joypad)
	  , display_(display)
	  , input_(this)
	  , cpu_(this)
	  , mmu_(this)
	  , gpu_(this)
{
}

game_boy::~game_boy()
{
	this->off_ = true;
}

void game_boy::serialize(utils::binary_buffer& buffer)
{
  this->cpu_.serialize(buffer);
  this->mmu_.serialize(buffer);
  this->gpu_.serialize(buffer);
}

void game_boy::run()
{
	this->off_ = false;
	while (!this->off_ && this->frame())
	{
	    while(this->paused_)
	    {
	        if(this->off_ || !this->get_display()->is_on())
		{
	            break;
	        }

	        std::this_thread::sleep_for(std::chrono::milliseconds(10));
	    }
	}
}

void game_boy::skip_bios()
{
	this->cpu_.skip_bios();
}

void game_boy::turn_off()
{
	this->off_ = true;
}

void game_boy::pause()
{
  this->paused_ = true;
}

void game_boy::resume()
{
  this->paused_ = false;
}

bool game_boy::is_paused()
{
	return this->paused_;
}

bool game_boy::frame()
{
	const uint32_t end_tick = this->cpu_.registers.m + 17556;

	const auto start = std::chrono::high_resolution_clock::now();

	while (this->cpu_.registers.m < end_tick)
	{
		if (!this->cpu_.execute())
		{
			return false;
		}
	}

	const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now() - start);

	if (delta < (15ms))
	{
		std::this_thread::sleep_for((15ms) - delta);
	}

	return true;
}

void game_boy::load_rom(std::vector<uint8_t> data)
{
	if (data.size() < sizeof(gb_rom))
	{
		throw std::runtime_error("Invalid rom");
	}

	auto* rom = reinterpret_cast<const gb_rom*>(data.data());

	std::string rom_name(rom->title, 16);
	while (!rom_name.empty() && !rom_name.back()) rom_name.pop_back();
	this->display_->set_title(std::move(rom_name));

	this->gpu_.set_is_color_gb(false);
	this->mmu_.load_rom(std::move(data));
}
