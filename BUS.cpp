#include "BUS.h"

BUS::BUS()
{
	cpu.ConnectCPU(this);
	ppu.ConnectPPU(this);

	/*
	for (int i = 0; i < 8192; i++)
	{
		ram[i] = rand() % 256;
	}

	for (int i = 0; i < 127; i++)
	{
		hram[i] = rand() % 256;
	}
	*/
}

void BUS::InsertCartridge(const char* name)
{
	cart.cart_name = name;
}

uint8_t BUS::CpuRead(uint16_t addr)
{
	uint8_t data = 0xFF;

	if (addr >= 0x0000 && addr <= 0x7FFF)
	{
		if (BOOT == 0x01)
		{
			// Cartridge
			if (addr >= 0x0000 && addr <= 0x7FFF)
			{
				cart.CpuRead(addr, data);
			}
		}
		else
		{
			// BOOT ROM
			if (addr >= 0x0000 && addr <= 0x00FF)
			{
				data = boot_rom.bios_data[addr];
			}
			// Cartridge
			else if (addr >= 0x0100 && addr <= 0x7FFF)
			{
				cart.CpuRead(addr, data);
			}
		}
	}
	// Video RAM
	else if (addr >= 0x8000 && addr <= 0x9FFF)   // && !ppu.bVramCpuBlock)
	{
		data = vram[addr & 0x1FFF];
	}
	// Cart Ram
	else if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		cart.CpuRead(addr, data);
	}
	// RAM
	else if (addr >= 0xC000 && addr <= 0xFDFF)
	{
		data = ram[addr & 0x1FFF];
	}
	// OAM
	else if (addr >= 0xFE00 && addr <= 0xFE9F) //&& !ppu.bOAMCpuBlock)
	{
		data = OAM[addr & 0x00FF];
	}
	// P1 (Controller)
	else if (addr == 0xFF00)
	{
		data = p1;
		//data = 0xff;
	}
	// Serial transfer data
	else if (addr == 0xFF01)
	{
		if (SC & 0x80)
			data = 0xFF;
		else
			data = SB;
	}
	// S10 Control
	else if (addr == 0xFF02)
	{
		data = SC;
	}
	// Divider Register
	else if (addr == 0xFF04)
	{
		data = DIV;
	}
	// Timer Counter
	else if (addr == 0xFF05)
	{
		data = TIMA;
	}
	// Timer Modulo
	else if (addr == 0xFF06)
	{
		data = TMA;
	}
	// Timer Control
	else if (addr == 0xFF07)
	{
		data = TAC;
	}
	// Interrupt Flag 
	else if (addr == 0xFF0F)
	{
		data = IF.data | 0xE0;
	}
	// APU Registers
	else if ((addr >= 0xFF10 && addr <= 0xFF14) ||
		(addr >= 0xFF16 && addr <= 0xFF1E) ||
		(addr >= 0xFF20 && addr <= 0xFF26) ||
		(addr >= 0xFF30 && addr <= 0xFF3F))
	{
		data = apu.Read(addr);
	}
	// PPU Registers
	else if (addr >= 0xFF40 && addr <= 0xFF4B)
	{
		data = ppu.CpuRead(addr);
	}
	// Boot Turn Off Register
	else if (addr == 0xFF50)
	{
		data = BOOT;
	}
	// HRAM
	else if (addr >= 0xFF80 && addr <= 0xFFFE)
	{
		data = hram[addr & 0x007F];
	}
	// Interrupt Enable Register
	else if (addr == 0xFFFF)
	{
		data = IE.data | 0xE0;
	}

	return data;
}

