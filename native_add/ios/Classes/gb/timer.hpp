#pragma once
#include "serializable.hpp"

class game_boy;
class cpu;

class timer : public serializable
{
public:
	timer();
	~timer();
	
        void serialize(utils::binary_buffer& buffer) override;

	void increment(game_boy* gb);

	uint32_t div{};
	uint32_t tma{};
	uint32_t tima{};
	uint32_t tac{};

	uint32_t main_clock{};
	uint32_t sub_clock{};
	uint32_t div_clock{};

private:
	void step(game_boy* gb);
};
