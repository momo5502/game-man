#include "std_include.hpp"
#include "cpu.hpp"
#include "game_boy.hpp"

#include "utils/utils.hpp"

const uint8_t cpu::operation_ticks[0x100] =
{
	2, 6, 4, 4, 2, 2, 4, 2, 10, 4, 4, 4, 2, 2, 4, 2, // 0x0_
	2, 6, 4, 4, 2, 2, 4, 2, 6, 4, 4, 4, 2, 2, 4, 2, // 0x1_
	4, 6, 4, 4, 2, 2, 4, 2, 4, 4, 4, 4, 2, 2, 4, 2, // 0x2_
	4, 6, 4, 4, 6, 6, 6, 2, 4, 4, 4, 4, 2, 2, 4, 2, // 0x3_
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0x4_
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0x5_
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0x6_
	4, 4, 4, 4, 4, 4, 2, 4, 2, 2, 2, 2, 2, 2, 4, 2, // 0x7_
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0x8_
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0x9_
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0xa_
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0xb_
	4, 6, 6, 8, 6, 8, 4, 0, 4, 8, 6, 0, 6, 12, 4, 0, // 0xc_
	4, 6, 6, 0, 6, 8, 4, 0, 4, 8, 6, 0, 6, 0, 4, 0, // 0xd_
	6, 4, 4, 0, 0, 8, 4, 0, 8, 2, 8, 0, 0, 0, 4, 0, // 0xe_
	6, 4, 4, 2, 0, 8, 4, 0, 6, 4, 8, 2, 0, 0, 4, 0 // 0xf_
};

const uint8_t cpu::ext_operation_ticks[0x100] =
{
	8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x0_
	8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x1_
	8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x2_
	8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x3_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x4_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x5_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x6_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x7_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x8_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x9_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xa_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xb_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xc_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xd_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0xe_
	8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8 // 0xf_
};

cpu::cpu(game_boy* game_boy) : ime_(true), gb_(game_boy)
{
	zero_object(this->registers);
	zero_object(this->operations_);
	zero_object(this->ext_operations_);

	this->setup_operations();
	this->setup_ext_operations();

	int32_t impl_op = 0;
	int32_t impl_cb = 0;

	const int32_t op_size = array_size(this->operations_);
	const int32_t cb_size = array_size(this->ext_operations_);

	for (int32_t i = 0; i < std::min(op_size, cb_size); ++i)
	{
		if (this->operations_[i]) impl_op++;
		if (this->ext_operations_[i]) impl_cb++;
	}

	printf("Operation coverage: %d/%d\n", impl_op, op_size);
	printf("Extended operation coverage: %d/%d\n", impl_cb, cb_size);
}

cpu::~cpu() = default;

