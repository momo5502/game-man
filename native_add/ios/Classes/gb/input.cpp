#include "std_include.hpp"
#include "input.hpp"
#include "game_boy.hpp"

input::input(game_boy* game_boy) : gb_(game_boy), column_(0)
{
}


uint8_t input::read() const
{
	uint8_t input = 0;

	if (this->column_ == 0x10)
	{
		input |= (!this->a() ? key_a : 0);
		input |= (!this->b() ? key_b : 0);
		input |= (!this->select() ? key_select : 0);
		input |= (!this->start() ? key_start : 0);
	}
	else if (this->column_ == 0x20)
	{
		input |= (!this->up() ? key_up : 0);
		input |= (!this->down() ? key_down : 0);
		input |= (!this->left() ? key_left : 0);
		input |= (!this->right() ? key_right : 0);
	}
	else input = 0xF;

	if ((input & 0xF) != 0xF && this->gb_->get_mmu()->i_e & 0x10)
	{
		this->gb_->get_mmu()->i_f |= 0x10;
	}

	input |= this->column_;

	return input;
}

void input::write(const uint8_t val)
{
	this->column_ = val & 0x30;
}

bool input::up() const
{
	return this->gb_->get_joypad()->is_up_pressed();
}

bool input::down() const
{
	return this->gb_->get_joypad()->is_down_pressed();
}

bool input::left() const
{
	return this->gb_->get_joypad()->is_left_pressed();
}

bool input::right() const
{
	return this->gb_->get_joypad()->is_right_pressed();
}

bool input::a() const
{
	return this->gb_->get_joypad()->is_a_pressed();
}

bool input::b() const
{
	return this->gb_->get_joypad()->is_b_pressed();
}

bool input::start() const
{
	return this->gb_->get_joypad()->is_start_pressed();
}

bool input::select() const
{
	return this->gb_->get_joypad()->is_select_pressed();
}
