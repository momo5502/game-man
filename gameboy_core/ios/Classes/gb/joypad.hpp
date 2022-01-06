#pragma once

class joypad
{
public:
	virtual ~joypad() = default;

	virtual bool is_up_pressed() = 0;
	virtual bool is_down_pressed() = 0;
	virtual bool is_left_pressed() = 0;
	virtual bool is_right_pressed() = 0;

	virtual bool is_a_pressed() = 0;
	virtual bool is_b_pressed() = 0;

	virtual bool is_start_pressed() = 0;
	virtual bool is_select_pressed() = 0;
};