void cpu::setup_operations()
{
	// NOP
	this->operations_[0x00] = [](game_boy*)
	{
	};

	// LD BC,nn
	this->operations_[0x01] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.bc = gb->get_cpu()->read_program_word();
	};

	// LD (BC),A
	this->operations_[0x02] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.bc, gb->get_cpu()->registers.a);
	};

	// INC BC
	this->operations_[0x03] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.bc++;
	};

	// INC B
	this->operations_[0x04] = [](game_boy* gb)
	{
		gb->get_cpu()->inc(&gb->get_cpu()->registers.b);
	};

	// DEC B
	this->operations_[0x05] = [](game_boy* gb)
	{
		gb->get_cpu()->dec(&gb->get_cpu()->registers.b);
	};

	// LD B,n
	this->operations_[0x06] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b = gb->get_cpu()->read_program_byte();
	};

	// RLC A
	this->operations_[0x07] = [](game_boy* gb)
	{
		gb->get_cpu()->rlc(&gb->get_cpu()->registers.a);
	};

	// LD (nn),SP
	this->operations_[0x08] = [](game_boy* gb)
	{
		gb->get_mmu()->write_word(gb->get_cpu()->read_program_word(), gb->get_cpu()->registers.sp);
	};

	// ADD HL,BC
	this->operations_[0x09] = [](game_boy* gb)
	{
		gb->get_cpu()->add_hl(gb->get_cpu()->registers.bc);
	};

	// LD A,(BC)
	this->operations_[0x0A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_mmu()->read_byte(gb->get_cpu()->registers.bc);
	};

	// DEC BC
	this->operations_[0x0B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.bc--;
	};

	// INC C
	this->operations_[0x0C] = [](game_boy* gb)
	{
		gb->get_cpu()->inc(&gb->get_cpu()->registers.c);
	};

	// DEC C
	this->operations_[0x0D] = [](game_boy* gb)
	{
		gb->get_cpu()->dec(&gb->get_cpu()->registers.c);
	};

	// LD C,n
	this->operations_[0x0E] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c = gb->get_cpu()->read_program_byte();
	};

	// RRCA
	this->operations_[0x0F] = [](game_boy* gb)
	{
		gb->get_cpu()->rrc(&gb->get_cpu()->registers.a);
	};

	// STOP
	this->operations_[0x10] = [](game_boy*)
	{
		throw std::runtime_error("Stop not implemented!");
	};

	// LD DE,nn
	this->operations_[0x11] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.de = gb->get_cpu()->read_program_word();
	};

	// LD (DE),A
	this->operations_[0x12] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.de, gb->get_cpu()->registers.a);
	};

	// INC DE
	this->operations_[0x13] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.de++;
	};

	// INC D
	this->operations_[0x14] = [](game_boy* gb)
	{
		gb->get_cpu()->inc(&gb->get_cpu()->registers.d);
	};

	// DEC D
	this->operations_[0x15] = [](game_boy* gb)
	{
		gb->get_cpu()->dec(&gb->get_cpu()->registers.d);
	};

	// LD D,n
	this->operations_[0x16] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d = gb->get_cpu()->read_program_byte();
	};

	// RL A
	this->operations_[0x17] = [](game_boy* gb)
	{
		gb->get_cpu()->rl(&gb->get_cpu()->registers.a);
	};

	// JR n
	this->operations_[0x18] = [](game_boy* gb)
	{
		const int8_t jump_loc = gb->get_cpu()->read_program_byte();
		gb->get_cpu()->registers.pc += jump_loc;
	};

	// ADD HL,DE
	this->operations_[0x19] = [](game_boy* gb)
	{
		gb->get_cpu()->add_hl(gb->get_cpu()->registers.de);
	};

	// LD A,(DE)
	this->operations_[0x1A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_mmu()->read_byte(gb->get_cpu()->registers.de);
	};

	// DEC DE
	this->operations_[0x1B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.de--;
	};

	// INC E
	this->operations_[0x1C] = [](game_boy* gb)
	{
		gb->get_cpu()->inc(&gb->get_cpu()->registers.e);
	};

	// DEC E
	this->operations_[0x1D] = [](game_boy* gb)
	{
		gb->get_cpu()->dec(&gb->get_cpu()->registers.e);
	};

	// LD E,n
	this->operations_[0x1E] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e = gb->get_cpu()->read_program_byte();
	};

	// RR a
	this->operations_[0x1F] = [](game_boy* gb)
	{
		gb->get_cpu()->rr(&gb->get_cpu()->registers.a);
	};

	// JR NZ,n
	this->operations_[0x20] = [](game_boy* gb)
	{
		const int8_t jump_loc = gb->get_cpu()->read_program_byte();
		if (!(gb->get_cpu()->registers.f & flag_zero))
		{
			gb->get_cpu()->registers.pc += jump_loc;
			gb->get_cpu()->registers.m++;
		}
	};

	// LD HL,nn
	this->operations_[0x21] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.hl = gb->get_cpu()->read_program_word();
	};

	// LDI (HL),A
	this->operations_[0x22] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.a);
		gb->get_cpu()->registers.hl++;
	};

	// INC HL
	this->operations_[0x23] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.hl++;
	};

	// INC H
	this->operations_[0x24] = [](game_boy* gb)
	{
		gb->get_cpu()->inc(&gb->get_cpu()->registers.h);
	};

	// DEC H
	this->operations_[0x25] = [](game_boy* gb)
	{
		gb->get_cpu()->dec(&gb->get_cpu()->registers.h);
	};

	// LD H,n
	this->operations_[0x26] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h = gb->get_cpu()->read_program_byte();
	};

	// DAA
	this->operations_[0x27] = [](game_boy* gb)
	{
		int32_t a = gb->get_cpu()->registers.a;
		if (!(gb->get_cpu()->registers.f & flag_zero))
		{
			if (gb->get_cpu()->registers.f & flag_half_carry || ((a & 0xF) > 9))
				a += 0x06;

			if (gb->get_cpu()->registers.f & flag_carry || (a > 0x9F))
				a += 0x60;
		}
		else
		{
			if (gb->get_cpu()->registers.f & flag_half_carry)
				a = (a - 6) & 0xFF;

			if (gb->get_cpu()->registers.f & flag_carry)
				a -= 0x60;
		}

		gb->get_cpu()->registers.f &= ~(flag_half_carry | flag_zero);

		if ((a & 0x100) == 0x100)
			gb->get_cpu()->registers.f |= flag_carry;

		a &= 0xFF;
		if (!a) gb->get_cpu()->registers.f |= flag_zero;

		gb->get_cpu()->registers.a = static_cast<uint8_t>(a);
	};

	// JR Z,n
	this->operations_[0x28] = [](game_boy* gb)
	{
		const int8_t jump_loc = gb->get_cpu()->read_program_byte();
		if (gb->get_cpu()->registers.f & flag_zero)
		{
			gb->get_cpu()->registers.pc += jump_loc;
			gb->get_cpu()->registers.m++;
		}
	};

	// ADD HL,HL
	this->operations_[0x29] = [](game_boy* gb)
	{
		gb->get_cpu()->add_hl(gb->get_cpu()->registers.hl);
	};

	// LDI A,(HL)
	this->operations_[0x2A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->registers.hl++;
	};

	// DEC HL
	this->operations_[0x2B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.hl--;
	};

	// INC L
	this->operations_[0x2C] = [](game_boy* gb)
	{
		gb->get_cpu()->inc(&gb->get_cpu()->registers.l);
	};

	// DEC L
	this->operations_[0x2D] = [](game_boy* gb)
	{
		gb->get_cpu()->dec(&gb->get_cpu()->registers.l);
	};

	// LD L,n
	this->operations_[0x2E] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l = gb->get_cpu()->read_program_byte();
	};

	// CPL
	this->operations_[0x2F] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a ^= 0xFF;
		gb->get_cpu()->registers.f |= flag_half_carry | flag_negative;
	};

	// JR NC,n
	this->operations_[0x30] = [](game_boy* gb)
	{
		const int8_t jump_loc = gb->get_cpu()->read_program_byte();
		if (!(gb->get_cpu()->registers.f & flag_carry))
		{
			gb->get_cpu()->registers.pc += jump_loc;
			gb->get_cpu()->registers.m++;
		}
	};

	// LD SP,nn
	this->operations_[0x31] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.sp = gb->get_cpu()->read_program_word();
	};

	// LDD (HL),A
	this->operations_[0x32] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.a);
		gb->get_cpu()->registers.hl--;
	};

	// INC SP
	this->operations_[0x33] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.sp++;
	};

	// INC (HL)
	this->operations_[0x34] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.f &= flag_carry;

		const uint8_t value = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) + 1;
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, value);

		if (!value) gb->get_cpu()->registers.f |= flag_zero;
		if ((value & 0x0F) == 0x00) gb->get_cpu()->registers.f |= flag_half_carry;
	};

	// DEC (HL)
	this->operations_[0x35] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.f &= flag_carry;
		gb->get_cpu()->registers.f |= flag_negative;

		const uint8_t value = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) - 1;
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, value);

		if (!value) gb->get_cpu()->registers.f |= flag_zero;
		if ((value & 0x0F) == 0x0F) gb->get_cpu()->registers.f |= flag_half_carry;
	};

	// LD (HL),n
	this->operations_[0x36] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->read_program_byte());
	};

	// SCF
	this->operations_[0x37] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.f |= flag_carry;
		gb->get_cpu()->registers.f &= ~flag_half_carry;
		gb->get_cpu()->registers.f &= ~flag_negative;
	};

	// JR C,n
	this->operations_[0x38] = [](game_boy* gb)
	{
		const int8_t jump_loc = gb->get_cpu()->read_program_byte();
		if (gb->get_cpu()->registers.f & flag_carry)
		{
			gb->get_cpu()->registers.pc += jump_loc;
			gb->get_cpu()->registers.m++;
		}
	};

	// ADD HL,SP
	this->operations_[0x39] = [](game_boy* gb)
	{
		gb->get_cpu()->add_hl(gb->get_cpu()->registers.hl);
	};

	// LDD A,(HL)
	this->operations_[0x3A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->registers.hl--;
	};

	// DEC SP
	this->operations_[0x3B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.sp--;
	};

	// INC A
	this->operations_[0x3C] = [](game_boy* gb)
	{
		gb->get_cpu()->inc(&gb->get_cpu()->registers.a);
	};

	// DEC A
	this->operations_[0x3D] = [](game_boy* gb)
	{
		gb->get_cpu()->dec(&gb->get_cpu()->registers.a);
	};

	// LD A,n
	this->operations_[0x3E] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_cpu()->read_program_byte();
	};

	// CCF
	this->operations_[0x3F] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.f ^= flag_carry;
		gb->get_cpu()->registers.f &= ~flag_half_carry;
		gb->get_cpu()->registers.f &= ~flag_negative;
	};

	// LD B,B
	this->operations_[0x40] = [](game_boy* gb)
	{
		// ReSharper disable once CppIdenticalOperandsInBinaryExpression
		gb->get_cpu()->registers.b = gb->get_cpu()->registers.b;
	};

	// LD B,C
	this->operations_[0x41] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b = gb->get_cpu()->registers.c;
	};

	// LD B,D
	this->operations_[0x42] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b = gb->get_cpu()->registers.d;
	};

	// LD B,E
	this->operations_[0x43] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b = gb->get_cpu()->registers.e;
	};

	// LD B,H
	this->operations_[0x44] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b = gb->get_cpu()->registers.h;
	};

	// LD B,L
	this->operations_[0x45] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b = gb->get_cpu()->registers.l;
	};

	// LD B,(HL)
	this->operations_[0x46] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
	};

	// LD B,A
	this->operations_[0x47] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b = gb->get_cpu()->registers.a;
	};

	// LD C,B
	this->operations_[0x48] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c = gb->get_cpu()->registers.b;
	};

	// LD C,C
	this->operations_[0x49] = [](game_boy* gb)
	{
		// ReSharper disable once CppIdenticalOperandsInBinaryExpression
		gb->get_cpu()->registers.c = gb->get_cpu()->registers.c;
	};

	// LD C,D
	this->operations_[0x4A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c = gb->get_cpu()->registers.d;
	};

	// LD C,E
	this->operations_[0x4B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c = gb->get_cpu()->registers.e;
	};

	// LD C,H
	this->operations_[0x4C] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c = gb->get_cpu()->registers.h;
	};

	// LD C,L
	this->operations_[0x4D] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c = gb->get_cpu()->registers.l;
	};

	// LD C,(HL)
	this->operations_[0x4E] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
	};

	// LD C,A
	this->operations_[0x4F] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c = gb->get_cpu()->registers.a;
	};

	// LD D,B
	this->operations_[0x50] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d = gb->get_cpu()->registers.b;
	};

	// LD D,C
	this->operations_[0x51] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d = gb->get_cpu()->registers.c;
	};

	// LD D,D
	this->operations_[0x52] = [](game_boy* gb)
	{
		// ReSharper disable once CppIdenticalOperandsInBinaryExpression
		gb->get_cpu()->registers.d = gb->get_cpu()->registers.d;
	};

	// LD D,E
	this->operations_[0x53] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d = gb->get_cpu()->registers.e;
	};

	// LD D,H
	this->operations_[0x54] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d = gb->get_cpu()->registers.h;
	};

	// LD D,L
	this->operations_[0x55] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d = gb->get_cpu()->registers.l;
	};

	// LD D,(HL)
	this->operations_[0x56] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
	};

	// LD D,A
	this->operations_[0x57] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d = gb->get_cpu()->registers.a;
	};

	// LD E,B
	this->operations_[0x58] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e = gb->get_cpu()->registers.b;
	};

	// LD E,C
	this->operations_[0x59] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e = gb->get_cpu()->registers.c;
	};

	// LD E,D
	this->operations_[0x5A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e = gb->get_cpu()->registers.d;
	};

	// LD E,E
	this->operations_[0x5B] = [](game_boy* gb)
	{
		// ReSharper disable once CppIdenticalOperandsInBinaryExpression
		gb->get_cpu()->registers.e = gb->get_cpu()->registers.e;
	};

	// LD E,H
	this->operations_[0x5C] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e = gb->get_cpu()->registers.h;
	};

	// LD E,L
	this->operations_[0x5D] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e = gb->get_cpu()->registers.l;
	};

	// LD E,(HL)
	this->operations_[0x5E] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
	};

	// LD E,A
	this->operations_[0x5F] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e = gb->get_cpu()->registers.a;
	};

	// LD H,B
	this->operations_[0x60] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h = gb->get_cpu()->registers.b;
	};

	// LD H,C
	this->operations_[0x61] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h = gb->get_cpu()->registers.c;
	};

	// LD H,D
	this->operations_[0x62] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h = gb->get_cpu()->registers.d;
	};

	// LD H,E
	this->operations_[0x63] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h = gb->get_cpu()->registers.e;
	};

	// LD H,H
	this->operations_[0x64] = [](game_boy* gb)
	{
		// ReSharper disable once CppIdenticalOperandsInBinaryExpression
		gb->get_cpu()->registers.h = gb->get_cpu()->registers.h;
	};

	// LD H,L
	this->operations_[0x65] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h = gb->get_cpu()->registers.l;
	};

	// LD H,(HL)
	this->operations_[0x66] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
	};

	// LD H,A
	this->operations_[0x67] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h = gb->get_cpu()->registers.a;
	};

	// LD L,B
	this->operations_[0x68] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l = gb->get_cpu()->registers.b;
	};

	// LD L,C
	this->operations_[0x69] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l = gb->get_cpu()->registers.c;
	};

	// LD L,D
	this->operations_[0x6A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l = gb->get_cpu()->registers.d;
	};

	// LD L,E
	this->operations_[0x6B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l = gb->get_cpu()->registers.e;
	};

	// LD L,H
	this->operations_[0x6C] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l = gb->get_cpu()->registers.h;
	};

	// LD L,L
	this->operations_[0x6D] = [](game_boy* gb)
	{
		// ReSharper disable once CppIdenticalOperandsInBinaryExpression
		gb->get_cpu()->registers.l = gb->get_cpu()->registers.l;
	};

	// LD L,(HL)
	this->operations_[0x6E] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
	};

	// LD L,A
	this->operations_[0x6F] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l = gb->get_cpu()->registers.a;
	};

	// LD (HL),B
	this->operations_[0x70] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.b);
	};

	// LD (HL),C
	this->operations_[0x71] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.c);
	};

	// LD (HL),D
	this->operations_[0x72] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.d);
	};

	// LD (HL),E
	this->operations_[0x73] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.e);
	};

	// LD (HL),H
	this->operations_[0x74] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.h);
	};

	// LD (HL),L
	this->operations_[0x75] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.l);
	};

	// HALT
	this->operations_[0x76] = [](game_boy* gb)
	{
		//printf("Halt called :(\n");
		//throw std::runtime_error("Halt not implemented!");
		gb->get_cpu()->halted_ = true;
		gb->get_cpu()->ime_ = true;
	};

	// LD(HL), A
	this->operations_[0x77] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, gb->get_cpu()->registers.a);
	};

	// LD A,B
	this->operations_[0x78] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_cpu()->registers.b;
	};

	// LD A,C
	this->operations_[0x79] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_cpu()->registers.c;
	};

	// LD A,D
	this->operations_[0x7A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_cpu()->registers.d;
	};

	// LD A,E
	this->operations_[0x7B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_cpu()->registers.e;
	};

	// LD A,H
	this->operations_[0x7C] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_cpu()->registers.h;
	};

	// LD A,L
	this->operations_[0x7D] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_cpu()->registers.l;
	};

	// LD A,(HL)
	this->operations_[0x7E] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
	};

	// LD A,A
	this->operations_[0x7F] = [](game_boy* gb)
	{
		// ReSharper disable once CppIdenticalOperandsInBinaryExpression
		gb->get_cpu()->registers.a = gb->get_cpu()->registers.a;
	};

	// ADD A,B
	this->operations_[0x80] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_cpu()->registers.b);
	};

	// ADD A,C
	this->operations_[0x81] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_cpu()->registers.c);
	};

	// ADD A,D
	this->operations_[0x82] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_cpu()->registers.d);
	};

	// ADD A,E
	this->operations_[0x83] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_cpu()->registers.e);
	};

	// ADD A,H
	this->operations_[0x84] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_cpu()->registers.h);
	};

	// ADD A,L
	this->operations_[0x85] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_cpu()->registers.l);
	};

	// ADD A,(HL)
	this->operations_[0x86] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl));
	};

	// ADD A,A
	this->operations_[0x87] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_cpu()->registers.a);
	};

	// ADC A,B
	this->operations_[0x88] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_cpu()->registers.b);
	};

	// ADC A,C
	this->operations_[0x89] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_cpu()->registers.c);
	};

	// ADC A,D
	this->operations_[0x8A] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_cpu()->registers.d);
	};

	// ADC A,E
	this->operations_[0x8B] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_cpu()->registers.e);
	};

	// ADC A,H
	this->operations_[0x8C] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_cpu()->registers.h);
	};

	// ADC A,L
	this->operations_[0x8D] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_cpu()->registers.l);
	};

	// ADC A,(HL)
	this->operations_[0x8E] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl));
	};

	// ADC A,A
	this->operations_[0x8F] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_cpu()->registers.a);
	};

	// SUB A,B
	this->operations_[0x90] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_cpu()->registers.b);
	};

	// SUB A,C
	this->operations_[0x91] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_cpu()->registers.c);
	};

	// SUB A,D
	this->operations_[0x92] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_cpu()->registers.d);
	};

	// SUB A,E
	this->operations_[0x93] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_cpu()->registers.e);
	};

	// SUB A,H
	this->operations_[0x94] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_cpu()->registers.h);
	};

	// SUB A,L
	this->operations_[0x95] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_cpu()->registers.l);
	};

	// SUB A,(HL)
	this->operations_[0x96] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl));
	};

	// SUB A,A
	this->operations_[0x97] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_cpu()->registers.a);
	};

	// SBC A,B
	this->operations_[0x98] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_cpu()->registers.b);
	};

	// SBC A,C
	this->operations_[0x99] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_cpu()->registers.c);
	};

	// SBC A,D
	this->operations_[0x9A] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_cpu()->registers.d);
	};

	// SBC A,E
	this->operations_[0x9B] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_cpu()->registers.e);
	};

	// SBC A,H
	this->operations_[0x9C] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_cpu()->registers.h);
	};

	// SBC A,L
	this->operations_[0x9D] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_cpu()->registers.l);
	};

	// SBC A,(HL)
	this->operations_[0x9E] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl));
	};

	// SBC A,A
	this->operations_[0x9F] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_cpu()->registers.a);
	};

	// AND B
	this->operations_[0xA0] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_cpu()->registers.b);
	};

	// AND C
	this->operations_[0xA1] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_cpu()->registers.c);
	};

	// AND D
	this->operations_[0xA2] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_cpu()->registers.d);
	};

	// AND E
	this->operations_[0xA3] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_cpu()->registers.e);
	};

	// AND H
	this->operations_[0xA4] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_cpu()->registers.h);
	};

	// AND L
	this->operations_[0xA5] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_cpu()->registers.l);
	};

	// AND A,(HL)
	this->operations_[0xA6] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl));
	};

	// AND A
	this->operations_[0xA7] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_cpu()->registers.a);
	};

	// XOR B
	this->operations_[0xA8] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_cpu()->registers.b);
	};

	// XOR C
	this->operations_[0xA9] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_cpu()->registers.c);
	};

	// XOR D
	this->operations_[0xAA] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_cpu()->registers.d);
	};

	// XOR E
	this->operations_[0xAB] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_cpu()->registers.e);
	};

	// XOR H
	this->operations_[0xAC] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_cpu()->registers.h);
	};

	// XOR L
	this->operations_[0xAD] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_cpu()->registers.l);
	};

	// XOR (HL)
	this->operations_[0xAE] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl));
	};

	// XOR A
	this->operations_[0xAF] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_cpu()->registers.a);
	};

	// OR B
	this->operations_[0xB0] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_cpu()->registers.b);
	};

	// OR C
	this->operations_[0xB1] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_cpu()->registers.c);
	};

	// OR D
	this->operations_[0xB2] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_cpu()->registers.d);
	};

	// OR E
	this->operations_[0xB3] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_cpu()->registers.e);
	};

	// OR H
	this->operations_[0xB4] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_cpu()->registers.h);
	};

	// OR L
	this->operations_[0xB5] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_cpu()->registers.l);
	};

	// OR (HL)
	this->operations_[0xB6] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl));
	};

	// OR A
	this->operations_[0xB7] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_cpu()->registers.a);
	};

	// CP B
	this->operations_[0xB8] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_cpu()->registers.b);
	};

	// CP C
	this->operations_[0xB9] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_cpu()->registers.c);
	};

	// CP D
	this->operations_[0xBA] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_cpu()->registers.d);
	};

	// CP E
	this->operations_[0xBB] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_cpu()->registers.e);
	};

	// CP H
	this->operations_[0xBC] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_cpu()->registers.h);
	};

	// CP L
	this->operations_[0xBD] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_cpu()->registers.l);
	};

	// CP (HL)
	this->operations_[0xBE] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl));
	};

	// CP A
	this->operations_[0xBF] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_cpu()->registers.a);
	};

	// RET NZ
	this->operations_[0xC0] = [](game_boy* gb)
	{
		if (!(gb->get_cpu()->registers.f & flag_zero))
		{
			gb->get_cpu()->registers.pc = gb->get_cpu()->stack_pop_word();
			gb->get_cpu()->registers.m += 3;
		}
	};

	// POP BC
	this->operations_[0xC1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.bc = gb->get_cpu()->stack_pop_word();
	};

	// JP NZ,nn
	this->operations_[0xC2] = [](game_boy* gb)
	{
		const uint16_t jump_loc = gb->get_cpu()->read_program_word();
		if (!(gb->get_cpu()->registers.f & flag_zero))
		{
			gb->get_cpu()->registers.pc = jump_loc;
			gb->get_cpu()->registers.m++;
		}
	};

	// JP nn
	this->operations_[0xC3] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.pc = gb->get_cpu()->read_program_word();
	};

	// CALL NZ,nn
	this->operations_[0xC4] = [](game_boy* gb)
	{
		const uint16_t call_loc = gb->get_cpu()->read_program_word();
		if (!(gb->get_cpu()->registers.f & flag_zero))
		{
			gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.pc);
			gb->get_cpu()->registers.pc = call_loc;
			gb->get_cpu()->registers.m += 3;
		}
	};

	// PUSH BC
	this->operations_[0xC5] = [](game_boy* gb)
	{
		gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.bc);
	};

	// ADD A,n
	this->operations_[0xC6] = [](game_boy* gb)
	{
		gb->get_cpu()->add(gb->get_cpu()->read_program_byte());
	};

	// RST 0
	this->operations_[0xC7] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_rst(0x00);
	};

	// RET Z
	this->operations_[0xC8] = [](game_boy* gb)
	{
		if (gb->get_cpu()->registers.f & flag_zero)
		{
			gb->get_cpu()->registers.pc = gb->get_cpu()->stack_pop_word();
			gb->get_cpu()->registers.m += 3;
		}
	};

	// RET
	this->operations_[0xC9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.pc = gb->get_cpu()->stack_pop_word();
	};

	// JP Z,nn
	this->operations_[0xCA] = [](game_boy* gb)
	{
		const uint16_t jump_loc = gb->get_cpu()->read_program_word();
		if (gb->get_cpu()->registers.f & flag_zero)
		{
			gb->get_cpu()->registers.pc = jump_loc;
			gb->get_cpu()->registers.m++;
		}
	};

	// Ext ops
	this->operations_[0xCB] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_ext(gb->get_cpu()->read_program_byte());
		// TODO: Check if timing needs 4 extra ticks to the additional 8/16 ticks of the extop
	};

	// CALL Z,nn
	this->operations_[0xCC] = [](game_boy* gb)
	{
		const uint16_t call_loc = gb->get_cpu()->read_program_word();
		if (gb->get_cpu()->registers.f & flag_zero)
		{
			gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.pc);
			gb->get_cpu()->registers.pc = call_loc;
			gb->get_cpu()->registers.m += 3;
		}
	};

	// CALL nn
	this->operations_[0xCD] = [](game_boy* gb)
	{
		gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.pc + 2);
		gb->get_cpu()->registers.pc = gb->get_cpu()->read_program_word();
	};

	// ADC A,n
	this->operations_[0xCE] = [](game_boy* gb)
	{
		gb->get_cpu()->adc(gb->get_cpu()->read_program_byte());
	};

	// RST 8
	this->operations_[0xCF] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_rst(0x08);
	};

	// RET NC
	this->operations_[0xD0] = [](game_boy* gb)
	{
		if (!(gb->get_cpu()->registers.f & flag_carry))
		{
			gb->get_cpu()->registers.pc = gb->get_cpu()->stack_pop_word();
			gb->get_cpu()->registers.m += 3;
		}
	};

	// POP DE
	this->operations_[0xD1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.de = gb->get_cpu()->stack_pop_word();
	};

	// JP NC,nn
	this->operations_[0xD2] = [](game_boy* gb)
	{
		const uint16_t jump_loc = gb->get_cpu()->read_program_word();
		if (!(gb->get_cpu()->registers.f & flag_carry))
		{
			gb->get_cpu()->registers.pc = jump_loc;
			gb->get_cpu()->registers.m++;
		}
	};

	// CALL NC,nn
	this->operations_[0xD4] = [](game_boy* gb)
	{
		const uint16_t call_loc = gb->get_cpu()->read_program_word();
		if (!(gb->get_cpu()->registers.f & flag_carry))
		{
			gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.pc);
			gb->get_cpu()->registers.pc = call_loc;
			gb->get_cpu()->registers.m += 3;
		}
	};

	// PUSH DE
	this->operations_[0xD5] = [](game_boy* gb)
	{
		gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.de);
	};

	// SUB A,n
	this->operations_[0xD6] = [](game_boy* gb)
	{
		gb->get_cpu()->sub(gb->get_cpu()->read_program_byte());
	};

	// RST 10
	this->operations_[0xD7] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_rst(0x10);
	};

	// RET C
	this->operations_[0xD8] = [](game_boy* gb)
	{
		if (gb->get_cpu()->registers.f & flag_carry)
		{
			gb->get_cpu()->registers.pc = gb->get_cpu()->stack_pop_word();
			gb->get_cpu()->registers.m += 3;
		}
	};

	// RETI
	this->operations_[0xD9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_cpu()->sav_registers_.a;
		gb->get_cpu()->registers.b = gb->get_cpu()->sav_registers_.b;
		gb->get_cpu()->registers.c = gb->get_cpu()->sav_registers_.c;
		gb->get_cpu()->registers.d = gb->get_cpu()->sav_registers_.d;
		gb->get_cpu()->registers.e = gb->get_cpu()->sav_registers_.e;
		gb->get_cpu()->registers.f = gb->get_cpu()->sav_registers_.f;
		gb->get_cpu()->registers.h = gb->get_cpu()->sav_registers_.h;
		gb->get_cpu()->registers.l = gb->get_cpu()->sav_registers_.l;

		gb->get_cpu()->ime_ = true;
		gb->get_cpu()->registers.pc = gb->get_cpu()->stack_pop_word();
	};

	// JP C,nn
	this->operations_[0xDA] = [](game_boy* gb)
	{
		const uint16_t jump_loc = gb->get_cpu()->read_program_word();
		if (gb->get_cpu()->registers.f & flag_carry)
		{
			gb->get_cpu()->registers.pc = jump_loc;
			gb->get_cpu()->registers.m++;
		}
	};

	// CALL C,nn
	this->operations_[0xDC] = [](game_boy* gb)
	{
		const uint16_t call_loc = gb->get_cpu()->read_program_word();
		if (gb->get_cpu()->registers.f & flag_carry)
		{
			gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.pc);
			gb->get_cpu()->registers.pc = call_loc;
			gb->get_cpu()->registers.m += 3;
		}
	};

	// SBC A,n
	this->operations_[0xDE] = [](game_boy* gb)
	{
		gb->get_cpu()->sbc(gb->get_cpu()->read_program_byte());
	};

	// RST 18
	this->operations_[0xDF] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_rst(0x18);
	};

	// LDH (n),A
	this->operations_[0xE0] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(0xFF00 | gb->get_cpu()->read_program_byte(), gb->get_cpu()->registers.a);
	};

	// POP HL
	this->operations_[0xE1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.hl = gb->get_cpu()->stack_pop_word();
	};

	// LDH (C),A
	this->operations_[0xE2] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(0xFF00 | gb->get_cpu()->registers.c, gb->get_cpu()->registers.a);
	};

	// PUSH HL
	this->operations_[0xE5] = [](game_boy* gb)
	{
		gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.hl);
	};

	// AND n
	this->operations_[0xE6] = [](game_boy* gb)
	{
		gb->get_cpu()->_and(gb->get_cpu()->read_program_byte());
	};

	// RST 20
	this->operations_[0xE7] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_rst(0x20);
	};

	// ADD SP,d
	this->operations_[0xE8] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.f = 0;

		const uint16_t value = gb->get_cpu()->read_program_byte();
		const int32_t result = gb->get_cpu()->registers.sp + value;

		gb->get_cpu()->registers.f = 0;
		if (((gb->get_cpu()->registers.sp ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100)
			gb->get_cpu()->registers.f |=
				flag_carry;
		if (((gb->get_cpu()->registers.sp ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10)
			gb->get_cpu()->registers.f |=
				flag_half_carry;

		gb->get_cpu()->registers.sp = static_cast<uint16_t>(result & 0xFFFF);
	};

	// JP (HL)
	this->operations_[0xE9] = [](game_boy* gb)
	{
		//gb->getCPU()->registers.pc = gb->getMMU()->readByte(gb->getCPU()->registers.hl);
		gb->get_cpu()->registers.pc = gb->get_cpu()->registers.hl;
	};

	// LD (nn),A
	this->operations_[0xEA] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->read_program_word(), gb->get_cpu()->registers.a);
	};

	// XOR n
	this->operations_[0xEE] = [](game_boy* gb)
	{
		gb->get_cpu()->_xor(gb->get_cpu()->read_program_byte());
	};

	// RST 28
	this->operations_[0xEF] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_rst(0x28);
	};

	// LDH A,(n)
	this->operations_[0xF0] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_mmu()->read_byte(0xFF00 | gb->get_cpu()->read_program_byte());
	};

	// POP AF
	this->operations_[0xF1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.af = gb->get_cpu()->stack_pop_word();
		gb->get_cpu()->registers.f &= 0xF0;
	};

	// LDH A,(C)
	this->operations_[0xF2] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_mmu()->read_byte(0xFF00 | gb->get_cpu()->registers.c);
	};

	// DI
	this->operations_[0xF3] = [](game_boy* gb)
	{
		gb->get_cpu()->ime_ = false;
	};

	// PUSH AF
	this->operations_[0xF5] = [](game_boy* gb)
	{
		gb->get_cpu()->stack_push_word(gb->get_cpu()->registers.af);
	};

	// OR n
	this->operations_[0xF6] = [](game_boy* gb)
	{
		gb->get_cpu()->_or(gb->get_cpu()->read_program_byte());
	};

	// RST 30
	this->operations_[0xF7] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_rst(0x30);
	};

	// LD SP,HL
	this->operations_[0xF9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.sp = gb->get_cpu()->registers.hl;
	};

	// LD A,(nn)
	this->operations_[0xFA] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a = gb->get_mmu()->read_byte(gb->get_cpu()->read_program_word());
	};

	// EI
	this->operations_[0xFB] = [](game_boy* gb)
	{
		gb->get_cpu()->ime_ = true;
	};

	// CP n
	this->operations_[0xFE] = [](game_boy* gb)
	{
		gb->get_cpu()->cp(gb->get_cpu()->read_program_byte());
	};

	// RST 38
	this->operations_[0xFF] = [](game_boy* gb)
	{
		gb->get_cpu()->execute_rst(0x38);
	};
}

