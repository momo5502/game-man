#include "std_include.hpp"
#include "timer.hpp"
#include "game_boy.hpp"

timer::timer() = default;
timer::~timer() = default;


void timer::serialize(utils::binary_buffer& buffer)
{
  buffer.handle(this->div);
  buffer.handle(this->tma);
  buffer.handle(this->tima);
  buffer.handle(this->tac);
  buffer.handle(this->main_clock);
  buffer.handle(this->sub_clock);
  buffer.handle(this->div_clock);
}

void timer::increment(game_boy* gb)
{
	this->sub_clock += gb->get_cpu()->registers.m;

	if (this->sub_clock > 3)
	{
		this->main_clock++;
		this->sub_clock -= 4;

		this->div_clock++;
		if (this->div_clock == 16)
		{
			this->div_clock = 0;
			this->div++;
			this->div &= 0xFF;
		}
	}

	if (this->tac & 4)
	{
		switch (this->tac & 3)
		{
		case 0:
			if (this->main_clock >= 64) this->step(gb);
			break;

		case 1:
			if (this->main_clock >= 1) this->step(gb);
			break;

		case 2:
			if (this->main_clock >= 4) this->step(gb);
			break;

		case 3:
			if (this->main_clock >= 16) this->step(gb);
			break;

		default:
			break;
		}
	}
}

void timer::step(game_boy* gb)
{
	this->tima++;
	this->main_clock = 0;

	if (this->tima > 0xFF)
	{
		this->tima = this->tma;
		if (gb->get_mmu()->i_e & 4) gb->get_mmu()->i_f |= 4;
	}
}
