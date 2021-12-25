#pragma once
#include "timer.hpp"

class game_boy;

enum flags
{
	flag_zero = 0x80,
	flag_negative = 0x40,
	flag_half_carry = 0x20,
	flag_carry = 0x10,
};

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
struct cpu_registers
{
	union
	{
		struct
		{
			uint8_t f;
			uint8_t a;
		};

		uint16_t af;
	};

	union
	{
		struct
		{
			uint8_t c;
			uint8_t b;
		};

		uint16_t bc;
	};

	union
	{
		struct
		{
			uint8_t e;
			uint8_t d;
		};

		uint16_t de;
	};

	union
	{
		struct
		{
			uint8_t l;
			uint8_t h;
		};

		uint16_t hl;
	};

	uint16_t sp;
	uint16_t pc;

	uint32_t m;
};
#ifdef _WIN32
#pragma warning(pop)
#endif

class cpu
{
public:
	typedef void (*operation)(game_boy*);

	cpu(game_boy* game_boy);
	~cpu();

	void stack_push_word(uint16_t value);
	void stack_push_byte(uint8_t value);

	uint16_t stack_pop_word();
	uint8_t stack_pop_byte();

	uint8_t read_program_byte();
	uint16_t read_program_word();

	bool execute();
	void execute_ext(uint8_t instruction);

	cpu_registers registers;
	timer timer;

	void skip_bios();

private:
	operation operations_[0x100];
	static const uint8_t operation_ticks[0x100];

	operation ext_operations_[0x100];
	static const uint8_t ext_operation_ticks[0x100];

	bool ime_;
	game_boy* gb_;
	cpu_registers sav_registers_;

	bool halted_ = false;

	void setup_operations();
	void setup_ext_operations();

	void execute_rst(uint16_t num);

	void add_hl(uint16_t value);

	void inc(uint8_t* reg);
	void dec(uint8_t* reg);
	void add(uint8_t reg);
	void sub(uint8_t reg);
	void _and(uint8_t reg);
	void _xor(uint8_t reg);
	void _or(uint8_t reg);
	void cp(uint8_t reg);
	void adc(uint8_t reg);
	void sbc(uint8_t reg);

	void bit(uint8_t reg, uint8_t _bit);

	void rlc(uint8_t* reg);
	void rrc(uint8_t* reg);

	void rl(uint8_t* reg);
	void rr(uint8_t* reg);

	void sla(uint8_t* reg);
	void sra(uint8_t* reg);

	void swap(uint8_t* reg);
	void srl(uint8_t* reg);
};