void cpu::setup_ext_operations()
{
	this->ext_operations_[0x00] = [](game_boy* gb)
	{
		gb->get_cpu()->rlc(&gb->get_cpu()->registers.b);
	};

	this->ext_operations_[0x01] = [](game_boy* gb)
	{
		gb->get_cpu()->rlc(&gb->get_cpu()->registers.c);
	};

	this->ext_operations_[0x02] = [](game_boy* gb)
	{
		gb->get_cpu()->rlc(&gb->get_cpu()->registers.d);
	};

	this->ext_operations_[0x03] = [](game_boy* gb)
	{
		gb->get_cpu()->rlc(&gb->get_cpu()->registers.e);
	};

	this->ext_operations_[0x04] = [](game_boy* gb)
	{
		gb->get_cpu()->rlc(&gb->get_cpu()->registers.h);
	};

	this->ext_operations_[0x05] = [](game_boy* gb)
	{
		gb->get_cpu()->rlc(&gb->get_cpu()->registers.l);
	};

	this->ext_operations_[0x06] = [](game_boy* gb)
	{
		uint8_t hl_val = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->rlc(&hl_val);
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, hl_val);
	};

	this->ext_operations_[0x07] = [](game_boy* gb)
	{
		gb->get_cpu()->rlc(&gb->get_cpu()->registers.a);
	};

	this->ext_operations_[0x08] = [](game_boy* gb)
	{
		gb->get_cpu()->rrc(&gb->get_cpu()->registers.b);
	};

	this->ext_operations_[0x09] = [](game_boy* gb)
	{
		gb->get_cpu()->rrc(&gb->get_cpu()->registers.c);
	};

	this->ext_operations_[0x0A] = [](game_boy* gb)
	{
		gb->get_cpu()->rrc(&gb->get_cpu()->registers.d);
	};

	this->ext_operations_[0x0B] = [](game_boy* gb)
	{
		gb->get_cpu()->rrc(&gb->get_cpu()->registers.e);
	};

	this->ext_operations_[0x0C] = [](game_boy* gb)
	{
		gb->get_cpu()->rrc(&gb->get_cpu()->registers.h);
	};

	this->ext_operations_[0x0D] = [](game_boy* gb)
	{
		gb->get_cpu()->rrc(&gb->get_cpu()->registers.l);
	};

	this->ext_operations_[0x0E] = [](game_boy* gb)
	{
		uint8_t hl_val = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->rrc(&hl_val);
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, hl_val);
	};

	this->ext_operations_[0x0F] = [](game_boy* gb)
	{
		gb->get_cpu()->rrc(&gb->get_cpu()->registers.a);
	};

	this->ext_operations_[0x10] = [](game_boy* gb)
	{
		gb->get_cpu()->rl(&gb->get_cpu()->registers.b);
	};

	this->ext_operations_[0x11] = [](game_boy* gb)
	{
		gb->get_cpu()->rl(&gb->get_cpu()->registers.c);
	};

	this->ext_operations_[0x12] = [](game_boy* gb)
	{
		gb->get_cpu()->rl(&gb->get_cpu()->registers.d);
	};

	this->ext_operations_[0x13] = [](game_boy* gb)
	{
		gb->get_cpu()->rl(&gb->get_cpu()->registers.e);
	};

	this->ext_operations_[0x14] = [](game_boy* gb)
	{
		gb->get_cpu()->rl(&gb->get_cpu()->registers.h);
	};

	this->ext_operations_[0x15] = [](game_boy* gb)
	{
		gb->get_cpu()->rl(&gb->get_cpu()->registers.l);
	};

	this->ext_operations_[0x16] = [](game_boy* gb)
	{
		uint8_t hl_val = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->rl(&hl_val);
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, hl_val);
	};

	this->ext_operations_[0x17] = [](game_boy* gb)
	{
		gb->get_cpu()->rl(&gb->get_cpu()->registers.a);
	};

	this->ext_operations_[0x18] = [](game_boy* gb)
	{
		gb->get_cpu()->rr(&gb->get_cpu()->registers.b);
	};

	this->ext_operations_[0x19] = [](game_boy* gb)
	{
		gb->get_cpu()->rr(&gb->get_cpu()->registers.c);
	};

	this->ext_operations_[0x1A] = [](game_boy* gb)
	{
		gb->get_cpu()->rr(&gb->get_cpu()->registers.d);
	};

	this->ext_operations_[0x1B] = [](game_boy* gb)
	{
		gb->get_cpu()->rr(&gb->get_cpu()->registers.e);
	};

	this->ext_operations_[0x1C] = [](game_boy* gb)
	{
		gb->get_cpu()->rr(&gb->get_cpu()->registers.h);
	};

	this->ext_operations_[0x1D] = [](game_boy* gb)
	{
		gb->get_cpu()->rr(&gb->get_cpu()->registers.l);
	};

	this->ext_operations_[0x1E] = [](game_boy* gb)
	{
		uint8_t hl_val = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->rr(&hl_val);
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, hl_val);
	};

	this->ext_operations_[0x1F] = [](game_boy* gb)
	{
		gb->get_cpu()->rr(&gb->get_cpu()->registers.a);
	};

	this->ext_operations_[0x20] = [](game_boy* gb)
	{
		gb->get_cpu()->sla(&gb->get_cpu()->registers.b);
	};

	this->ext_operations_[0x21] = [](game_boy* gb)
	{
		gb->get_cpu()->sla(&gb->get_cpu()->registers.c);
	};

	this->ext_operations_[0x22] = [](game_boy* gb)
	{
		gb->get_cpu()->sla(&gb->get_cpu()->registers.d);
	};

	this->ext_operations_[0x23] = [](game_boy* gb)
	{
		gb->get_cpu()->sla(&gb->get_cpu()->registers.e);
	};

	this->ext_operations_[0x24] = [](game_boy* gb)
	{
		gb->get_cpu()->sla(&gb->get_cpu()->registers.h);
	};

	this->ext_operations_[0x25] = [](game_boy* gb)
	{
		gb->get_cpu()->sla(&gb->get_cpu()->registers.l);
	};

	this->ext_operations_[0x26] = [](game_boy* gb)
	{
		uint8_t hl_val = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->sla(&hl_val);
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, hl_val);
	};

	this->ext_operations_[0x27] = [](game_boy* gb)
	{
		gb->get_cpu()->sla(&gb->get_cpu()->registers.a);
	};

	this->ext_operations_[0x28] = [](game_boy* gb)
	{
		gb->get_cpu()->sra(&gb->get_cpu()->registers.b);
	};

	this->ext_operations_[0x29] = [](game_boy* gb)
	{
		gb->get_cpu()->sra(&gb->get_cpu()->registers.c);
	};

	this->ext_operations_[0x2A] = [](game_boy* gb)
	{
		gb->get_cpu()->sra(&gb->get_cpu()->registers.d);
	};

	this->ext_operations_[0x2B] = [](game_boy* gb)
	{
		gb->get_cpu()->sra(&gb->get_cpu()->registers.e);
	};

	this->ext_operations_[0x2C] = [](game_boy* gb)
	{
		gb->get_cpu()->sra(&gb->get_cpu()->registers.h);
	};

	this->ext_operations_[0x2D] = [](game_boy* gb)
	{
		gb->get_cpu()->sra(&gb->get_cpu()->registers.l);
	};

	this->ext_operations_[0x2E] = [](game_boy* gb)
	{
		uint8_t hl_val = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->sra(&hl_val);
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, hl_val);
	};

	this->ext_operations_[0x2F] = [](game_boy* gb)
	{
		gb->get_cpu()->sra(&gb->get_cpu()->registers.a);
	};

	this->ext_operations_[0x30] = [](game_boy* gb)
	{
		gb->get_cpu()->swap(&gb->get_cpu()->registers.b);
	};

	this->ext_operations_[0x31] = [](game_boy* gb)
	{
		gb->get_cpu()->swap(&gb->get_cpu()->registers.c);
	};

	this->ext_operations_[0x32] = [](game_boy* gb)
	{
		gb->get_cpu()->swap(&gb->get_cpu()->registers.d);
	};

	this->ext_operations_[0x33] = [](game_boy* gb)
	{
		gb->get_cpu()->swap(&gb->get_cpu()->registers.e);
	};

	this->ext_operations_[0x34] = [](game_boy* gb)
	{
		gb->get_cpu()->swap(&gb->get_cpu()->registers.h);
	};

	this->ext_operations_[0x35] = [](game_boy* gb)
	{
		gb->get_cpu()->swap(&gb->get_cpu()->registers.l);
	};

	this->ext_operations_[0x36] = [](game_boy* gb)
	{
		uint8_t hl_val = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->swap(&hl_val);
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, hl_val);
	};

	this->ext_operations_[0x37] = [](game_boy* gb)
	{
		gb->get_cpu()->swap(&gb->get_cpu()->registers.a);
	};

	this->ext_operations_[0x38] = [](game_boy* gb)
	{
		gb->get_cpu()->srl(&gb->get_cpu()->registers.b);
	};

	this->ext_operations_[0x39] = [](game_boy* gb)
	{
		gb->get_cpu()->srl(&gb->get_cpu()->registers.c);
	};

	this->ext_operations_[0x3A] = [](game_boy* gb)
	{
		gb->get_cpu()->srl(&gb->get_cpu()->registers.d);
	};

	this->ext_operations_[0x3B] = [](game_boy* gb)
	{
		gb->get_cpu()->srl(&gb->get_cpu()->registers.e);
	};

	this->ext_operations_[0x3C] = [](game_boy* gb)
	{
		gb->get_cpu()->srl(&gb->get_cpu()->registers.h);
	};

	this->ext_operations_[0x3D] = [](game_boy* gb)
	{
		gb->get_cpu()->srl(&gb->get_cpu()->registers.l);
	};

	this->ext_operations_[0x3E] = [](game_boy* gb)
	{
		uint8_t hl_val = gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl);
		gb->get_cpu()->srl(&hl_val);
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl, hl_val);
	};

	this->ext_operations_[0x3F] = [](game_boy* gb)
	{
		gb->get_cpu()->srl(&gb->get_cpu()->registers.a);
	};

	this->ext_operations_[0x40] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.b, 0);
	};

	this->ext_operations_[0x41] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.c, 0);
	};

	this->ext_operations_[0x42] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.d, 0);
	};

	this->ext_operations_[0x43] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.e, 0);
	};

	this->ext_operations_[0x44] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.h, 0);
	};

	this->ext_operations_[0x45] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.l, 0);
	};

	this->ext_operations_[0x46] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl), 0);
	};

	this->ext_operations_[0x47] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.a, 0);
	};

	this->ext_operations_[0x48] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.b, 1);
	};

	this->ext_operations_[0x49] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.c, 1);
	};

	this->ext_operations_[0x4A] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.d, 1);
	};

	this->ext_operations_[0x4B] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.e, 1);
	};

	this->ext_operations_[0x4C] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.h, 1);
	};

	this->ext_operations_[0x4D] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.l, 1);
	};

	this->ext_operations_[0x4E] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl), 1);
	};

	this->ext_operations_[0x4F] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.a, 1);
	};

	this->ext_operations_[0x50] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.b, 2);
	};

	this->ext_operations_[0x51] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.c, 2);
	};

	this->ext_operations_[0x52] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.d, 2);
	};

	this->ext_operations_[0x53] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.e, 2);
	};

	this->ext_operations_[0x54] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.h, 2);
	};

	this->ext_operations_[0x55] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.l, 2);
	};

	this->ext_operations_[0x56] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl), 2);
	};

	this->ext_operations_[0x57] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.a, 2);
	};

	this->ext_operations_[0x58] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.b, 3);
	};

	this->ext_operations_[0x59] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.c, 3);
	};

	this->ext_operations_[0x5A] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.d, 3);
	};

	this->ext_operations_[0x5B] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.e, 3);
	};

	this->ext_operations_[0x5C] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.h, 3);
	};

	this->ext_operations_[0x5D] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.l, 3);
	};

	this->ext_operations_[0x5E] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl), 3);
	};

	this->ext_operations_[0x5F] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.a, 3);
	};

	this->ext_operations_[0x60] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.b, 4);
	};

	this->ext_operations_[0x61] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.c, 4);
	};

	this->ext_operations_[0x62] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.d, 4);
	};

	this->ext_operations_[0x63] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.e, 4);
	};

	this->ext_operations_[0x64] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.h, 4);
	};

	this->ext_operations_[0x65] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.l, 4);
	};

	this->ext_operations_[0x66] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl), 4);
	};

	this->ext_operations_[0x67] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.a, 4);
	};

	this->ext_operations_[0x68] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.b, 5);
	};

	this->ext_operations_[0x69] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.c, 5);
	};

	this->ext_operations_[0x6A] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.d, 5);
	};

	this->ext_operations_[0x6B] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.e, 5);
	};

	this->ext_operations_[0x6C] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.h, 5);
	};

	this->ext_operations_[0x6D] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.l, 5);
	};

	this->ext_operations_[0x6E] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl), 5);
	};

	this->ext_operations_[0x6F] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.a, 5);
	};

	this->ext_operations_[0x70] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.b, 6);
	};

	this->ext_operations_[0x71] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.c, 6);
	};

	this->ext_operations_[0x72] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.d, 6);
	};

	this->ext_operations_[0x73] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.e, 6);
	};

	this->ext_operations_[0x74] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.h, 6);
	};

	this->ext_operations_[0x75] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.l, 6);
	};

	this->ext_operations_[0x76] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl), 6);
	};

	this->ext_operations_[0x77] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.a, 6);
	};

	this->ext_operations_[0x78] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.b, 7);
	};

	this->ext_operations_[0x79] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.c, 7);
	};

	this->ext_operations_[0x7A] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.d, 7);
	};

	this->ext_operations_[0x7B] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.e, 7);
	};

	this->ext_operations_[0x7C] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.h, 7);
	};

	this->ext_operations_[0x7D] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.l, 7);
	};

	this->ext_operations_[0x7E] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl), 7);
	};

	this->ext_operations_[0x7F] = [](game_boy* gb)
	{
		gb->get_cpu()->bit(gb->get_cpu()->registers.a, 7);
	};

	this->ext_operations_[0x80] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b &= ~(1 << 0);
	};

	this->ext_operations_[0x81] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c &= ~(1 << 0);
	};

	this->ext_operations_[0x82] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d &= ~(1 << 0);
	};

	this->ext_operations_[0x83] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e &= ~(1 << 0);
	};

	this->ext_operations_[0x84] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h &= ~(1 << 0);
	};

	this->ext_operations_[0x85] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l &= ~(1 << 0);
	};

	this->ext_operations_[0x86] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) & ~(1 << 0));
	};

	this->ext_operations_[0x87] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a &= ~(1 << 0);
	};

	this->ext_operations_[0x88] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b &= ~(1 << 1);
	};

	this->ext_operations_[0x89] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c &= ~(1 << 1);
	};

	this->ext_operations_[0x8A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d &= ~(1 << 1);
	};

	this->ext_operations_[0x8B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e &= ~(1 << 1);
	};

	this->ext_operations_[0x8C] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h &= ~(1 << 1);
	};

	this->ext_operations_[0x8D] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l &= ~(1 << 1);
	};

	this->ext_operations_[0x8E] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) & ~(1 << 1));
	};

	this->ext_operations_[0x8F] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a &= ~(1 << 1);
	};

	this->ext_operations_[0x90] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b &= ~(1 << 2);
	};

	this->ext_operations_[0x91] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c &= ~(1 << 2);
	};

	this->ext_operations_[0x92] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d &= ~(1 << 2);
	};

	this->ext_operations_[0x93] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e &= ~(1 << 2);
	};

	this->ext_operations_[0x94] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h &= ~(1 << 2);
	};

	this->ext_operations_[0x95] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l &= ~(1 << 2);
	};

	this->ext_operations_[0x96] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) & ~(1 << 2));
	};

	this->ext_operations_[0x97] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a &= ~(1 << 2);
	};

	this->ext_operations_[0x98] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b &= ~(1 << 3);
	};

	this->ext_operations_[0x99] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c &= ~(1 << 3);
	};

	this->ext_operations_[0x9A] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d &= ~(1 << 3);
	};

	this->ext_operations_[0x9B] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e &= ~(1 << 3);
	};

	this->ext_operations_[0x9C] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h &= ~(1 << 3);
	};

	this->ext_operations_[0x9D] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l &= ~(1 << 3);
	};

	this->ext_operations_[0x9E] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) & ~(1 << 3));
	};

	this->ext_operations_[0x9F] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a &= ~(1 << 3);
	};

	this->ext_operations_[0xA0] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b &= ~(1 << 4);
	};

	this->ext_operations_[0xA1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c &= ~(1 << 4);
	};

	this->ext_operations_[0xA2] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d &= ~(1 << 4);
	};

	this->ext_operations_[0xA3] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e &= ~(1 << 4);
	};

	this->ext_operations_[0xA4] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h &= ~(1 << 4);
	};

	this->ext_operations_[0xA5] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l &= ~(1 << 4);
	};

	this->ext_operations_[0xA6] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) & ~(1 << 4));
	};

	this->ext_operations_[0xA7] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a &= ~(1 << 4);
	};

	this->ext_operations_[0xA8] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b &= ~(1 << 5);
	};

	this->ext_operations_[0xA9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c &= ~(1 << 5);
	};

	this->ext_operations_[0xAA] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d &= ~(1 << 5);
	};

	this->ext_operations_[0xAB] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e &= ~(1 << 5);
	};

	this->ext_operations_[0xAC] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h &= ~(1 << 5);
	};

	this->ext_operations_[0xAD] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l &= ~(1 << 5);
	};

	this->ext_operations_[0xAE] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) & ~(1 << 5));
	};

	this->ext_operations_[0xAF] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a &= ~(1 << 5);
	};

	this->ext_operations_[0xB0] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b &= ~(1 << 6);
	};

	this->ext_operations_[0xB1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c &= ~(1 << 6);
	};

	this->ext_operations_[0xB2] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d &= ~(1 << 6);
	};

	this->ext_operations_[0xB3] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e &= ~(1 << 6);
	};

	this->ext_operations_[0xB4] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h &= ~(1 << 6);
	};

	this->ext_operations_[0xB5] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l &= ~(1 << 6);
	};

	this->ext_operations_[0xB6] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) & ~(1 << 6));
	};

	this->ext_operations_[0xB7] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a &= ~(1 << 6);
	};

	this->ext_operations_[0xB8] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b &= ~(1 << 7);
	};

	this->ext_operations_[0xB9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c &= ~(1 << 7);
	};

	this->ext_operations_[0xBA] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d &= ~(1 << 7);
	};

	this->ext_operations_[0xBB] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e &= ~(1 << 7);
	};

	this->ext_operations_[0xBC] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h &= ~(1 << 7);
	};

	this->ext_operations_[0xBD] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l &= ~(1 << 7);
	};

	this->ext_operations_[0xBE] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) & ~(1 << 7));
	};

	this->ext_operations_[0xBF] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a &= ~(1 << 7);
	};

	this->ext_operations_[0xC0] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b |= 1 << 0;
	};

	this->ext_operations_[0xC1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c |= 1 << 0;
	};

	this->ext_operations_[0xC2] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d |= 1 << 0;
	};

	this->ext_operations_[0xC3] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e |= 1 << 0;
	};

	this->ext_operations_[0xC4] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h |= 1 << 0;
	};

	this->ext_operations_[0xC5] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l |= 1 << 0;
	};

	this->ext_operations_[0xC6] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) | (1 << 0));
	};

	this->ext_operations_[0xC7] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a |= 1 << 0;
	};

	this->ext_operations_[0xC8] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b |= 1 << 1;
	};

	this->ext_operations_[0xC9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c |= 1 << 1;
	};

	this->ext_operations_[0xCA] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d |= 1 << 1;
	};

	this->ext_operations_[0xCB] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e |= 1 << 1;
	};

	this->ext_operations_[0xCC] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h |= 1 << 1;
	};

	this->ext_operations_[0xCD] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l |= 1 << 1;
	};

	this->ext_operations_[0xCE] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) | (1 << 1));
	};

	this->ext_operations_[0xCF] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a |= 1 << 1;
	};

	this->ext_operations_[0xD0] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b |= 1 << 2;
	};

	this->ext_operations_[0xD1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c |= 1 << 2;
	};

	this->ext_operations_[0xD2] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d |= 1 << 2;
	};

	this->ext_operations_[0xD3] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e |= 1 << 2;
	};

	this->ext_operations_[0xD4] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h |= 1 << 2;
	};

	this->ext_operations_[0xD5] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l |= 1 << 2;
	};

	this->ext_operations_[0xD6] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) | (1 << 2));
	};

	this->ext_operations_[0xD7] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a |= 1 << 2;
	};

	this->ext_operations_[0xD8] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b |= 1 << 3;
	};

	this->ext_operations_[0xD9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c |= 1 << 3;
	};

	this->ext_operations_[0xDA] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d |= 1 << 3;
	};

	this->ext_operations_[0xDB] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e |= 1 << 3;
	};

	this->ext_operations_[0xDC] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h |= 1 << 3;
	};

	this->ext_operations_[0xDD] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l |= 1 << 3;
	};

	this->ext_operations_[0xDE] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) | (1 << 3));
	};

	this->ext_operations_[0xDF] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a |= 1 << 3;
	};

	this->ext_operations_[0xE0] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b |= 1 << 4;
	};

	this->ext_operations_[0xE1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c |= 1 << 4;
	};

	this->ext_operations_[0xE2] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d |= 1 << 4;
	};

	this->ext_operations_[0xE3] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e |= 1 << 4;
	};

	this->ext_operations_[0xE4] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h |= 1 << 4;
	};

	this->ext_operations_[0xE5] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l |= 1 << 4;
	};

	this->ext_operations_[0xE6] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) | (1 << 4));
	};

	this->ext_operations_[0xE7] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a |= 1 << 4;
	};

	this->ext_operations_[0xE8] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b |= 1 << 5;
	};

	this->ext_operations_[0xE9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c |= 1 << 5;
	};

	this->ext_operations_[0xEA] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d |= 1 << 5;
	};

	this->ext_operations_[0xEB] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e |= 1 << 5;
	};

	this->ext_operations_[0xEC] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h |= 1 << 5;
	};

	this->ext_operations_[0xED] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l |= 1 << 5;
	};

	this->ext_operations_[0xEE] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) | (1 << 5));
	};

	this->ext_operations_[0xEF] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a |= 1 << 5;
	};

	this->ext_operations_[0xF0] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b |= 1 << 6;
	};

	this->ext_operations_[0xF1] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c |= 1 << 6;
	};

	this->ext_operations_[0xF2] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d |= 1 << 6;
	};

	this->ext_operations_[0xF3] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e |= 1 << 6;
	};

	this->ext_operations_[0xF4] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h |= 1 << 6;
	};

	this->ext_operations_[0xF5] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l |= 1 << 6;
	};

	this->ext_operations_[0xF6] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) | (1 << 6));
	};

	this->ext_operations_[0xF7] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a |= 1 << 6;
	};

	this->ext_operations_[0xF8] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.b |= 1 << 7;
	};

	this->ext_operations_[0xF9] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.c |= 1 << 7;
	};

	this->ext_operations_[0xFA] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.d |= 1 << 7;
	};

	this->ext_operations_[0xFB] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.e |= 1 << 7;
	};

	this->ext_operations_[0xFC] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.h |= 1 << 7;
	};

	this->ext_operations_[0xFD] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.l |= 1 << 7;
	};

	this->ext_operations_[0xFE] = [](game_boy* gb)
	{
		gb->get_mmu()->write_byte(gb->get_cpu()->registers.hl,
		                          gb->get_mmu()->read_byte(gb->get_cpu()->registers.hl) | (1 << 7));
	};

	this->ext_operations_[0xFF] = [](game_boy* gb)
	{
		gb->get_cpu()->registers.a |= 1 << 7;
	};
}

