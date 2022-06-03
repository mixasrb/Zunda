#pragma once
#include <cstdint>
#include <iostream>

class BUS;

// SHARP LR35902

class CPU
{
private:

	void CpuWrite(uint16_t addr, uint8_t data);
	uint8_t CpuRead(uint16_t addr);

public:

	BUS* bus = nullptr;

	void ConnectCPU(BUS* ptr)
	{
		bus = ptr;
	}

	void start();

	void clock();

	//private:
public:

	// REGISTERS

	void LoadA(uint8_t input); // Accumulator
	void LoadF(uint8_t input);
	void LoadB(uint8_t input);
	void LoadC(uint8_t input);
	void LoadD(uint8_t input);
	void LoadE(uint8_t input);
	void LoadH(uint8_t input);
	void LoadL(uint8_t input);

	uint8_t GetA() const; // Accumulator
	uint8_t GetB() const;
	uint8_t GetC() const;
	uint8_t GetD() const;
	uint8_t GetE() const;
	uint8_t GetH() const;
	uint8_t GetL() const;

	uint8_t f; // Flag Register

	uint16_t sp; // Stack Pointer
	uint16_t pc; // Program Counter

	uint16_t af; // AF Register
	uint16_t bc; // BC Register
	uint16_t de; // DE Register
	uint16_t hl; // HL Register

	enum Flags
	{
		Z = 1 << 7,
		N = 1 << 6,
		H = 1 << 5,
		C = 1 << 4,
		U3 = 1 << 3,
		U2 = 1 << 2,
		U1 = 1 << 1,
		U0 = 1 << 0,
	};

	uint8_t cycles = 0;
	uint16_t opcode;

	// Interrupt Master Enable Flag
	bool bIME = false;

	// Adresing modes
	uint8_t d8();
	uint16_t d16();
	uint8_t a8();
	uint16_t a16();
	int8_t r8();

	uint8_t Interrupt();

	uint8_t DoInstruction(const uint16_t& opcode);

	// Helper Variables

	bool bChangeInterruptState = false;
	bool bInterruptEnableFlag = false;

	uint8_t temp;
	uint8_t temp2;
	uint16_t temp16;
	uint32_t temp32;
	uint8_t c;
	uint16_t addr;
	uint8_t pc_lo;
	uint8_t pc_hi;
	uint8_t nibble1;
	uint8_t nibble2;

	uint16_t data16;

	bool bFlag = false;
	bool bStop = false;
	bool bHALT = false;
	bool bHaltBug = false;
	bool Prefix_CB = false;

	void SetFlag(Flags F, bool condition);
	uint8_t GetFlag(Flags F) const;

	// Debugging Staff

public:

	bool InstructionDone = false;
	std::string str_opcode;
};