#include "CPU.h"
#include "BUS.h"

//#define TIMER

void CPU::CpuWrite(uint16_t addr, uint8_t data)
{
	bus->CpuWrite(addr, data);
}

uint8_t CPU::CpuRead(uint16_t addr)
{
	uint8_t data = 0x00;
	data = bus->CpuRead(addr);
	return data;
}

void CPU::start()
{
	pc = 0x0000;
	sp = 0x0000;

	f = 0x00;
	af = 0x0000;
	bc = 0x0000;
	de = 0x0000;
	hl = 0x0000;
}

void CPU::clock()
{
	if (cycles == 0)
	{
		if (!Prefix_CB)
			cycles = Interrupt() >> 2;

		if (cycles == 0)
		{
			opcode = CpuRead(pc);

			pc++;
			if (bHaltBug)
			{
				bHaltBug = false;
				pc--;
			}

			cycles = DoInstruction(opcode) >> 2;

			//std::cout << str_opcode << " pc:" << std::uppercase << std::hex << pc << std::endl;
		}

		InstructionDone = true;
	}
	cycles--;
}

void CPU::LoadA(uint8_t input)
{
	af &= 0x00FF;
	af |= (uint16_t)input << 8;
}
void CPU::LoadF(uint8_t input)
{
	f = input;
	af &= 0xFF00;
	af |= (uint16_t)input;
}

void CPU::LoadB(uint8_t input)
{
	bc &= 0x00FF;
	bc |= (uint16_t)input << 8;
}

void CPU::LoadC(uint8_t input)
{
	bc &= 0xFF00;
	bc |= (uint16_t)input;
}

void CPU::LoadD(uint8_t input)
{
	de &= 0x00FF;
	de |= (uint16_t)input << 8;
}

void CPU::LoadE(uint8_t input)
{
	de &= 0xFF00;
	de |= (uint16_t)input;
}

void CPU::LoadH(uint8_t input)
{
	hl &= 0x00FF;
	hl |= (uint16_t)input << 8;
}

void CPU::LoadL(uint8_t input)
{
	hl &= 0xFF00;
	hl |= (uint16_t)input;
}

uint8_t CPU::GetA() const
{
	return af >> 8;
}

uint8_t CPU::GetB() const
{
	return bc >> 8;
}

uint8_t CPU::GetC() const
{
	return (uint8_t)(bc & 0x00FF);
}

uint8_t CPU::GetD() const
{
	return de >> 8;
}

uint8_t CPU::GetE() const
{
	return (uint8_t)(de & 0x00FF);
}

uint8_t CPU::GetH() const
{
	return hl >> 8;
}

uint8_t CPU::GetL() const
{
	return (uint8_t)(hl & 0x00FF);
}

// Flags Helper Functions 

void CPU::SetFlag(Flags F, bool condition)
{
	if (condition)
		f |= F;
	else
		f &= ~F;

	af &= 0xFF00;
	af |= (uint16_t)f;
}

uint8_t CPU::GetFlag(Flags F) const
{
	if (f & F)
		return 1;
	else
		return 0;
}

// Addresing modes 

uint8_t CPU::d8()
{
	uint8_t data = CpuRead(pc);
	pc++;
	return data;
}

uint16_t CPU::d16()
{
	uint8_t data_lo = CpuRead(pc);
	pc++;
	uint8_t data_hi = CpuRead(pc);
	pc++;
	return ((uint16_t)data_hi << 8) | data_lo;
}

uint8_t CPU::a8()
{
	uint8_t data = CpuRead(pc);
	pc++;
	return data;
}

uint16_t CPU::a16()
{
	uint8_t addr_lo = CpuRead(pc);
	pc++;
	uint8_t addr_hi = CpuRead(pc);
	pc++;

	return ((uint16_t)addr_hi << 8) | addr_lo;
}

int8_t CPU::r8()
{
	int8_t sdata = CpuRead(pc);
	pc++;
	return sdata;
}

uint8_t CPU::Interrupt() // I am not sure at all.
{
	if (bIME)
	{
		if (bus->IE.V_Blank && bus->IF.V_Blank)
		{
			bIME = false;

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0040;

			bus->IF.V_Blank = 0;

			//printf("Interrupt V.blank\n");
			return 5; // I am totally unsure :p.
		}
		else if (bus->IE.LCDC && bus->IF.LCDC)
		{
			bIME = false;

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0048;

			bus->IF.LCDC = 0;

			//printf("Interrupt LCDC\n");
			return 5; // I am totally unsure :p.
		}

		else if (bus->IE.Timer_Overflow && bus->IF.Timer_Overflow)
		{
			bIME = false;

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0050;

			bus->IF.Timer_Overflow = 0;

			//printf("Interrupt Timer_Overflow\n");
			return 5; // I am totally unsure :p.
		}
		// TIMER

		else if (bus->IE.Serial_I_O && bus->IF.Serial_I_O)
		{
			bIME = false;

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0058;

			bus->IF.Serial_I_O = 0;

			//	printf("Serial I/O\n");
			return 5; // I am totally unsure :p.
		}
		else if (bus->IE.P10_P13 && bus->IF.P10_P13)
		{
			bIME = false;

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0060;

			bus->IF.P10_P13 = 0;

			printf("P10_P13\n");
			return 5; // I am totally unsure :p.
		}

	}

	return 0;
}

// Instructions