void cpu::add_hl(const uint16_t value)
{
	const int32_t result = this->registers.hl + value;

	this->registers.f &= flag_zero;
	if (result & 0x10000) this->registers.f |= flag_carry;
	if ((this->registers.hl ^ value ^ (result & 0xFFFF)) & 0x1000) this->registers.f |= flag_half_carry;

	this->registers.hl = static_cast<uint16_t>(result & 0xFFFF);
}

void cpu::inc(uint8_t* reg)
{
	(*reg)++;
	this->registers.f &= flag_carry;
	if ((*reg & 0x0F) == 0x00) this->registers.f |= flag_half_carry;
	if (!*reg) this->registers.f |= flag_zero;
}

void cpu::dec(uint8_t* reg)
{
	(*reg)--;
	this->registers.f &= flag_carry;
	if ((*reg & 0x0F) == 0x0F) this->registers.f |= flag_half_carry;
	if (!*reg) this->registers.f |= flag_zero;
	this->registers.f |= flag_negative;
}

void cpu::add(const uint8_t reg)
{
	const int8_t value = reg;
	const uint32_t result = this->registers.a + value;
	const int32_t carrybits = this->registers.a ^ value ^ result;

	this->registers.a = static_cast<uint8_t>(result & 0xFF);

	this->registers.f = 0;
	if ((carrybits & 0x100) != 0) this->registers.f |= flag_carry;
	if (!this->registers.a) this->registers.f |= flag_zero;
	if ((carrybits & 0x10) != 0) this->registers.f |= flag_half_carry;
}

