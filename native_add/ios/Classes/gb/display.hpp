#pragma once
#include <cstdint>
#include <string>

#define GB_WIDTH (160)
#define GB_HEIGHT (144)

struct color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

class display
{
public:
	virtual ~display() = default;
	virtual void draw_frame(const color* buffer/* GB_WIDTH * GB_HEIGHT */) = 0;

	virtual bool is_on()
	{
		return true;
	}

	virtual void set_title(std::string title)
	{
		(void)title;
	}
};