void BUS::CpuWrite(uint16_t addr, uint8_t data)
{
	if (addr >= 0x0000 && addr <= 0x7FFF)
	{
		cart.CpuWrite(addr, data);
	}
	// Video RAM
	else if (addr >= 0x8000 && addr <= 0x9FFF) // && !ppu.bVramCpuBlock)
	{
		vram[addr & 0x1FFF] = data;
	}
	// Cart Ram
	else if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		cart.CpuWrite(addr, data);
	}
	// RAM
	else if (addr >= 0xC000 && addr <= 0xFDFF)
	{
		ram[addr & 0x1FFF] = data;
	}
	// OAM
	else if (addr >= 0xFE00 && addr <= 0xFE9F) // && !ppu.bOAMCpuBlock)
	{
		OAM[addr & 0x00FF] = data;
	}
	// P1 (Controller)
	else if (addr == 0xFF00)
	{
		p1 = data | 0xC0;
	}
	// Serial transfer data
	else if (addr == 0xFF01)
	{
		SB = data;

		if (SC & 0x80)
			std::cout << SB;
	}
	// S10 Control
	else if (addr == 0xFF02)
	{
		SC = data;
		///IF.Serial_I_O = 1; I should implement that
	}
	// Divider Register
	else if (addr == 0xFF04)
	{
		divider = 0;
	}
	// Timer Counter
	else if (addr == 0xFF05)
	{
		TIMA = data;
	}
	// Timer Modulo
	else if (addr == 0xFF06)
	{
		TMA = data;
	}
	// Timer Control
	else if (addr == 0xFF07)
	{
		TAC = data;

		switch (TAC & 0x03)
		{
		case 0x00:
			tPeriod = 0x0200;
			break;
		case 0x01:
			tPeriod = 0x0008;
			break;
		case 0x02:
			tPeriod = 0x0020;
			break;
		case 0x03:
			tPeriod = 0x0080;
			break;
		}
	}
	// Interrupt Flag 
	else if (addr == 0xFF0F)
	{
		IF.data = data | 0xE0;
	}
	// APU Registers
	else if ((addr >= 0xFF10 && addr <= 0xFF14) ||
		(addr >= 0xFF16 && addr <= 0xFF1E) ||
		(addr >= 0xFF20 && addr <= 0xFF26) ||
		(addr >= 0xFF30 && addr <= 0xFF3F))
	{
		apu.Write(addr, data);
	}
	// PPU Registers
	else if (addr >= 0xFF40 && addr <= 0xFF4B)
	{
		ppu.CpuWrite(addr, data);
	}
	// Boot Turn Off Register
	else if (addr == 0xFF50)
	{
		BOOT = data;
	}
	// HRAM
	else if (addr >= 0xFF80 && addr <= 0xFFFE)
	{
		hram[addr & 0x007F] = data;
	}
	// Interrupt Enable Register
	else if (addr == 0xFFFF)
	{
		IE.data = data | 0xE0;
	}
}

uint8_t BUS::PpuRead(uint16_t addr)
{
	uint8_t data = 0x00;

	data = vram[addr & 0x1FFF];

	return data;
}

void BUS::start()
{
	cpu.start();
	cart.ReadCart();
}

bool BUS::Clock()
{

	if (!cpu.bHALT)
		cpu.clock();

	if (cpu.bHALT)
		if (IF.data & IE.data & 0x1F)
		{
			cpu.bHALT = false;

			// Halt Bug
			if (!cpu.bIME)
				cpu.bHaltBug = true;
		}

	if (cpu.bStop)
		if (bButtonPressed)
			cpu.bStop = false;

	ppu.clock();

	fSample = apu.Clock();

	fTime += 1.0f / (114.0f * 154.0f) / 59.7f;
	if (fTime >= 1.0f / 44100.0f)
	{
		fTime -= 1.0f / 44100.0f;
		bSampleReady = true;
	}
	else
		bSampleReady = false;


	//Timer
	if (bOverflow)
	{
		bOverflow = false;
		TIMA = TMA;
		IF.Timer_Overflow = 1;
	}


	divider += 4;
	DIV = divider >> 8;
	if (TAC & 0x04)
	{
		if ((bLast) && !(divider & tPeriod))
		{
			TIMA++;
			if (TIMA == 0)
			{
				bOverflow = true;
			}

		}
		bLast = divider & tPeriod;
	}


	serial += 4;
	if ((SC & 0x01) && (SC & 0x80))
	{
		if ((bLast_serial) && !(serial & 0x80))
		{
			SB << 1;
			c++;
			if (c == 7)
			{
				c = 0;
				SC &= 0xFE;
				IF.Serial_I_O = 1;
			}
		}
	}
	else
		c = 0;

	bLast_serial = serial & 0x80;

	// DMA Transfer
	if (ppu.bDMA)
	{
		uint8_t temp = CpuRead(ppu.DMA * 0x0100 + DMA_Clocks);
		CpuWrite(0xFE00 + DMA_Clocks, temp);
		DMA_Clocks++;
	}
	if (DMA_Clocks == 160)
	{
		DMA_Clocks = 0;
		ppu.bDMA = false;
	}



	bButtonPressed = false;

#if LOG
	log.TraceLog(cpu.str_opcode, ppu.cycles, ppu.lines, SystemCycleCounter);
#endif

	SystemCycleCounter++;
	return bSampleReady;
	}