void cpu::adc(const uint8_t reg)
{
	const int32_t carry = (this->registers.f & flag_carry) ? 1 : 0;

	const int32_t value = reg + carry;
	const int32_t result = this->registers.a + value;

	this->registers.f = 0;
	if (result > 0xFF) this->registers.f |= flag_carry;
	if (!result) this->registers.f |= flag_zero;
	if (((this->registers.a & 0x0F) + (reg & 0x0F) + carry) > 0x0F) this->registers.f |= flag_half_carry;

	this->registers.a = static_cast<uint8_t>(result & 0xFF);
}

void cpu::sbc(const uint8_t reg)
{
	const int32_t carry = (this->registers.f & flag_carry) ? 1 : 0;
	const int32_t result = this->registers.a - reg - carry;

	this->registers.f = flag_negative;
	if (!(result & 0xFF)) this->registers.f |= flag_zero;
	if (result < 0) this->registers.f |= flag_carry;
	if (((this->registers.a & 0x0F) - (reg & 0x0F) - carry) < 0) this->registers.f |= flag_half_carry;

	this->registers.a = static_cast<uint8_t>(result);
}

void cpu::sub(const uint8_t reg)
{
	const int8_t value = reg;
	const uint32_t result = this->registers.a - value;
	const int32_t carrybits = this->registers.a ^ value ^ result;

	this->registers.f = flag_negative;
	if ((carrybits & 0x100) != 0) this->registers.f |= flag_carry;
	if ((carrybits & 0x10) != 0) this->registers.f |= flag_half_carry;
	if (!result) this->registers.f |= flag_zero;

	this->registers.a = static_cast<uint8_t>(result);
}