uint8_t CPU::DoInstruction(const uint16_t& opcode)
{
	if (!Prefix_CB)
	{
		switch (opcode)
		{
		case 0x00: // NOP

			str_opcode = "NOP";

			return 4;

		case 0x01: // LD BC,d16

			str_opcode = "LD BC,d16";

			bc = d16();

			return 12;

		case 0x02: // LD (BC),A

			str_opcode = "LD (BC),A";

			CpuWrite(bc, GetA());

			return 8;

		case 0x03: // INC BC

			str_opcode = "INC BC";

			bc++;

			return 8;

		case 0x04: // INC B

			str_opcode = "INC B";

			SetFlag(Z, GetB() == 0xFF);
			SetFlag(N, 0);
			SetFlag(H, (GetB() & 0x0F) == 0x0F); //???????

			LoadB((uint8_t)(GetB() + 1));

			return 4;

		case 0x05: // DEC B

			str_opcode = "DEC B";

			SetFlag(Z, GetB() == 0x01);
			SetFlag(N, 1);
			SetFlag(H, (GetB() & 0x0F) == 0x00); // not implemented

			LoadB(GetB() - 1);

			return 4;

		case 0x06: // LD B,d8

			str_opcode = "LD B,d8";

			LoadB(d8());

			return 8;

		case 0x07: // RLCA

			str_opcode = "RLCA";

			SetFlag(Z, 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetA() >> 7);

			LoadA((GetA() << 1) | GetFlag(C));

			return 4;

		case 0x08: // LD (a16),SP

			str_opcode = "LD (a16),SP";

			addr = a16();

			CpuWrite(addr, (sp << 8) >> 8);
			CpuWrite(addr + 1, sp >> 8);

			return 20;

		case 0x09: // ADD HL,BC

			str_opcode = "ADD HL,BC";

			temp32 = hl + bc;

			SetFlag(N, 0);
			SetFlag(H, ((hl & 0x0FFF) + (bc & 0x0FFF)) >= 0x1000);
			SetFlag(C, temp32 & 0x00010000);

			hl = (uint16_t)(temp32);

			return 8;

		case 0x0A: // LD A,(BC)

			str_opcode = "LD A,(BC)";

			LoadA(CpuRead(bc));

			return 8;

		case 0x0B: // DEC BC

			str_opcode = "DEC BC";

			bc--;

			return 8;

		case 0x0C: // INC C

			str_opcode = "INC C";

			SetFlag(Z, GetC() == 0xFF);
			SetFlag(N, 0);
			SetFlag(H, (GetC() & 0x0F) == 0x0F);

			LoadC((uint8_t)(GetC() + 1));

			return 4;

		case 0x0D: // DEC C

			str_opcode = "DEC C";

			SetFlag(Z, GetC() == 0x01);
			SetFlag(N, 1);
			SetFlag(H, (GetC() & 0x0F) == 0x00);

			LoadC((uint8_t)(GetC() - 1));

			return 4;

		case 0x0E: // LD C,d8

			str_opcode = "LD C,d8";

			LoadC(d8());

			return 8;

		case 0x0F: // RRCA

			str_opcode = "RRCA";

			SetFlag(Z, 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetA() & 0x01);

			LoadA((GetA() >> 1) | (GetFlag(C) << 7));

			return 4;

		case 0x10: // STOP

			str_opcode = "STOP";

			// Not fully Implemented
			printf("stop");

			bStop = true;

			return 4;

		case 0x11: // LD DE,d16

			str_opcode = "LD DE,d16";

			de = d16();

			return 12;

		case 0x12: // LD (DE),A

			str_opcode = "LD (DE),A";

			CpuWrite(de, GetA());

			return 8;

		case 0x13: // INC DE

			str_opcode = "INC DE";

			de++;

			return 8;

		case 0x14: // INC D

			str_opcode = "INC D";

			SetFlag(Z, GetD() == 0xFF);
			SetFlag(N, 0);
			SetFlag(H, (GetD() & 0x0F) == 0x0F); //???????

			LoadD((uint8_t)(GetD() + 1));

			return 4;

		case 0x15: // DEC D

			str_opcode = "DEC D";

			SetFlag(Z, GetD() == 0x01);
			SetFlag(N, 1);
			SetFlag(H, (GetD() & 0x0F) == 0x00);

			LoadD((uint8_t)(GetD() - 1));

			return 4;

		case 0x16: // LD D,d8

			str_opcode = "LD D,d8";

			LoadD(d8());

			return 8;

		case 0x17: // RLA

			str_opcode = "RLA";

			c = (GetA() & 0x80) >> 7;
			temp = (GetA() << 1) | GetFlag(C);

			LoadA(temp);

			SetFlag(Z, 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			return 4;

		case 0x18: // JR r8

			str_opcode = "JR r8";

			pc += (int8_t)r8();

			return 12;

		case 0x19: // ADD HL,DE

			str_opcode = "ADD HL,DE";

			temp32 = hl + de;

			SetFlag(N, 0);
			SetFlag(H, ((hl & 0x0FFF) + (de & 0x0FFF)) >= 0x1000);
			SetFlag(C, temp32 & 0x00010000);

			hl = (uint16_t)(temp32);

			return 8;

		case 0x1A: // LD A,(DE)

			str_opcode = "LD A,(DE)";

			LoadA(CpuRead(de));

			return 8;

		case 0x1B: // DEC DE

			str_opcode = "DEC DE";

			de--;

			return 8;

		case 0x1C: // INC E

			str_opcode = "INC E";

			SetFlag(Z, GetE() == 0xFF);
			SetFlag(N, 0);
			SetFlag(H, (GetE() & 0x0F) == 0x0F);

			LoadE((uint8_t)(GetE() + 1));

			return 4;

		case 0x1D: // DEC E

			str_opcode = "DEC E";

			SetFlag(Z, GetE() == 0x01);
			SetFlag(N, 1);
			SetFlag(H, (GetE() & 0x0F) == 0x00);

			LoadE((uint8_t)(GetE() - 1));

			return 4;

		case 0x1E: // LD E,d8

			str_opcode = "LD E,d8";

			LoadE(d8());

			return 8;

		case 0x1F: // RRA

			str_opcode = "RRA";

			c = GetA() << 7;

			LoadA((GetA() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			return 4;

		case 0x20: // JR NZ,r8

			str_opcode = "JR NZ,r8";

			temp = r8();
			if (!GetFlag(Z))
			{
				pc += (int8_t)temp;
				return 12;
			}

			return 8;

		case 0x21: // LD HL,d16

			str_opcode = "LD HL,d16";

			hl = d16();

			return 12;

		case 0x22: // LD (HL+),A

			str_opcode = "LD (HL+),A";

			CpuWrite(hl, GetA());
			hl++;

			return 8;

		case 0x23: // INC HL

			str_opcode = "INC HL";

			hl++;

			return 8;

		case 0x24: // INC H

			str_opcode = "INC H";

			SetFlag(Z, GetH() == 0xFF);
			SetFlag(N, 0);
			SetFlag(H, (GetH() & 0x0F) == 0x0F);

			LoadH((uint8_t)(GetH() + 1));

			return 4;

		case 0x25: // DEC H

			str_opcode = "DEC H";

			SetFlag(Z, GetH() == 0x01);
			SetFlag(N, 1);
			SetFlag(H, (GetH() & 0x0F) == 0x00);

			LoadH((uint8_t)(GetH() - 1));

			return 4;

		case 0x26: // LD H,d8

			str_opcode = "LD H,d8";

			LoadH(d8());

			return 8;

		case 0x27: // DAA // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{
			str_opcode = "DAA";

			if (!GetFlag(N) && (GetA() >= 0x9A))
				SetFlag(C, 1);
			if (!GetFlag(N) && ((GetA() & 0x0F) >= 0x0A))
				SetFlag(H, 1);

			uint8_t adjustment = (GetFlag(H) ? 6 : 0) | (GetFlag(C) ? 0x60 : 0);

			LoadA(GetA() + (GetFlag(N) ? -adjustment : adjustment));

			SetFlag(Z, GetA() == 0x00);
			SetFlag(H, 0);


			return 4;

			/*

			temp = GetA();


			if (GetFlag(N))
			{
				if (GetFlag(H))
					temp -= 0x06;
				if (GetFlag(C))
					temp -= 0x60;
			}
			else
			{
				if (GetFlag(H) || (temp & 0x0F) > 0x09)
					temp += 0x06;
				if (GetFlag(C) || (temp > 0x9F))
				{
					temp += 0x60;
					SetFlag(C, 1);
				}
			}

			SetFlag(Z, temp == 0x00);
			SetFlag(H, 0);

			LoadA(temp);

			return 4;
			*/
			/*
			uint8_t temp_lo = temp & 0x0F;
			uint8_t temp_hi = (temp & 0xF0) >> 4;

			if (!GetFlag(N))
			{
				if (!GetFlag(C))
				{
					if ((temp_hi >= 0x00) && (temp_hi <= 0x09) && !GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x09))
					{
						LoadA(temp);
						SetFlag(C, 0);
					}
					else if ((temp_hi >= 0x00) && (temp_hi <= 0x08) && !GetFlag(H) && (temp_lo >= 0x0A) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0x06);
						SetFlag(C, 0);
					}
					else if ((temp_hi >= 0x00) && (temp_hi <= 0x09) && GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0x06);
						SetFlag(C, 0);
					}
					else if ((temp_hi >= 0x0A) && (temp_hi <= 0x0F) && !GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x09))
					{
						LoadA(temp + 0x60);
						SetFlag(C, 1);
					}
					else if ((temp_hi >= 0x09) && (temp_hi <= 0x0F) && !GetFlag(H) && (temp_lo >= 0x0A) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0x66);
						SetFlag(C, 1);
					}
					else if ((temp_hi >= 0x0A) && (temp_hi <= 0x0F) && GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0x66);
						SetFlag(C, 1);
					}
				}
				else
				{
					if ((temp_hi >= 0x00) && (temp_hi <= 0x0F) && !GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x09))
					{
						LoadA(temp + 0x60);
						SetFlag(C, 1);
					}
					else if ((temp_hi >= 0x00) && (temp_hi <= 0x0F) && !GetFlag(H) && (temp_lo >= 0x0A) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0x66);
						SetFlag(C, 1);
					}
					else if ((temp_hi >= 0x00) && (temp_hi <= 0x0F) && GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0x66);
						SetFlag(C, 1);
					}
				}
			}
			else
			{
				if (!GetFlag(C))
				{
					if ((temp_hi >= 0x00) && (temp_hi <= 0x09) && !GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x09))
					{
						LoadA(temp);
						SetFlag(C, 0);
					}
					else if ((temp_hi >= 0x00) && (temp_hi <= 0x0F) && GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0xFA);
						SetFlag(C, 0);
					}
				}
				else
				{
					if ((temp_hi >= 0x00) && (temp_hi <= 0x0F) && !GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0xA0);
						SetFlag(C, 1);
					}
					else if ((temp_hi >= 0x00) && (temp_hi <= 0x0F) && GetFlag(H) && (temp_lo >= 0x00) && (temp_lo <= 0x0F))
					{
						LoadA(temp + 0x9A);
						SetFlag(C, 1);
					}
				}
			}

			SetFlag(Z, GetA() == 0x00);
			SetFlag(H, 0);
			return 4; */
		}

		case 0x28: // JR Z,r8

			str_opcode = "JR Z,r8";

			temp = r8();

			if (GetFlag(Z))
			{
				pc += (int8_t)temp;
				return 12;
			}

			return 8;

		case 0x29: // ADD HL,HL

			str_opcode = "ADD HL,HL";

			temp32 = hl + hl;

			SetFlag(N, 0);
			SetFlag(H, ((hl & 0x0FFF) + (hl & 0x0FFF)) >= 0x1000);
			SetFlag(C, temp32 & 0x00010000);

			hl = (uint16_t)(temp32);

			return 8;

		case 0x2A: // LD A,(HL+)

			str_opcode = "LD A,(HL+)";

			LoadA(CpuRead(hl));

			hl++;

			return 8;

		case 0x2B: // DEC HL

			str_opcode = "DEC HL";

			hl--;

			return 8;

		case 0x2C: // INC L

			str_opcode = "INC L";

			SetFlag(Z, GetL() == 0xFF);
			SetFlag(N, 0);
			SetFlag(H, (GetL() & 0x0F) == 0x0F);

			LoadL((uint8_t)(GetL() + 1));

			return 4;

		case 0x2D: // DEC L

			str_opcode = "DEC L";

			SetFlag(Z, GetL() == 0x01);
			SetFlag(N, 1);
			SetFlag(H, (GetL() & 0x0F) == 0x00);

			LoadL((uint8_t)(GetL() - 1));

			return 4;

		case 0x2E: // LD L,d8

			str_opcode = "LD L,d8";

			LoadL(d8());

			return 8;

		case 0x2F: // CPL

			str_opcode = "CPL";

			LoadA(~GetA());

			SetFlag(N, 1);
			SetFlag(H, 1);

			return 4;

		case 0x30: // JR NC,r8

			str_opcode = "JR NC,r8";

			temp = r8();
			if (!GetFlag(C))
			{
				pc += (int8_t)temp;
				return 12;
			}

			return 8;

		case 0x31: // LD SP,d16

			str_opcode = "LD SP,d16";

			sp = d16();

			return 12;

		case 0x32: // LD (HL-),A

			str_opcode = "LD (HL-),A";

			CpuWrite(hl, GetA());
			hl--;

			return 8;

		case 0x33: // INC SP

			str_opcode = "INC SP";

			sp++;

			return 8;

		case 0x34: // INC (HL)

			str_opcode = "INC (HL)";

			temp = CpuRead(hl);

			SetFlag(Z, temp == 0xFF);
			SetFlag(N, 0);
			SetFlag(H, (temp & 0x0F) == 0x0F);

			CpuWrite(hl, (uint8_t)(temp + 1));

			return 12;

		case 0x35: // DEC (HL)

			str_opcode = "DEC (HL)";

			temp = CpuRead(hl);

			SetFlag(Z, temp == 0x01);
			SetFlag(N, 1);
			SetFlag(H, (temp & 0x0F) == 0x00);

			CpuWrite(hl, (uint8_t)(temp - 1));

			return 12;

		case 0x36: // LD (HL),d8

			str_opcode = "LD (HL),d8";

			temp = d8();

			CpuWrite(hl, temp);

			return 12;

		case 0x37: // SCF

			str_opcode = "SCF";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 1);

			return 4;

		case 0x38: // JR C,r8

			str_opcode = "JR C,r8";

			temp = r8();

			if (GetFlag(C))
			{
				pc += (int8_t)temp;
				return 12;
			}

			return 8;

		case 0x39: // ADD HL,SP

			str_opcode = "ADD HL,SP";

			temp32 = hl + sp;

			SetFlag(N, 0);
			SetFlag(H, ((hl & 0x0FFF) + (sp & 0x0FFF)) >= 0x1000);
			SetFlag(C, temp32 & 0x00010000);

			hl = (uint16_t)(temp32);

			return 8;

		case 0x3A: // LD A,(HL-)

			str_opcode = "LD A,(HL-)";

			LoadA(CpuRead(hl));
			hl--;

			return 8;

		case 0x3B: // DEC SP

			str_opcode = "DEC SP";

			sp--;

			return 8;

		case 0x3C: // INC A

			str_opcode = "INC A";

			SetFlag(Z, GetA() == 0xFF);
			SetFlag(N, 0);
			SetFlag(H, (GetA() & 0x0F) == 0x0F);

			LoadA((uint8_t)(GetA() + 1));

			return 4;

		case 0x3D: // DEC A

			str_opcode = "DEC A";

			SetFlag(Z, GetA() == 0x01);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) == 0x00);

			LoadA((uint8_t)(GetA() - 1));

			return 4;

		case 0x3E: // LD A,d8

			str_opcode = "LD A,d8";

			LoadA(d8());

			return 8;

		case 0x3F: // CCF

			str_opcode = "CCF";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, !GetFlag(C));

			return 4;

		case 0x40: // LD B,B

			str_opcode = "LD B,B";

			return 4;

		case 0x41: // LD B,C

			str_opcode = "LD B,C";

			LoadB(GetC());

			return 4;

		case 0x42: // LD B,D

			str_opcode = "LD B,D";

			LoadB(GetD());

			return 4;

		case 0x43: // LD B,E

			str_opcode = "LD B,E";

			LoadB(GetE());

			return 4;

		case 0x44: // LD B,H

			str_opcode = "LD B,H";

			LoadB(GetH());

			return 4;

		case 0x45: // LD B,L

			str_opcode = "LD B,L";

			LoadB(GetL());

			return 4;

		case 0x46: // LD B,(HL)

			str_opcode = "LD B,(HL)";

			LoadB(CpuRead(hl));

			return 8;

		case 0x47: // LD B,A

			str_opcode = "LD B,A";

			LoadB(GetA());

			return 4;

		case 0x48: // LD C,B

			str_opcode = "LD C,B";

			LoadC(GetB());

			return 4;

		case 0x49: // LD C,C

			str_opcode = "LD C,C";

			return 4;

		case 0x4A: // LD C,D

			str_opcode = "LD C,D";

			LoadC(GetD());

			return 4;

		case 0x4B: // LD C,E

			str_opcode = "LD C,E";

			LoadC(GetE());

			return 4;

		case 0x4C: // LD C,H

			str_opcode = "LD C,H";

			LoadC(GetH());

			return 4;

		case 0x4D: // LD C,L

			str_opcode = "LD C,L";

			LoadC(GetL());

			return 4;

		case 0x4E: // LD C,(HL)

			str_opcode = "LD C,(HL)";

			LoadC(CpuRead(hl));

			return 8;

		case 0x4F: // LD C,A

			str_opcode = "LD C,A";

			LoadC(GetA());

			return 4;

		case 0x50: // LD D,B

			str_opcode = "LD D,B";

			LoadD(GetB());

			return 4;

		case 0x51: // LD D,C

			str_opcode = "LD D,C";

			LoadD(GetC());

			return 4;

		case 0x52: // LD D,D

			str_opcode = "LD D,D";

			return 4;

		case 0x53: // LD D,E

			str_opcode = "LD D,E";

			LoadD(GetE());

			return 4;

		case 0x54: // LD D,H

			str_opcode = "LD D,H";

			LoadD(GetH());

			return 4;

		case 0x55: // LD D,L

			str_opcode = "LD D,L";

			LoadD(GetL());

			return 4;

		case 0x56: // LD D,(HL)

			str_opcode = "LD D,(HL)";

			LoadD(CpuRead(hl));

			return 8;

		case 0x57: // LD D,A

			str_opcode = "LD D,A";

			LoadD(GetA());

			return 4;

		case 0x58: // LD E,B

			str_opcode = "LD E,B";

			LoadE(GetB());

			return 4;

		case 0x59: // LD E,C

			str_opcode = "LD E,C";

			LoadE(GetC());

			return 4;

		case 0x5A: // LD E,D

			str_opcode = "LD E,D";

			LoadE(GetD());

			return 4;

		case 0x5B: // LD E,E

			str_opcode = "LD E,E";

			return 4;

		case 0x5C: // LD E,H

			str_opcode = "LD E,H";

			LoadE(GetH());

			return 4;

		case 0x5D: // LD E,L

			str_opcode = "LD E,L";

			LoadE(GetL());

			return 4;

		case 0x5E: // LD E,(HL)

			str_opcode = "LD E,(HL)";

			LoadE(CpuRead(hl));

			return 8;

		case 0x5F: // LD E,A

			str_opcode = "LD E,A";

			LoadE(GetA());

			return 4;

		case 0x60: // LD H,B

			str_opcode = "LD H,B";

			LoadH(GetB());

			return 4;

		case 0x61: // LD H,C

			str_opcode = "LD H,C";

			LoadH(GetC());

			return 4;

		case 0x62: // LD H,D

			str_opcode = "LD H,D";

			LoadH(GetD());

			return 4;

		case 0x63: // LD H,E

			str_opcode = "LD H,E";

			LoadH(GetE());

			return 4;

		case 0x64: // LD H,H

			str_opcode = "LD H,H";

			return 4;

		case 0x65: // LD H,L

			str_opcode = "LD H,L";

			LoadH(GetL());

			return 4;

		case 0x66: // LD H,(HL)

			str_opcode = "LD H,(HL)";

			LoadH(CpuRead(hl));

			return 8;

		case 0x67: // LD H,A

			str_opcode = "LD H,A";

			LoadH(GetA());

			return 4;

		case 0x68: // LD L,B

			str_opcode = "LD L,B";

			LoadL(GetB());

			return 4;

		case 0x69: // LD L,C

			str_opcode = "LD L,C";

			LoadL(GetC());

			return 4;

		case 0x6A: // LD L,D

			str_opcode = "LD L,D";

			LoadL(GetD());

			return 4;

		case 0x6B: // LD L,E

			str_opcode = "LD L,E";

			LoadL(GetE());

			return 4;

		case 0x6C: // LD L,H

			str_opcode = "LD L,H";

			LoadL(GetH());

			return 4;

		case 0x6D: // LD L,L

			str_opcode = "LD L,L";

			return 4;

		case 0x6E: // LD L,(HL)

			str_opcode = "LD L,(HL)";

			LoadL(CpuRead(hl));

			return 8;

		case 0x6F: // LD L,A

			str_opcode = "LD L,A";

			LoadL(GetA());

			return 4;

		case 0x70: // LD (HL),B

			str_opcode = "LD (HL),B";

			CpuWrite(hl, GetB());

			return 8;

		case 0x71: // LD (HL),C

			str_opcode = "LD (HL),C";

			CpuWrite(hl, GetC());

			return 8;

		case 0x72: // LD (HL),D

			str_opcode = "LD (HL),D";

			CpuWrite(hl, GetD());

			return 8;

		case 0x73: // LD (HL),E

			str_opcode = "LD (HL),E";

			CpuWrite(hl, GetE());

			return 8;

		case 0x74: // LD (HL),H

			str_opcode = "LD (HL),H";

			CpuWrite(hl, GetH());

			return 8;

		case 0x75: // LD (HL),L

			str_opcode = "LD (HL),L";

			CpuWrite(hl, GetL());

			return 8;

		case 0x76: // HALT

			str_opcode = "HALT";

			bHALT = true;

			return 4;

		case 0x77: // LD (HL),A

			str_opcode = "LD (HL),A";

			CpuWrite(hl, GetA());

			return 8;

		case 0x78: // LD A,B

			str_opcode = "LD A,B";

			LoadA(GetB());

			return 4;

		case 0x79: // LD A,C

			str_opcode = "LD A,C";

			LoadA(GetC());

			return 4;

		case 0x7A: // LD A,D

			str_opcode = "LD A,D";

			LoadA(GetD());

			return 4;

		case 0x7B: // LD A,E

			str_opcode = "LD A,E";

			LoadA(GetE());

			return 4;

		case 0x7C: // LD A,H

			str_opcode = "LD A,H";

			LoadA(GetH());

			return 4;

		case 0x7D: // LD A,L

			str_opcode = "LD A,L";

			LoadA(GetL());

			return 4;

		case 0x7E: // LD A,(HL)

			str_opcode = "LD A,(HL)";

			LoadA(CpuRead(hl));

			return 8;

		case 0x7F: // LD A,A

			str_opcode = "LD A,A";

			return 8;

		case 0x80: // ADD A,B

			str_opcode = "ADD A,B";

			temp = GetB();

			temp16 = GetA() + temp;

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x81: // ADD A,C

			str_opcode = "ADD A,C";

			temp = GetC();

			temp16 = GetA() + temp;

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x82: // ADD A,D

			str_opcode = "ADD A,D";

			temp = GetD();

			temp16 = GetA() + temp;

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x83: // ADD A,E

			str_opcode = "ADD A,E";

			temp = GetE();

			temp16 = GetA() + temp;

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x84: // ADD A,H

			str_opcode = "ADD A,H";

			temp = GetH();

			temp16 = GetA() + temp;

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x85: // ADD A,L

			str_opcode = "ADD A,L";

			temp = GetL();

			temp16 = GetA() + temp;

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x86: // ADD A,(HL)

			str_opcode = "ADD A,(HL)";

			temp = CpuRead(hl);

			temp16 = GetA() + temp;

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 8;

		case 0x87: // ADD A,A

			str_opcode = "ADD A,A";

			temp16 = GetA() + GetA();

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (2 * (GetA() & 0x0F)) & 0xF0);
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x88: // ADC A,B

			str_opcode = "ADC A,B";

			temp = GetB();

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0)); // Not implemented
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x89: // ADC A,C

			str_opcode = "ADC A,C";

			temp = GetC();

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x8A: // ADC A,D

			str_opcode = "ADC A,D";

			temp = GetD();

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x8B: // ADC A,E

			str_opcode = "ADC A,E";

			temp = GetE();

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x8C: // ADC A,H

			str_opcode = "ADC A,H";

			temp = GetH();

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x8D: // ADC A,L

			str_opcode = "ADC A,L";

			temp = GetL();

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x8E: // ADC A,(HL)

			str_opcode = "ADC A,(HL)";

			temp = CpuRead(hl);

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 8;

		case 0x8F: // ADC A,A

			str_opcode = "ADC A,A";

			temp = GetA();

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 4;

		case 0x90: // SUB B

			str_opcode = "SUB B";

			temp = GetB();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 4;

		case 0x91: // SUB C

			str_opcode = "SUB C";

			temp = GetC();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 4;

		case 0x92: // SUB D

			str_opcode = "SUB D";

			temp = GetD();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 4;

		case 0x93: // SUB E

			str_opcode = "SUB E";

			temp = GetE();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 4;

		case 0x94: // SUB H

			str_opcode = "SUB H";

			temp = GetH();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 4;

		case 0x95: // SUB L

			str_opcode = "SUB L";

			temp = GetL();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 4;

		case 0x96: // SUB (HL)

			str_opcode = "SUB (HL)";

			temp = CpuRead(hl);

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 8;

		case 0x97: // SUB A

			str_opcode = "SUB A";

			temp = GetA();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 4;

		case 0x98: // SBC A,B

			str_opcode = "SBC A,B";

			temp16 = GetB() + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((GetB() & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 4;

		case 0x99: // SBC A,C

			str_opcode = "SBC A,C";

			temp16 = GetC() + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((GetC() & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 4;

		case 0x9A: // SBC A,D

			str_opcode = "SBC A,D";

			temp16 = GetD() + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((GetD() & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 4;

		case 0x9B: // SBC A,E

			str_opcode = "SBC A,E";

			temp16 = GetE() + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((GetE() & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 4;

		case 0x9C: // SBC A,H

			str_opcode = "SBC A,H";

			temp16 = GetH() + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((GetH() & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 4;

		case 0x9D: // SBC A,L

			str_opcode = "SBC A,L";

			temp16 = GetL() + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((GetL() & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 4;

		case 0x9E: // SBC A,(HL)

			str_opcode = "SBC A,(HL)";

			temp = CpuRead(hl);
			temp16 = temp + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 8;

		case 0x9F: // SBC A,A

			str_opcode = "SBC A,A";

			temp16 = GetA() + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((GetA() & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 4;

		case 0xA0: // AND B

			str_opcode = "AND B";

			LoadA(GetA() & GetB());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 4;

		case 0xA1: // AND C

			str_opcode = "AND C";

			LoadA(GetA() & GetC());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 4;

		case 0xA2: // AND D

			str_opcode = "AND D";

			LoadA(GetA() & GetD());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 4;

		case 0xA3: // AND E

			str_opcode = "AND E";

			LoadA(GetA() & GetE());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 4;

		case 0xA4: // AND H

			str_opcode = "AND H";

			LoadA(GetA() & GetH());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 4;

		case 0xA5: // AND L

			str_opcode = "AND L";

			LoadA(GetA() & GetL());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 4;

		case 0xA6: // AND (HL)

			str_opcode = "AND (HL)";

			LoadA(GetA() & CpuRead(hl));

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 8;

		case 0xA7: // AND A

			str_opcode = "AND A";

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 4;

		case 0xA8: // XOR B

			str_opcode = "XOR B";

			LoadA(GetA() ^ GetB());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xA9: // XOR C

			str_opcode = "XOR C";

			LoadA(GetA() ^ GetC());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xAA: // XOR D

			str_opcode = "XOR D";

			LoadA(GetA() ^ GetD());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xAB: // XOR E

			str_opcode = "XOR E";

			LoadA(GetA() ^ GetE());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xAC: // XOR H

			str_opcode = "XOR H";

			LoadA(GetA() ^ GetH());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xAD: // XOR L

			str_opcode = "XOR L";

			LoadA(GetA() ^ GetL());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xAE: // XOR (HL)

			str_opcode = "XOR (HL)";

			LoadA(GetA() ^ CpuRead(hl));

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 8;

		case 0xAF: // XOR A

			str_opcode = "XOR A";

			LoadA(GetA() ^ GetA());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xB0: // OR B

			str_opcode = "OR B";

			LoadA(GetA() | GetB());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xB1: // OR C

			str_opcode = "OR C";

			LoadA(GetA() | GetC());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xB2: // OR D

			str_opcode = "OR D";

			LoadA(GetA() | GetD());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xB3: // OR E

			str_opcode = "OR E";

			LoadA(GetA() | GetE());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xB4: // OR H

			str_opcode = "OR H";

			LoadA(GetA() | GetH());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xB5: // OR L

			str_opcode = "OR L";

			LoadA(GetA() | GetL());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xB6: // OR (HL)

			str_opcode = "OR (HL)";

			LoadA(GetA() | CpuRead(hl));

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 8;

		case 0xB7: // OR A

			str_opcode = "OR A";

			LoadA(GetA() | GetA());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 4;

		case 0xB8: // CP B

			str_opcode = "CP B";

			temp = GetB();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			return 4;

		case 0xB9: // CP C

			str_opcode = "CP C";

			temp = GetC();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			return 4;

		case 0xBA: // CP D

			str_opcode = "CP D";

			temp = GetD();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			return 4;

		case 0xBB: // CP E

			str_opcode = "CP E";

			temp = GetE();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			return 4;

		case 0xBC: // CP H

			str_opcode = "CP H";

			temp = GetH();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			return 4;

		case 0xBD: // CP L

			str_opcode = "CP L";

			temp = GetL();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			return 4;

		case 0xBE: // CP (HL)

			str_opcode = "CP (HL)";

			temp = CpuRead(hl);

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			return 8;

		case 0xBF: // CP A

			str_opcode = "CP A";

			temp = GetA();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			return 4;

		case 0xC0: // RET NZ

			str_opcode = "RET NZ";

			if (!GetFlag(Z))
			{
				pc_lo = CpuRead(sp);
				sp++;
				pc_hi = CpuRead(sp);
				sp++;

				pc = ((uint16_t)pc_hi << 8) | pc_lo;
				return 20;
			}

			return 8;

		case 0xC1: // POP BC

			str_opcode = "POP BC";

			LoadC(CpuRead(sp));
			sp++;
			LoadB(CpuRead(sp));
			sp++;

			return 12;

		case 0xC2: // JP NZ,a16

			str_opcode = "JP NZ,a16";

			addr = a16();

			if (!GetFlag(Z))
			{

				pc = addr;

				return 16;
			}

			return 12;

		case 0xC3: // JP a16

			str_opcode = "JP a16";

			pc = a16();

			return 16;

		case 0xC4: // CALL NZ,a16

			str_opcode = "CALL NZ,a16";

			addr = a16();

			if (!GetFlag(Z))
			{
				sp--;
				CpuWrite(sp, (pc & 0xFF00) >> 8);
				sp--;
				CpuWrite(sp, pc & 0x00FF);

				pc = addr;

				return 24;
			}

			return 12;

		case 0xC5: // PUSH BC

			str_opcode = "PUSH BC";

			sp--;
			CpuWrite(sp, GetB());
			sp--;
			CpuWrite(sp, GetC());

			return 16;

		case 0xC6: // ADD A,d8

			str_opcode = "ADD A,d8";

			temp = d8();

			temp16 = GetA() + temp;

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 8;

		case 0xC7: // RST 00H

			str_opcode = "RST 00H";

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0000;

			return 16;

		case 0xC8: // RET Z

			str_opcode = "RET Z";

			if (GetFlag(Z))
			{
				pc_lo = CpuRead(sp);
				sp++;
				pc_hi = CpuRead(sp);
				sp++;

				pc = ((uint16_t)pc_hi << 8) | pc_lo;
				return 20;
			}

			return 8;

		case 0xC9: // RET

			str_opcode = "RET";

			pc_lo = CpuRead(sp);
			sp++;
			pc_hi = CpuRead(sp);
			sp++;

			pc = ((uint16_t)pc_hi << 8) | pc_lo;

			return 16;

		case 0xCA: // JP Z,a16

			str_opcode = "JP Z,a16";

			addr = a16();

			if (GetFlag(Z))
			{
				pc = addr;

				return 16;
			}

			return 12;

		case 0xCB: // PREFIX CB

			str_opcode = "CB";

			Prefix_CB = true;

			return 4;

		case 0xCC: // CALL Z,a16

			str_opcode = "CALL Z,a16";

			addr = a16();

			if (GetFlag(Z))
			{
				sp--;
				CpuWrite(sp, (pc & 0xFF00) >> 8);
				sp--;
				CpuWrite(sp, pc & 0x00FF);

				pc = addr;

				return 24;
			}

			return 12;

		case 0xCD: // CALL a16

			str_opcode = "CALL a16";

			addr = a16();

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = addr;

			return 24;

		case 0xCE: // ADC A,d8

			str_opcode = "ADC A,d8";

			temp = d8();

			temp16 = GetA() + temp + (uint16_t)GetFlag(C);

			SetFlag(Z, (uint8_t)temp16 == 0x00);
			SetFlag(N, 0);
			SetFlag(H, (((GetA() & 0x0F) + (temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, temp16 & 0xFF00);

			LoadA((uint8_t)(temp16));

			return 8;

		case 0xCF: // RST 08H

			str_opcode = "RST 08H";

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0000 + 0x0008;

			return 16;

		case 0xD0: // RET NC

			str_opcode = "RET NC";

			if (!GetFlag(C))
			{
				pc_lo = CpuRead(sp);
				sp++;
				pc_hi = CpuRead(sp);
				sp++;

				pc = ((uint16_t)pc_hi << 8) | pc_lo;
				return 20;
			}

			return 8;

		case 0xD1: // POP DE

			str_opcode = "POP DE";

			LoadE(CpuRead(sp));
			sp++;
			LoadD(CpuRead(sp));
			sp++;

			return 12;

		case 0xD2: // JP NC,a16

			str_opcode = "JP NC,a16";

			addr = a16();

			if (!GetFlag(C))
			{

				pc = addr;

				return 16;
			}

			return 12;

		case 0xD4: // CALL NC,a16

			str_opcode = "CALL NC,a16";

			addr = a16();

			if (!GetFlag(C))
			{
				sp--;
				CpuWrite(sp, (pc & 0xFF00) >> 8);
				sp--;
				CpuWrite(sp, pc & 0x00FF);

				pc = addr;

				return 24;
			}

			return 12;

		case 0xD5: // PUSH DE

			str_opcode = "PUSH DE";

			sp--;
			CpuWrite(sp, GetD());
			sp--;
			CpuWrite(sp, GetE());

			return 16;

		case 0xD6: // SUB d8

			str_opcode = "SUB d8";

			temp = d8();

			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F));
			SetFlag(C, GetA() < temp);

			LoadA(GetA() - temp);

			return 8;

		case 0xD7: // RST 10H

			str_opcode = "RST 10H";

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0000 + 0x0010;

			return 16;

		case 0xD8: // RET C

			str_opcode = "RET C";

			if (GetFlag(C))
			{
				pc_lo = CpuRead(sp);
				sp++;
				pc_hi = CpuRead(sp);
				sp++;

				pc = ((uint16_t)pc_hi << 8) | pc_lo;

				return 20;
			}

			return 8;

		case 0xD9: // RETI

			str_opcode = "RETI";

			pc_lo = CpuRead(sp);
			sp++;
			pc_hi = CpuRead(sp);
			sp++;

			pc = ((uint16_t)pc_hi << 8) | pc_lo;

			bIME = true;

			return 16;

		case 0xDA: // JP C,a16

			str_opcode = "JP C,a16";

			addr = a16();

			if (GetFlag(C))
			{

				pc = addr;

				return 16;
			}

			return 12;

		case 0xDC: // CALL C,a16

			str_opcode = "CALL C,a16";

			addr = a16();

			if (GetFlag(C))
			{
				sp--;
				CpuWrite(sp, (pc & 0xFF00) >> 8);
				sp--;
				CpuWrite(sp, pc & 0x00FF);

				pc = addr;

				return 24;
			}

			return 12;

		case 0xDE: // SBC A,d8

			str_opcode = "SBC A,d8";

			temp = d8();

			temp16 = temp + GetFlag(C);

			SetFlag(Z, GetA() == (uint8_t)temp16);
			SetFlag(N, 1);
			SetFlag(H, ((GetA() & 0x0F) < ((uint8_t)temp16 & 0x0F))
				|| (((temp & 0x0F) + GetFlag(C)) & 0xF0));
			SetFlag(C, (GetA() < (uint8_t)temp16) || (temp16 & 0x0100));

			LoadA(GetA() - (uint8_t)temp16);

			return 8;

		case 0xDF: // RST 18H

			str_opcode = "RST 18H";

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0000 + 0x0018;

			return 16;

		case 0xE0: // LDH (a8),A == LD ($FF00+a8),A

			str_opcode = "LDH (a8),A";

			CpuWrite(0xFF00 + a8(), GetA());

			return 12;

		case 0xE1: // POP HL

			str_opcode = "POP HL";

			LoadL(CpuRead(sp));
			sp++;
			LoadH(CpuRead(sp));
			sp++;

			return 12;

		case 0xE2: // LD (C),A == LD (0xFF00 + C),A

			str_opcode = "LD (C),A";

			CpuWrite(0xFF00 + GetC(), GetA());

			return 8;

		case 0xE5: // PUSH HL 

			str_opcode = "PUSH HL";

			sp--;
			CpuWrite(sp, GetH());
			sp--;
			CpuWrite(sp, GetL());

			return 16;

		case 0xE6: // AND d8

			str_opcode = "AND d8";

			LoadA(GetA() & d8());

			SetFlag(Z, GetA() == 0);
			SetFlag(N, 0);
			SetFlag(H, 1);
			SetFlag(C, 0);

			return 8;

		case 0xE7: // RST 20H

			str_opcode = "RST 20H";

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0000 + 0x020;

			return 16;

		case 0xE8: // ADD SP,r8

			str_opcode = "ADD SP,r8";

			temp = r8();
			temp32 = sp + (int8_t)temp;

			SetFlag(Z, 0);
			SetFlag(N, 0);
			SetFlag(H, ((sp & 0x000F) + ((uint16_t)temp & 0x000F)) >= 0x0010);
			SetFlag(C, (sp & 0x00FF) + (temp & 0x00FF) >= 0x0100);

			sp = (uint16_t)(temp32);

			return 16;

		case 0xE9: // JP (HL)

			str_opcode = "JP (HL)";

			pc = hl;

			return 4;

		case 0xEA: // LD (a16),A

			str_opcode = "LD (a16),A";

			CpuWrite(a16(), GetA());

			return 16;

		case 0xEE: // XOR d8

			str_opcode = "XOR d8";

			temp = d8();

			LoadA(GetA() ^ temp);

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 8;

		case 0xEF: // RST 28H

			str_opcode = "RST 28H";

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0000 + 0x0028;

			return 16;

		case 0xF0: // LDH A,(a8)

			str_opcode = "LDH A,(a8)";

			LoadA(CpuRead(0xFF00 + a8()));

			return 12;

		case 0xF1: // POP AF

			str_opcode = "POP AF";

			LoadF(CpuRead(sp) & 0xF0);
			sp++;
			LoadA(CpuRead(sp));
			sp++;

			return 12;

		case 0xF2: // LD A,(C)

			str_opcode = "LD A,(C)";

			LoadA(CpuRead(0xFF00 + GetC()));

			return 8;

		case 0xF3: // DI

			str_opcode = "DI";

			bIME = false;

			// Not Implemented accurate I think.
			// Disables Interrupts after the instruction is executed.

			return 4;

		case 0xF5: // PUSH AF

			str_opcode = "PUSH AF";

			sp--;
			CpuWrite(sp, GetA());
			sp--;
			CpuWrite(sp, f);

			return 16;

		case 0xF6: // OR d8

			str_opcode = "OR d8";

			LoadA(GetA() | d8());

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			return 8;

		case 0xF7: // RST 30H

			str_opcode = "RST 30H";

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0000 + 0x0030;

			return 16;

		case 0xF8: // LD HL,SP+r8

			str_opcode = "LD HL,SP+r8";

			temp = r8();
			temp32 = sp + (int8_t)temp;

			SetFlag(Z, 0);
			SetFlag(N, 0);
			SetFlag(H, ((sp & 0x000F) + ((uint16_t)temp & 0x000F)) >= 0x0010);
			SetFlag(C, (sp & 0x00FF) + (temp & 0x00FF) >= 0x0100);

			hl = (uint16_t)(temp32);

			return 12;

		case 0xF9: // LD SP,HL

			str_opcode = "LD SP,HL";

			sp = hl;

			return 8;

		case 0xFA: // LD A,(a16)

			str_opcode = "LD A,(a16)";

			LoadA(CpuRead(a16()));

			return 16;

		case 0xFB: // EI

			str_opcode = "EI";

			bIME = true;

			// The same conclusion as for DI instruction.

			return 4;

		case 0xFE: // CP d8

			str_opcode = "CP d8";

			temp = d8();
			SetFlag(Z, GetA() == temp);
			SetFlag(N, 1);
			SetFlag(H, (GetA() & 0x0F) < (temp & 0x0F)); // >
			SetFlag(C, GetA() < temp); // < ???

			return 8;

		case 0xFF: // RST 38H

			str_opcode = "RST 38H";

			sp--;
			CpuWrite(sp, (pc & 0xFF00) >> 8);
			sp--;
			CpuWrite(sp, pc & 0x00FF);

			pc = 0x0000 + 0x0038;

			return 16;

		default:
			std::cout << "The illegal 0x" << std::uppercase << std::hex << opcode << " isn't implemented yet" << std::endl;
			std::cout << "PC: 0x" << std::uppercase << std::hex << pc << std::endl;
			return 0;
		}
	}
	else
	{
		switch (opcode)
		{
		case 0x00: // RLC B

			str_opcode = "RLC B";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetB() >> 7);

			LoadB((GetB() << 1) | GetFlag(C));

			SetFlag(Z, GetB() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x01: // RLC C

			str_opcode = "RLC C";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetC() >> 7);

			LoadC((GetC() << 1) | GetFlag(C));

			SetFlag(Z, GetC() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x02: // RLC D

			str_opcode = "RLC D";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetD() >> 7);

			LoadD((GetD() << 1) | GetFlag(C));

			SetFlag(Z, GetD() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x03: // RLC E

			str_opcode = "RLC E";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetE() >> 7);

			LoadE((GetE() << 1) | GetFlag(C));

			SetFlag(Z, GetE() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x04: // RLC H

			str_opcode = "RLC H";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetH() >> 7);

			LoadH((GetH() << 1) | GetFlag(C));

			SetFlag(Z, GetH() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x05: // RLC L

			str_opcode = "RLC L";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetL() >> 7);

			LoadL((GetL() << 1) | GetFlag(C));

			SetFlag(Z, GetL() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x06: // RLC (HL)

			str_opcode = "RLC (HL)";

			temp = CpuRead(hl);

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, temp >> 7);

			CpuWrite(hl, (temp << 1) | GetFlag(C));

			SetFlag(Z, temp == 0x00);

			Prefix_CB = false;

			return 16;

		case 0x07: // RLC A

			str_opcode = "RLC A";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetA() >> 7);

			LoadA((GetA() << 1) | GetFlag(C));

			SetFlag(Z, GetA() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x08: // RRC B

			str_opcode = "RRC B";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetB() & 0x01);

			LoadB((GetB() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetB() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x09: // RRC C

			str_opcode = "RRC C";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetC() & 0x01);

			LoadC((GetC() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetC() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x0A: // RRC D

			str_opcode = "RRC D";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetD() & 0x01);

			LoadD((GetD() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetD() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x0B: // RRC E

			str_opcode = "RRC E";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetE() & 0x01);

			LoadE((GetE() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetE() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x0C: // RRC H

			str_opcode = "RRC H";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetH() & 0x01);

			LoadH((GetH() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetH() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x0D: // RRC L

			str_opcode = "RRC L";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetL() & 0x01);

			LoadL((GetL() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetL() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x0E: // RRC (HL)

			str_opcode = "RRC (HL)";

			temp = CpuRead(hl);

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, temp & 0x01);

			CpuWrite(hl, (temp >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, temp == 0x00);

			Prefix_CB = false;

			return 16;

		case 0x0F: // RRC A

			str_opcode = "RRC A";

			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, GetA() & 0x01);

			LoadA((GetA() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetA() == 0x00);

			Prefix_CB = false;

			return 8;

		case 0x10: // RL B

			str_opcode = "RL B";

			c = GetB() >> 7;
			temp = (GetB() << 1) | GetFlag(C);

			LoadB(temp);

			SetFlag(Z, temp == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x11: // RL C

			str_opcode = "RL C";


			c = GetC() >> 7;
			temp = (GetC() << 1) | GetFlag(C);

			LoadC(temp);

			SetFlag(Z, temp == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x12: // RL D

			str_opcode = "RL D";


			c = GetD() >> 7;
			temp = (GetD() << 1) | GetFlag(C);

			LoadD(temp);

			SetFlag(Z, temp == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x13: // RL E

			str_opcode = "RL E";


			c = GetE() >> 7;
			temp = (GetE() << 1) | GetFlag(C);

			LoadE(temp);

			SetFlag(Z, temp == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x14: // RL H

			str_opcode = "RL H";


			c = GetH() >> 7;
			temp = (GetH() << 1) | GetFlag(C);

			LoadH(temp);

			SetFlag(Z, temp == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x15: // RL L

			str_opcode = "RL L";


			c = GetL() >> 7;
			temp = (GetL() << 1) | GetFlag(C);

			LoadL(temp);

			SetFlag(Z, temp == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x16: // RL (HL)

			str_opcode = "RL (HL)";

			temp2 = CpuRead(hl);

			c = temp2 >> 7;
			temp = (temp2 << 1) | GetFlag(C);

			CpuWrite(hl, temp);

			SetFlag(Z, temp == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 16;

		case 0x17: // RL A

			str_opcode = "RL A";


			c = GetA() >> 7;
			temp = (GetA() << 1) | GetFlag(C);

			LoadA(temp);

			SetFlag(Z, temp == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x18: // RR B

			str_opcode = "RR B";

			c = GetB() & 0x01;
			LoadB((GetB() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetB() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x19: // RR C

			str_opcode = "RR C";

			c = GetC() & 0x01;
			LoadC((GetC() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetC() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x1A: // RR D

			str_opcode = "RR D";

			c = GetD() & 0x01;
			LoadD((GetD() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetD() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x1B: // RR E

			str_opcode = "RR E";

			c = GetE() & 0x01;
			LoadE((GetE() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetE() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x1C: // RR H

			str_opcode = "RR H";

			c = GetH() & 0x01;
			LoadH((GetH() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetH() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x1D: // RR L

			str_opcode = "RR L";

			c = GetL() & 0x01;
			LoadL((GetL() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetL() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x1E: // RR (HL)

			str_opcode = "RR (HL)";

			temp2 = CpuRead(hl);

			c = temp2 & 0x01;
			CpuWrite(hl, (temp2 >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, CpuRead(hl) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 16;

		case 0x1F: // RR A

			str_opcode = "RR A";

			c = GetA() & 0x01;
			LoadA((GetA() >> 1) | (GetFlag(C) << 7));

			SetFlag(Z, GetA() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, c);

			Prefix_CB = false;

			return 8;

		case 0x20: // SLA B

			str_opcode = "SLA B";

			SetFlag(C, GetB() & 0x80);

			LoadB(GetB() << 1);

			SetFlag(Z, GetB() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x21: // SLA C

			str_opcode = "SLA C";

			SetFlag(C, GetC() & 0x80);

			LoadC(GetC() << 1);

			SetFlag(Z, GetC() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x22: // SLA D

			str_opcode = "SLA D";

			SetFlag(C, GetD() & 0x80);

			LoadD(GetD() << 1);

			SetFlag(Z, GetD() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x23: // SLA E

			str_opcode = "SLA E";

			SetFlag(C, GetE() & 0x80);

			LoadE(GetE() << 1);

			SetFlag(Z, GetE() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x24: // SLA H

			str_opcode = "SLA H";

			SetFlag(C, GetH() & 0x80);

			LoadH(GetH() << 1);

			SetFlag(Z, GetH() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x25: // SLA L

			str_opcode = "SLA L";

			SetFlag(C, GetL() & 0x80);

			LoadL(GetL() << 1);

			SetFlag(Z, GetL() == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x26: // SLA (HL)

			str_opcode = "SLA (HL)";

			temp = CpuRead(hl);

			SetFlag(C, temp & 0x80);

			CpuWrite(hl, (temp << 1));

			SetFlag(Z, CpuRead(hl) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 16;

		case 0x27: // SLA A

			str_opcode = "SLA A";

			SetFlag(C, GetA() & 0x80);

			LoadA(GetA() << 1);

			SetFlag(Z, GetA() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x28: // SRA B

			str_opcode = "SRA B";

			SetFlag(C, GetB() & 0x01);

			c = GetB() & 0x80;
			LoadB((GetB() >> 1) | c);

			SetFlag(Z, GetB() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x29: // SRA C

			str_opcode = "SRA C";

			SetFlag(C, GetC() & 0x01);

			c = GetC() & 0x80;
			LoadC((GetC() >> 1) | c);

			SetFlag(Z, GetC() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x2A: // SRA D

			str_opcode = "SRA D";

			SetFlag(C, GetD() & 0x01);

			c = GetD() & 0x80;
			LoadD((GetD() >> 1) | c);

			SetFlag(Z, GetD() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x2B: // SRA E

			str_opcode = "SRA E";

			SetFlag(C, GetE() & 0x01);

			c = GetE() & 0x80;
			LoadE((GetE() >> 1) | c);

			SetFlag(Z, GetE() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x2C: // SRA H

			str_opcode = "SRA H";

			SetFlag(C, GetH() & 0x01);

			c = GetH() & 0x80;
			LoadH((GetH() >> 1) | c);

			SetFlag(Z, GetH() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x2D: // SRA L

			str_opcode = "SRA L";

			SetFlag(C, GetL() & 0x01);

			c = GetL() & 0x80;
			LoadL((GetL() >> 1) | c);

			SetFlag(Z, GetL() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x2E: // SRA (HL)

			str_opcode = "SRA (HL)";

			temp = CpuRead(hl);

			SetFlag(C, temp & 0x01);

			c = temp & 0x80;
			CpuWrite(hl, ((temp >> 1) | c));

			SetFlag(Z, CpuRead(hl) == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 16;

		case 0x2F: // SRA A

			str_opcode = "SRA A";

			SetFlag(C, GetA() & 0x01);

			c = GetA() & 0x80;
			LoadA((GetA() >> 1) | c);

			SetFlag(Z, GetA() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x30: // SWAP B

			str_opcode = "SWAP B";

			nibble1 = GetB() & 0x0F;
			nibble2 = GetB() & 0xF0;

			nibble1 <<= 4;
			nibble2 >>= 4;

			temp = nibble1 | nibble2;

			LoadB(temp);

			SetFlag(Z, temp == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			Prefix_CB = false;

			return 8;

		case 0x31: // SWAP C

			str_opcode = "SWAP C";

			nibble1 = GetC() & 0x0F;
			nibble2 = GetC() & 0xF0;

			nibble1 <<= 4;
			nibble2 >>= 4;

			temp = nibble1 | nibble2;

			LoadC(temp);

			SetFlag(Z, temp == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			Prefix_CB = false;

			return 8;

		case 0x32: // SWAP D

			str_opcode = "SWAP D";

			nibble1 = GetD() & 0x0F;
			nibble2 = GetD() & 0xF0;

			nibble1 <<= 4;
			nibble2 >>= 4;

			temp = nibble1 | nibble2;

			LoadD(temp);

			SetFlag(Z, temp == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			Prefix_CB = false;

			return 8;

		case 0x33: // SWAP E

			str_opcode = "SWAP E";

			nibble1 = GetE() & 0x0F;
			nibble2 = GetE() & 0xF0;

			nibble1 <<= 4;
			nibble2 >>= 4;

			temp = nibble1 | nibble2;

			LoadE(temp);

			SetFlag(Z, temp == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			Prefix_CB = false;

			return 8;

		case 0x34: // SWAP H

			str_opcode = "SWAP H";

			nibble1 = GetH() & 0x0F;
			nibble2 = GetH() & 0xF0;

			nibble1 <<= 4;
			nibble2 >>= 4;

			temp = nibble1 | nibble2;

			LoadH(temp);

			SetFlag(Z, temp == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			Prefix_CB = false;

			return 8;

		case 0x35: // SWAP L

			str_opcode = "SWAP L";

			nibble1 = GetL() & 0x0F;
			nibble2 = GetL() & 0xF0;

			nibble1 <<= 4;
			nibble2 >>= 4;

			temp = nibble1 | nibble2;

			LoadL(temp);

			SetFlag(Z, temp == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			Prefix_CB = false;

			return 8;

		case 0x36: // SWAP (HL)

			str_opcode = "SWAP (HL)";

			temp2 = CpuRead(hl);

			nibble1 = temp2 & 0x0F;
			nibble2 = temp2 & 0xF0;

			nibble1 <<= 4;
			nibble2 >>= 4;

			temp = nibble1 | nibble2;

			CpuWrite(hl, temp);

			SetFlag(Z, temp == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			Prefix_CB = false;

			return 16;

		case 0x37: // SWAP A

			str_opcode = "SWAP A";

			nibble1 = GetA() & 0x0F;
			nibble2 = GetA() & 0xF0;

			nibble1 <<= 4;
			nibble2 >>= 4;

			temp = nibble1 | nibble2;

			LoadA(temp);

			SetFlag(Z, temp == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);
			SetFlag(C, 0);

			Prefix_CB = false;

			return 8;

		case 0x38: // SRL B

			str_opcode = "SRL B";

			SetFlag(C, GetB() & 0x01);

			LoadB(GetB() >> 1);

			SetFlag(Z, GetB() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x39: // SRL C

			str_opcode = "SRL C";

			SetFlag(C, GetC() & 0x01);

			LoadC(GetC() >> 1);

			SetFlag(Z, GetC() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x3A: // SRL D

			str_opcode = "SRL D";

			SetFlag(C, GetD() & 0x01);

			LoadD(GetD() >> 1);

			SetFlag(Z, GetD() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x3B: // SRL E

			str_opcode = "SRL E";

			SetFlag(C, GetE() & 0x01);

			LoadE(GetE() >> 1);

			SetFlag(Z, GetE() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x3C: // SRL H

			str_opcode = "SRL H";

			SetFlag(C, GetH() & 0x01);

			LoadH(GetH() >> 1);

			SetFlag(Z, GetH() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x3D: // SRL L

			str_opcode = "SRL L";

			SetFlag(C, GetL() & 0x01);

			LoadL(GetL() >> 1);

			SetFlag(Z, GetL() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x3E: // SRL (HL)

			str_opcode = "SRL (HL)";

			temp = CpuRead(hl);

			SetFlag(C, temp & 0x01);

			CpuWrite(hl, (temp >> 1));

			SetFlag(Z, CpuRead(hl) == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 16;

		case 0x3F: // SRL A

			str_opcode = "SRL A";

			SetFlag(C, GetA() & 0x01);

			LoadA(GetA() >> 1);

			SetFlag(Z, GetA() == 0);
			SetFlag(N, 0);
			SetFlag(H, 0);

			Prefix_CB = false;

			return 8;

		case 0x40: // BIT 0,B

			str_opcode = "BIT 0,B";

			SetFlag(Z, (GetB() & 0x01) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x41: // BIT 0,C

			str_opcode = "BIT 0,C";

			SetFlag(Z, (GetC() & 0x01) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x42: // BIT 0,D

			str_opcode = "BIT 0,D";

			SetFlag(Z, (GetD() & 0x01) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x43: // BIT 0,E

			str_opcode = "BIT 0,E";

			SetFlag(Z, (GetE() & 0x01) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x44: // BIT 0,H

			str_opcode = "BIT 0,H";

			SetFlag(Z, (GetH() & 0x01) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x45: // BIT 0,L

			str_opcode = "BIT 0,L";

			SetFlag(Z, (GetL() & 0x01) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x46: // BIT 0,(HL)

			str_opcode = "BIT 0,(HL)";

			SetFlag(Z, (CpuRead(hl) & 0x01) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 16;

		case 0x47: // BIT 0,A

			str_opcode = "BIT 0,A";

			SetFlag(Z, (GetA() & 0x01) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x48: // BIT 1,B

			str_opcode = "BIT 1,B";

			SetFlag(Z, (GetB() & 0x02) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x49: // BIT 1,C

			str_opcode = "BIT 1,C";

			SetFlag(Z, (GetC() & 0x02) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x4A: // BIT 1,D

			str_opcode = "BIT 1,D";

			SetFlag(Z, (GetD() & 0x02) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x4B: // BIT 1,E

			str_opcode = "BIT 1,E";

			SetFlag(Z, (GetE() & 0x02) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x4C: // BIT 1,H

			str_opcode = "BIT 1,H";

			SetFlag(Z, (GetH() & 0x02) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x4D: // BIT 1,L

			str_opcode = "BIT 1,L";

			SetFlag(Z, (GetL() & 0x02) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x4E: // BIT 1,(HL)

			str_opcode = "BIT 1,(HL)";

			SetFlag(Z, (CpuRead(hl) & 0x02) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 16;

		case 0x4F: // BIT 1,A

			str_opcode = "BIT 1,A";

			SetFlag(Z, (GetA() & 0x02) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x50: // BIT 2,B

			str_opcode = "BIT 2,B";

			SetFlag(Z, (GetB() & 0x04) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x51: // BIT 2,C

			str_opcode = "BIT 2,C";

			SetFlag(Z, (GetC() & 0x04) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x52: // BIT 2,D

			str_opcode = "BIT 2,D";

			SetFlag(Z, (GetD() & 0x04) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x53: // BIT 2,E

			str_opcode = "BIT 2,E";

			SetFlag(Z, (GetE() & 0x04) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x54: // BIT 2,H

			str_opcode = "BIT 2,H";

			SetFlag(Z, (GetH() & 0x04) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x55: // BIT 2,L

			str_opcode = "BIT 2,L";

			SetFlag(Z, (GetL() & 0x04) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x56: // BIT 2,(HL)

			str_opcode = "BIT 2,(HL)";

			SetFlag(Z, (CpuRead(hl) & 0x04) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 16;

		case 0x57: // BIT 2,A

			str_opcode = "BIT 2,A";

			SetFlag(Z, (GetA() & 0x04) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x58: // BIT 3,B

			str_opcode = "BIT 3,B";

			SetFlag(Z, (GetB() & 0x08) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x59: // BIT 3,C

			str_opcode = "BIT 3,C";

			SetFlag(Z, (GetC() & 0x08) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x5A: // BIT 3,D

			str_opcode = "BIT 3,D";

			SetFlag(Z, (GetD() & 0x08) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x5B: // BIT 3,E

			str_opcode = "BIT 3,E";

			SetFlag(Z, (GetE() & 0x08) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x5C: // BIT 3,H

			str_opcode = "BIT 3,H";

			SetFlag(Z, (GetH() & 0x08) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x5D: // BIT 3,L

			str_opcode = "BIT 3,L";

			SetFlag(Z, (GetL() & 0x08) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x5E: // BIT 3,(HL)

			str_opcode = "BIT 3,(HL)";

			SetFlag(Z, (CpuRead(hl) & 0x08) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 16;

		case 0x5F: // BIT 3,A

			str_opcode = "BIT 3,A";

			SetFlag(Z, (GetA() & 0x08) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x60: // BIT 4,B

			str_opcode = "BIT 4,B";

			SetFlag(Z, (GetB() & 0x10) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x61: // BIT 4,C

			str_opcode = "BIT 4,C";

			SetFlag(Z, (GetC() & 0x10) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x62: // BIT 4,D

			str_opcode = "BIT 4,D";

			SetFlag(Z, (GetD() & 0x10) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x63: // BIT 4,E

			str_opcode = "BIT 4,E";

			SetFlag(Z, (GetE() & 0x10) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x64: // BIT 4,H

			str_opcode = "BIT 4,H";

			SetFlag(Z, (GetH() & 0x10) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x65: // BIT 4,L

			str_opcode = "BIT 4,L";

			SetFlag(Z, (GetL() & 0x10) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x66: // BIT 4,(HL)

			str_opcode = "BIT 4,(HL)";

			SetFlag(Z, (CpuRead(hl) & 0x10) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 16;

		case 0x67: // BIT 4,A

			str_opcode = "BIT 4,A";

			SetFlag(Z, (GetA() & 0x10) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x68: // BIT 5,B

			str_opcode = "BIT 5,B";

			SetFlag(Z, (GetB() & 0x20) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x69: // BIT 5,C

			str_opcode = "BIT 5,C";

			SetFlag(Z, (GetC() & 0x20) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x6A: // BIT 5,D

			str_opcode = "BIT 5,D";

			SetFlag(Z, (GetD() & 0x20) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x6B: // BIT 5,E

			str_opcode = "BIT 5,E";

			SetFlag(Z, (GetE() & 0x20) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x6C: // BIT 5,H

			str_opcode = "BIT 5,H";

			SetFlag(Z, (GetH() & 0x20) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x6D: // BIT 5,L

			str_opcode = "BIT 5,L";

			SetFlag(Z, (GetL() & 0x20) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x6E: // BIT 5,(HL)

			str_opcode = "BIT 5,(HL)";

			SetFlag(Z, (CpuRead(hl) & 0x20) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 16;

		case 0x6F: // BIT 5,A

			str_opcode = "BIT 5,A";

			SetFlag(Z, (GetA() & 0x20) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x70: // BIT 6,B

			str_opcode = "BIT 6,B";

			SetFlag(Z, (GetB() & 0x40) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x71: // BIT 6,C

			str_opcode = "BIT 6,C";

			SetFlag(Z, (GetC() & 0x40) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x72: // BIT 6,D

			str_opcode = "BIT 6,D";

			SetFlag(Z, (GetD() & 0x40) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x73: // BIT 6,E

			str_opcode = "BIT 6,E";

			SetFlag(Z, (GetE() & 0x40) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x74: // BIT 6,H

			str_opcode = "BIT 6,H";

			SetFlag(Z, (GetH() & 0x40) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x75: // BIT 6,L

			str_opcode = "BIT 6,L";

			SetFlag(Z, (GetL() & 0x40) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x76: // BIT 6,(HL)

			str_opcode = "BIT 6,(HL)";

			SetFlag(Z, (CpuRead(hl) & 0x40) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 16;

		case 0x77: // BIT 6,A

			str_opcode = "BIT 6,A";

			SetFlag(Z, (GetA() & 0x40) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x78: // BIT 7,B

			str_opcode = "BIT 7,B";

			SetFlag(Z, (GetB() & 0x80) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x79: // BIT 7,C

			str_opcode = "BIT 7,C";

			SetFlag(Z, (GetC() & 0x80) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x7A: // BIT 7,D

			str_opcode = "BIT 7,D";

			SetFlag(Z, (GetD() & 0x80) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x7B: // BIT 7,E

			str_opcode = "BIT 7,E";

			SetFlag(Z, (GetE() & 0x80) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x7C: // BIT 7,H

			str_opcode = "BIT 7,H";

			SetFlag(Z, (GetH() & 0x80) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x7D: // BIT 7,L

			str_opcode = "BIT 7,L";

			SetFlag(Z, (GetL() & 0x80) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x7E: // BIT 7,(HL)

			str_opcode = "BIT 7,(HL)";

			SetFlag(Z, (CpuRead(hl) & 0x80) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 16;

		case 0x7F: // BIT 7,A

			str_opcode = "BIT 7,A";

			SetFlag(Z, (GetA() & 0x80) == 0x00);
			SetFlag(N, 0);
			SetFlag(H, 1);

			Prefix_CB = false;

			return 8;

		case 0x80: // RES 0,B

			str_opcode = "RES 0,B";

			LoadB(GetB() & 0xFE);

			Prefix_CB = false;

			return 8;

		case 0x81: // RES 0,C

			str_opcode = "RES 0,C";

			LoadC(GetC() & 0xFE);

			Prefix_CB = false;

			return 8;

		case 0x82: // RES 0,D

			str_opcode = "RES 0,D";

			LoadD(GetD() & 0xFE);

			Prefix_CB = false;

			return 8;

		case 0x83: // RES 0,E

			str_opcode = "RES 0,E";

			LoadE(GetE() & 0xFE);

			Prefix_CB = false;

			return 8;

		case 0x84: // RES 0,H

			str_opcode = "RES 0,H";

			LoadH(GetH() & 0xFE);

			Prefix_CB = false;

			return 8;

		case 0x85: // RES 0,L

			str_opcode = "RES 0,L";

			LoadL(GetL() & 0xFE);

			Prefix_CB = false;

			return 8;

		case 0x86: // RES 0,(HL)

			str_opcode = "RES 0,(HL)";

			CpuWrite(hl, CpuRead(hl) & 0xFE);

			Prefix_CB = false;

			return 16;

		case 0x87: // RES 0,A

			str_opcode = "RES 0,A";

			LoadA(GetA() & 0xFE);

			Prefix_CB = false;

			return 8;

		case 0x88: // RES 1,B

			str_opcode = "RES 1,B";

			LoadB(GetB() & 0xFD);

			Prefix_CB = false;

			return 8;

		case 0x89: // RES 1,C

			str_opcode = "RES 1,C";

			LoadC(GetC() & 0xFD);

			Prefix_CB = false;

			return 8;

		case 0x8A: // RES 1,D

			str_opcode = "RES 1,D";

			LoadD(GetD() & 0xFD);

			Prefix_CB = false;

			return 8;

		case 0x8B: // RES 1,E

			str_opcode = "RES 1,E";

			LoadE(GetE() & 0xFD);

			Prefix_CB = false;

			return 8;

		case 0x8C: // RES 1,H

			str_opcode = "RES 1,H";

			LoadH(GetH() & 0xFD);

			Prefix_CB = false;

			return 8;

		case 0x8D: // RES 1,L

			str_opcode = "RES 1,L";

			LoadL(GetL() & 0xFD);

			Prefix_CB = false;

			return 8;

		case 0x8E: // RES 1,(HL)

			str_opcode = "RES 0,(HL)";

			CpuWrite(hl, CpuRead(hl) & 0xFD);

			Prefix_CB = false;

			return 16;

		case 0x8F: // RES 1,A

			str_opcode = "RES 1,A";

			LoadA(GetA() & 0xFD);

			Prefix_CB = false;

			return 8;

		case 0x90: // RES 2,B

			str_opcode = "RES 2,B";

			LoadB(GetB() & 0xFB);

			Prefix_CB = false;

			return 8;

		case 0x91: // RES 2,C

			str_opcode = "RES 2,C";

			LoadC(GetC() & 0xFB);

			Prefix_CB = false;

			return 8;

		case 0x92: // RES 2,D

			str_opcode = "RES 2,D";

			LoadD(GetD() & 0xFB);

			Prefix_CB = false;

			return 8;

		case 0x93: // RES 2,E

			str_opcode = "RES 2,E";

			LoadE(GetE() & 0xFB);

			Prefix_CB = false;

			return 8;

		case 0x94: // RES 2,H

			str_opcode = "RES 2,H";

			LoadH(GetH() & 0xFB);

			Prefix_CB = false;

			return 8;

		case 0x95: // RES 2,L

			str_opcode = "RES 2,L";

			LoadL(GetL() & 0xFB);

			Prefix_CB = false;

			return 8;

		case 0x96: // RES 2,(HL)

			str_opcode = "RES 2,(HL)";

			CpuWrite(hl, CpuRead(hl) & 0xFB);

			Prefix_CB = false;

			return 16;

		case 0x97: // RES 2,A

			str_opcode = "RES 2,A";

			LoadA(GetA() & 0xFB);

			Prefix_CB = false;

			return 8;

		case 0x98: // RES 3,B

			str_opcode = "RES 3,B";

			LoadB(GetB() & 0xF7);

			Prefix_CB = false;

			return 8;

		case 0x99: // RES 3,C

			str_opcode = "RES 3,C";

			LoadC(GetC() & 0xF7);

			Prefix_CB = false;

			return 8;

		case 0x9A: // RES 3,D

			str_opcode = "RES 3,D";

			LoadD(GetD() & 0xF7);

			Prefix_CB = false;

			return 8;

		case 0x9B: // RES 3,E

			str_opcode = "RES 3,E";

			LoadE(GetE() & 0xF7);

			Prefix_CB = false;

			return 8;

		case 0x9C: // RES 3,H

			str_opcode = "RES 3,H";

			LoadH(GetH() & 0xF7);

			Prefix_CB = false;

			return 8;

		case 0x9D: // RES 3,L

			str_opcode = "RES 3,L";

			LoadL(GetL() & 0xF7);

			Prefix_CB = false;

			return 8;

		case 0x9E: // RES 3,(HL)

			str_opcode = "RES 3,(HL)";

			CpuWrite(hl, CpuRead(hl) & 0xF7);

			Prefix_CB = false;

			return 16;

		case 0x9F: // RES 3,A

			str_opcode = "RES 3,A";

			LoadA(GetA() & 0xF7);

			Prefix_CB = false;

			return 8;

		case 0xA0: // RES 4,B

			str_opcode = "RES 4,B";

			LoadB(GetB() & 0xEF);

			Prefix_CB = false;

			return 8;

		case 0xA1: // RES 4,C

			str_opcode = "RES 4,C";

			LoadC(GetC() & 0xEF);

			Prefix_CB = false;

			return 8;

		case 0xA2: // RES 4,D

			str_opcode = "RES 4,D";

			LoadD(GetD() & 0xEF);

			Prefix_CB = false;

			return 8;

		case 0xA3: // RES 4,E

			str_opcode = "RES 4,E";

			LoadE(GetE() & 0xEF);

			Prefix_CB = false;

			return 8;

		case 0xA4: // RES 4,H

			str_opcode = "RES 4,H";

			LoadH(GetH() & 0xEF);

			Prefix_CB = false;

			return 8;

		case 0xA5: // RES 4,L

			str_opcode = "RES 4,L";

			LoadL(GetL() & 0xEF);

			Prefix_CB = false;

			return 8;

		case 0xA6: // RES 4,(HL)

			str_opcode = "RES 4,(HL)";

			CpuWrite(hl, CpuRead(hl) & 0xEF);

			Prefix_CB = false;

			return 16;

		case 0xA7: // RES 4,A

			str_opcode = "RES 4,A";

			LoadA(GetA() & 0xEF);

			Prefix_CB = false;

			return 8;

		case 0xA8: // RES 5,B

			str_opcode = "RES 5,B";

			LoadB(GetB() & 0xDF);

			Prefix_CB = false;

			return 8;

		case 0xA9: // RES 5,C

			str_opcode = "RES 5,C";

			LoadC(GetC() & 0xDF);

			Prefix_CB = false;

			return 8;

		case 0xAA: // RES 5,D

			str_opcode = "RES 5,D";

			LoadD(GetD() & 0xDF);

			Prefix_CB = false;

			return 8;

		case 0xAB: // RES 5,E

			str_opcode = "RES 5,E";

			LoadE(GetE() & 0xDF);

			Prefix_CB = false;

			return 8;

		case 0xAC: // RES 5,H

			str_opcode = "RES 5,H";

			LoadH(GetH() & 0xDF);

			Prefix_CB = false;

			return 8;

		case 0xAD: // RES 5,L

			str_opcode = "RES 5,L";

			LoadL(GetL() & 0xDF);

			Prefix_CB = false;

			return 8;

		case 0xAE: // RES 5,(HL)

			str_opcode = "RES 5,(HL)";

			CpuWrite(hl, CpuRead(hl) & 0xDF);

			Prefix_CB = false;

			return 16;

		case 0xAF: // RES 5,A

			str_opcode = "RES 5,A";

			LoadA(GetA() & 0xDF);

			Prefix_CB = false;

			return 8;

		case 0xB0: // RES 6,B

			str_opcode = "RES 6,B";

			LoadB(GetB() & 0xBF);

			Prefix_CB = false;

			return 8;

		case 0xB1: // RES 6,C

			str_opcode = "RES 6,C";

			LoadC(GetC() & 0xBF);

			Prefix_CB = false;

			return 8;

		case 0xB2: // RES 6,D

			str_opcode = "RES 6,D";

			LoadD(GetD() & 0xBF);

			Prefix_CB = false;

			return 8;

		case 0xB3: // RES 6,E

			str_opcode = "RES 6,E";

			LoadE(GetE() & 0xBF);

			Prefix_CB = false;

			return 8;

		case 0xB4: // RES 6,H

			str_opcode = "RES 6,H";

			LoadH(GetH() & 0xBF);

			Prefix_CB = false;

			return 8;

		case 0xB5: // RES 6,L

			str_opcode = "RES 6,L";

			LoadL(GetL() & 0xBF);

			Prefix_CB = false;

			return 8;

		case 0xB6: // RES 6,(HL)

			str_opcode = "RES 6,(HL)";

			CpuWrite(hl, CpuRead(hl) & 0xBF);

			Prefix_CB = false;

			return 16;

		case 0xB7: // RES 6,A

			str_opcode = "RES 6,A";

			LoadA(GetA() & 0xBF);

			Prefix_CB = false;

			return 8;

		case 0xB8: // RES 7,B

			str_opcode = "RES 7,B";

			LoadB(GetB() & 0x7F);

			Prefix_CB = false;

			return 8;

		case 0xB9: // RES 7,C

			str_opcode = "RES 6,C";

			LoadC(GetC() & 0x7F);

			Prefix_CB = false;

			return 8;

		case 0xBA: // RES 7,D

			str_opcode = "RES 7,D";

			LoadD(GetD() & 0x7F);

			Prefix_CB = false;

			return 8;

		case 0xBB: // RES 7,E

			str_opcode = "RES 7,E";

			LoadE(GetE() & 0x7F);

			Prefix_CB = false;

			return 8;

		case 0xBC: // RES 7,H

			str_opcode = "RES 7,H";

			LoadH(GetH() & 0x7F);

			Prefix_CB = false;

			return 8;

		case 0xBD: // RES 7,L

			str_opcode = "RES 7,L";

			LoadL(GetL() & 0x7F);

			Prefix_CB = false;

			return 8;

		case 0xBE: // RES 7,(HL)

			str_opcode = "RES 7,(HL)";

			CpuWrite(hl, CpuRead(hl) & 0x7F);

			Prefix_CB = false;

			return 16;

		case 0xBF: // RES 7,A

			str_opcode = "RES 7,A";

			LoadA(GetA() & 0x7F);

			Prefix_CB = false;

			return 8;

		case 0xC0: // SET 0,B

			str_opcode = "SET 0,B";

			LoadB(GetB() | 0x01);

			Prefix_CB = false;

			return 8;

		case 0xC1: // SET 0,C

			str_opcode = "SET 0,C";

			LoadC(GetC() | 0x01);

			Prefix_CB = false;

			return 8;

		case 0xC2: // SET 0,D

			str_opcode = "SET 0,D";

			LoadD(GetD() | 0x01);

			Prefix_CB = false;

			return 8;

		case 0xC3: // SET 0,E

			str_opcode = "SET 0,E";

			LoadE(GetE() | 0x01);

			Prefix_CB = false;

			return 8;

		case 0xC4: // SET 0,H

			str_opcode = "SET 0,H";

			LoadH(GetH() | 0x01);

			Prefix_CB = false;

			return 8;

		case 0xC5: // SET 0,L

			str_opcode = "SET 0,L";

			LoadL(GetL() | 0x01);

			Prefix_CB = false;

			return 8;

		case 0xC6: // SET 0,(HL)

			str_opcode = "SET 0,(HL)";

			CpuWrite(hl, (CpuRead(hl) | 0x01));

			Prefix_CB = false;

			return 16;

		case 0xC7: // SET 0,A

			str_opcode = "SET 0,A";

			LoadA(GetA() | 0x01);

			Prefix_CB = false;

			return 8;

		case 0xC8: // SET 1,B

			str_opcode = "SET 1,B";

			LoadB(GetB() | 0x02);

			Prefix_CB = false;

			return 8;

		case 0xC9: // SET 1,C

			str_opcode = "SET 1,C";

			LoadC(GetC() | 0x02);

			Prefix_CB = false;

			return 8;

		case 0xCA: // SET 1,D

			str_opcode = "SET 1,D";

			LoadD(GetD() | 0x02);

			Prefix_CB = false;

			return 8;

		case 0xCB: // SET 1,E

			str_opcode = "SET 1,E";

			LoadE(GetE() | 0x02);

			Prefix_CB = false;

			return 8;

		case 0xCC: // SET 1,H

			str_opcode = "SET 1,H";

			LoadH(GetH() | 0x02);

			Prefix_CB = false;

			return 8;

		case 0xCD: // SET 1,L

			str_opcode = "SET 1,L";

			LoadL(GetL() | 0x02);

			Prefix_CB = false;

			return 8;

		case 0xCE: // SET 1,(HL)

			str_opcode = "SET 1,(HL)";

			CpuWrite(hl, (CpuRead(hl) | 0x02));

			Prefix_CB = false;

			return 16;

		case 0xCF: // SET 1,A

			str_opcode = "SET 1,A";

			LoadA(GetA() | 0x02);

			Prefix_CB = false;

			return 8;

		case 0xD0: // SET 2,B

			str_opcode = "SET 2,B";

			LoadB(GetB() | 0x04);

			Prefix_CB = false;

			return 8;

		case 0xD1: // SET 2,C

			str_opcode = "SET 2,C";

			LoadC(GetC() | 0x04);

			Prefix_CB = false;

			return 8;

		case 0xD2: // SET 2,D

			str_opcode = "SET 2,D";

			LoadD(GetD() | 0x04);

			Prefix_CB = false;

			return 8;

		case 0xD3: // SET 2,E

			str_opcode = "SET 2,E";

			LoadE(GetE() | 0x04);

			Prefix_CB = false;

			return 8;

		case 0xD4: // SET 2,H

			str_opcode = "SET 2,H";

			LoadH(GetH() | 0x04);

			Prefix_CB = false;

			return 8;

		case 0xD5: // SET 2,L

			str_opcode = "SET 2,L";

			LoadL(GetL() | 0x04);

			Prefix_CB = false;

			return 8;

		case 0xD6: // SET 2,(HL)

			str_opcode = "SET 2,(HL)";

			CpuWrite(hl, (CpuRead(hl) | 0x04));

			Prefix_CB = false;

			return 16;

		case 0xD7: // SET 2,A

			str_opcode = "SET 2,A";

			LoadA(GetA() | 0x04);

			Prefix_CB = false;

			return 8;

		case 0xD8: // SET 3,B

			str_opcode = "SET 3,B";

			LoadB(GetB() | 0x08);

			Prefix_CB = false;

			return 8;

		case 0xD9: // SET 4,C

			str_opcode = "SET 4,C";

			LoadC(GetC() | 0x08);

			Prefix_CB = false;

			return 8;

		case 0xDA: // SET 3,D

			str_opcode = "SET 3,D";

			LoadD(GetD() | 0x08);

			Prefix_CB = false;

			return 8;

		case 0xDB: // SET 3,E

			str_opcode = "SET 3,E";

			LoadE(GetE() | 0x08);

			Prefix_CB = false;

			return 8;

		case 0xDC: // SET 3,H

			str_opcode = "SET 3,H";

			LoadH(GetH() | 0x08);

			Prefix_CB = false;

			return 8;

		case 0xDD: // SET 3,L

			str_opcode = "SET 3,L";

			LoadL(GetL() | 0x08);

			Prefix_CB = false;

			return 8;

		case 0xDE: // SET 3,(HL)

			str_opcode = "SET 0,(HL)";

			CpuWrite(hl, (CpuRead(hl) | 0x08));

			Prefix_CB = false;

			return 16;

		case 0xDF: // SET 3,A

			str_opcode = "SET 3,A";

			LoadA(GetA() | 0x08);

			Prefix_CB = false;

			return 8;

		case 0xE0: // SET 4,B

			str_opcode = "SET 4,B";

			LoadB(GetB() | 0x10);

			Prefix_CB = false;

			return 8;

		case 0xE1: // SET 4,C

			str_opcode = "SET 4,C";

			LoadC(GetC() | 0x10);

			Prefix_CB = false;

			return 8;

		case 0xE2: // SET 4,D

			str_opcode = "SET 4,D";

			LoadD(GetD() | 0x10);

			Prefix_CB = false;

			return 8;

		case 0xE3: // SET 4,E

			str_opcode = "SET 4,E";

			LoadE(GetE() | 0x10);

			Prefix_CB = false;

			return 8;

		case 0xE4: // SET 4,H

			str_opcode = "SET 4,H";

			LoadH(GetH() | 0x10);

			Prefix_CB = false;

			return 8;

		case 0xE5: // SET 4,L

			str_opcode = "SET 4,L";

			LoadL(GetL() | 0x10);

			Prefix_CB = false;

			return 8;

		case 0xE6: // SET 4,(HL)

			str_opcode = "SET 4,(HL)";

			CpuWrite(hl, (CpuRead(hl) | 0x10));

			Prefix_CB = false;

			return 16;

		case 0xE7: // SET 4,A

			str_opcode = "SET 4,A";

			LoadA(GetA() | 0x10);

			Prefix_CB = false;

			return 8;

		case 0xE8: // SET 5,B

			str_opcode = "SET 5,B";

			LoadB(GetB() | 0x20);

			Prefix_CB = false;

			return 8;

		case 0xE9: // SET 5,C

			str_opcode = "SET 5,C";

			LoadC(GetC() | 0x20);

			Prefix_CB = false;

			return 8;

		case 0xEA: // SET 5,D

			str_opcode = "SET 5,D";

			LoadD(GetD() | 0x20);

			Prefix_CB = false;

			return 8;

		case 0xEB: // SET 5,E

			str_opcode = "SET 5,E";

			LoadE(GetE() | 0x20);

			Prefix_CB = false;

			return 8;

		case 0xEC: // SET 5,H

			str_opcode = "SET 5,H";

			LoadH(GetH() | 0x20);

			Prefix_CB = false;

			return 8;

		case 0xED: // SET 5,L

			str_opcode = "SET 5,L";

			LoadL(GetL() | 0x20);

			Prefix_CB = false;

			return 8;

		case 0xEE: // SET 5,(HL)

			str_opcode = "SET 5,(HL)";

			CpuWrite(hl, (CpuRead(hl) | 0x20));

			Prefix_CB = false;

			return 16;

		case 0xEF: // SET 5,A

			str_opcode = "SET 5,A";

			LoadA(GetA() | 0x20);

			Prefix_CB = false;

			return 8;

		case 0xF0: // SET 6,B

			str_opcode = "SET 6,B";

			LoadB(GetB() | 0x40);

			Prefix_CB = false;

			return 8;

		case 0xF1: // SET 6,C

			str_opcode = "SET 6,C";

			LoadC(GetC() | 0x40);

			Prefix_CB = false;

			return 8;

		case 0xF2: // SET 6,D

			str_opcode = "SET 6,D";

			LoadD(GetD() | 0x40);

			Prefix_CB = false;

			return 8;

		case 0xF3: // SET 6,E

			str_opcode = "SET 6,E";

			LoadE(GetE() | 0x40);

			Prefix_CB = false;

			return 8;

		case 0xF4: // SET 6,H

			str_opcode = "SET 6,H";

			LoadH(GetH() | 0x40);

			Prefix_CB = false;

			return 8;

		case 0xF5: // SET 6,L

			str_opcode = "SET 6,L";

			LoadL(GetL() | 0x40);

			Prefix_CB = false;

			return 8;

		case 0xF6: // SET 6,(HL)

			str_opcode = "SET 6,(HL)";

			CpuWrite(hl, (CpuRead(hl) | 0x40));

			Prefix_CB = false;

			return 16;

		case 0xF7: // SET 6,A

			str_opcode = "SET 6,A";

			LoadA(GetA() | 0x40);

			Prefix_CB = false;

			return 8;

		case 0xF8: // SET 7,B

			str_opcode = "SET 7,B";

			LoadB(GetB() | 0x80);

			Prefix_CB = false;

			return 8;

		case 0xF9: // SET 7,C

			str_opcode = "SET 7,C";

			LoadC(GetC() | 0x80);

			Prefix_CB = false;

			return 8;

		case 0xFA: // SET 7,D

			str_opcode = "SET 7,D";

			LoadD(GetD() | 0x80);

			Prefix_CB = false;

			return 8;

		case 0xFB: // SET 7,E

			str_opcode = "SET 7,E";

			LoadE(GetE() | 0x80);

			Prefix_CB = false;

			return 8;

		case 0xFC: // SET 7,H

			str_opcode = "SET 7,H";

			LoadH(GetH() | 0x80);

			Prefix_CB = false;

			return 8;

		case 0xFD: // SET 7,L

			str_opcode = "SET 7,L";

			LoadL(GetL() | 0x80);

			Prefix_CB = false;

			return 8;

		case 0xFE: // SET 7,(HL)

			str_opcode = "SET 7,(HL)";

			CpuWrite(hl, CpuRead(hl) | 0x80);

			Prefix_CB = false;

			return 16;

		case 0xFF: // SET 7,A

			str_opcode = "SET 7,A";

			LoadA(GetA() | 0x80);

			Prefix_CB = false;

			return 8;

		default:
			std::cout << "The CB 0x" << std::uppercase << std::hex << opcode << " isn't implemented yet" << std::endl;
			std::cout << "PC: 0x" << std::uppercase << std::hex << pc << std::endl;
			Prefix_CB = false;
			return 0;
		}
	}
	return 0;
}