void cpu::_and(const uint8_t reg)
{
	this->registers.f = flag_half_carry;

	this->registers.a &= reg;
	if (!this->registers.a) this->registers.f |= flag_zero;
}

void cpu::_xor(const uint8_t reg)
{
	this->registers.a ^= reg;
	this->registers.f = this->registers.a ? 0 : flag_zero;
}

void cpu::_or(const uint8_t reg)
{
	this->registers.a |= reg;
	this->registers.f = this->registers.a ? 0 : flag_zero;
}

void cpu::cp(const uint8_t reg)
{
	this->registers.f = flag_negative;
	if (this->registers.a < reg) this->registers.f |= flag_carry;
	if (this->registers.a == reg) this->registers.f |= flag_zero;
	if (((this->registers.a - reg) & 0xF) > (this->registers.a & 0xF)) this->registers.f |= flag_half_carry;
}

auto cpu::bit(const uint8_t reg, const uint8_t _bit) -> void
{
	this->registers.f &= ~(flag_zero | flag_negative);
	this->registers.f |= flag_half_carry;
	if (((reg >> _bit) & 0x01) == 0) this->registers.f |= flag_zero;
}

void cpu::rlc(uint8_t* reg)
{
	const bool carry = (*reg & 0x80) == 0x80;
	*reg <<= 1;
	*reg |= carry ? 0x01 : 0;

	this->registers.f = carry ? flag_carry : 0;
	if (!*reg && reg != &this->registers.a) this->registers.f |= flag_zero;
}

void cpu::rrc(uint8_t* reg)
{
	const bool carry = *reg & 0x01;
	*reg >>= 1;
	*reg |= carry ? 0x80 : 0;

	this->registers.f = carry ? flag_carry : 0;
	if (!*reg && reg != &this->registers.a) this->registers.f |= flag_zero;
}

void cpu::rl(uint8_t* reg)
{
	const uint8_t carry = (this->registers.f & flag_carry) ? 1 : 0;
	this->registers.f = (*reg & 0x80) ? flag_carry : 0;

	*reg <<= 1;
	*reg |= carry;

	if (!*reg && reg != &this->registers.a) this->registers.f |= flag_zero;
}

void cpu::rr(uint8_t* reg)
{
	const uint8_t carry = (this->registers.f & flag_carry) ? 0x80 : 0;
	this->registers.f = (*reg & 0x01) ? flag_carry : 0;

	*reg >>= 1;
	*reg |= carry;

	if (!*reg && reg != &this->registers.a) this->registers.f |= flag_zero;
}

void cpu::sla(uint8_t* reg)
{
	this->registers.f = (*reg & 0x80) ? flag_carry : 0;
	*reg <<= 1;
	if (!*reg) this->registers.f |= flag_zero;
}

void cpu::sra(uint8_t* reg)
{
	const uint8_t carry = (*reg & 0x80);
	this->registers.f = (*reg & 0x01) ? flag_carry : 0;
	*reg >>= 1;
	*reg |= carry;
	if (!*reg) this->registers.f |= flag_zero;
}

void cpu::swap(uint8_t* reg)
{
	*reg = ((*reg & 0x0F) << 4) | ((*reg >> 4) & 0x0F);
	this->registers.f = !*reg ? flag_zero : 0;
}

void cpu::srl(uint8_t* reg)
{
	this->registers.f = (*reg & 0x01) ? flag_carry : 0;
	*reg >>= 1;
	if (!*reg) this->registers.f |= flag_zero;
}

void cpu::execute_rst(const uint16_t num)
{
	std::memcpy(&this->sav_registers_, &this->registers, sizeof this->registers);
	this->stack_push_word(this->registers.pc);
	this->registers.pc = num;
	this->registers.m += 4;
}

uint8_t cpu::read_program_byte()
{
	const uint16_t addr = this->registers.pc++;
	printf("PRogByte: %hu\n", addr);
	this->registers.pc &= 0xFFFF;
	return this->gb_->get_mmu()->read_byte(addr);
}

uint16_t cpu::read_program_word()
{
	const uint16_t low = this->read_program_byte();
	const uint16_t high = this->read_program_byte();
	return low | (high << 8);
}

void cpu::stack_push_word(const uint16_t value)
{
	this->registers.sp -= 2;
	this->gb_->get_mmu()->write_word(this->registers.sp, value);
}

void cpu::stack_push_byte(const uint8_t value)
{
	this->registers.sp -= 1;
	this->gb_->get_mmu()->write_byte(this->registers.sp, value);
}

uint16_t cpu::stack_pop_word()
{
	const uint16_t value = this->gb_->get_mmu()->read_word(this->registers.sp);
	this->registers.sp += 2;
	return value;
}

uint8_t cpu::stack_pop_byte()
{
	const uint8_t value = this->gb_->get_mmu()->read_byte(this->registers.sp);
	this->registers.sp += 1;
	return value;
}

bool cpu::execute()
{
	const uint16_t pc = this->registers.pc;
	const uint8_t instruction = this->halted_ ? 0 : this->read_program_byte();
	const auto operation = this->operations_[instruction];

	if (operation)
	{
		try
		{
			if (!this->halted_)
			{
#ifdef DEBUG_OPS
				printf("Operation %X (%X) execution |\taf= %04X\tbc= %04X\tde= %04X\thl= %04X\tsp= %04X\tpc= %04X\n", instruction, pc, this->registers.af, this->registers.bc, this->registers.de, this->registers.hl, this->registers.sp, pc);
#endif

				operation(this->gb_);
			}

			this->registers.m += (cpu::operation_ticks[instruction] / 2);
			this->timer.increment(this->gb_);

			if (this->ime_ && this->gb_->get_mmu()->i_e && this->gb_->get_mmu()->i_f)
			{
				this->halted_ = false;
				this->ime_ = false;

				const uint8_t ifired = this->gb_->get_mmu()->i_e & this->gb_->get_mmu()->i_f;
				if (ifired & 1) // VBlank
				{
					this->gb_->get_mmu()->i_f &= 0xFE;
					this->execute_rst(0x40);
				}
				else if (ifired & 2) // LCDStat
				{
					this->gb_->get_mmu()->i_f &= 0xFD;
					this->execute_rst(0x48);
				}
				else if (ifired & 4) // Timer
				{
					this->gb_->get_mmu()->i_f &= 0xFB;
					this->execute_rst(0x50);
				}
				else if (ifired & 8) // Serial
				{
					this->gb_->get_mmu()->i_f &= 0xF7;
					this->execute_rst(0x58);
				}
				else if (ifired & 16) // Joypad
				{
					this->gb_->get_mmu()->i_f &= 0xEF;
					this->execute_rst(0x60);
				}
				else
				{
					this->ime_ = true;
				}
			}

			this->timer.increment(this->gb_);

			if (!this->gb_->get_display()->is_on()) return false;
			this->gb_->get_gpu()->frame();

			if (this->registers.pc == 0x100) this->gb_->get_mmu()->mark_bios_pass();
			return true;
		}
		catch (std::exception& e)
		{
			printf("Operation %X (%X) has thrown an exception: %s\n", instruction, pc, e.what());
		}
		catch (...)
		{
			printf("Operation %X (%X) has thrown an unknown exception\n", instruction, pc);
		}
	}
	else
	{
		printf("Unsupported instruction %X (%X)\n", instruction, pc);
	}

	return false;
}

void cpu::execute_ext(const uint8_t instruction)
{
	const auto ext_op = this->ext_operations_[instruction];

	if (ext_op)
	{
		try
		{
			ext_op(this->gb_);
			this->registers.m += (cpu::ext_operation_ticks[instruction] / 4);

#ifdef DEBUG_OPS
			printf("Extended operation %X (%X) executed\n", instruction, this->registers.pc);
#endif
			return;
		}
		catch (std::exception& e)
		{
			printf("Extended operation %X (%X) has thrown an exception: %s\n", instruction, this->registers.pc,
			       e.what());
		}
		catch (...)
		{
			printf("Extended operation %X (%X) has thrown an exception\n", instruction, this->registers.pc);
		}
	}
	else
	{
		printf("Unsupported extended operation %X (%X)\n", instruction, this->registers.pc);
	}

	throw std::exception();
}

void cpu::skip_bios()
{
	this->registers.af = 0x01B0;
	this->registers.bc = 0x0013;
	this->registers.de = 0x00D8;
	this->registers.hl = 0x014D;
	this->registers.sp = 0xFFFE;
	this->registers.pc = 0x100;

	this->gb_->get_mmu()->mark_bios_pass();

	this->gb_->get_mmu()->write_byte(0xFF05, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF06, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF07, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF10, 0x80);
	this->gb_->get_mmu()->write_byte(0xFF11, 0xBF);
	this->gb_->get_mmu()->write_byte(0xFF12, 0xF3);
	this->gb_->get_mmu()->write_byte(0xFF14, 0xBF);
	this->gb_->get_mmu()->write_byte(0xFF16, 0x3F);
	this->gb_->get_mmu()->write_byte(0xFF17, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF19, 0xBF);
	this->gb_->get_mmu()->write_byte(0xFF1A, 0x7F);
	this->gb_->get_mmu()->write_byte(0xFF1B, 0xFF);
	this->gb_->get_mmu()->write_byte(0xFF1C, 0x9F);
	this->gb_->get_mmu()->write_byte(0xFF1E, 0xBF);
	this->gb_->get_mmu()->write_byte(0xFF20, 0xFF);
	this->gb_->get_mmu()->write_byte(0xFF21, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF22, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF23, 0xBF);
	this->gb_->get_mmu()->write_byte(0xFF24, 0x77);
	this->gb_->get_mmu()->write_byte(0xFF25, 0xF3);
	this->gb_->get_mmu()->write_byte(0xFF26, 0xF1);
	this->gb_->get_mmu()->write_byte(0xFF40, 0x91);
	this->gb_->get_mmu()->write_byte(0xFF42, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF43, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF45, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF47, 0xFC);
	this->gb_->get_mmu()->write_byte(0xFF48, 0xFF);
	this->gb_->get_mmu()->write_byte(0xFF49, 0xFF);
	this->gb_->get_mmu()->write_byte(0xFF4A, 0x00);
	this->gb_->get_mmu()->write_byte(0xFF4B, 0x00);
	this->gb_->get_mmu()->write_byte(0xFFFF, 0x00);
}
