#include "BootRom.h"

BootRom::BootRom()
{
	std::ifstream boot_rom;

	boot_rom.open("BIOS\\[BIOS] Nintendo Game Boy Boot ROM.gb", std::ifstream::binary);
	if (boot_rom.is_open())
	{
		boot_rom.read((char*)bios_data, 0x100);
		boot_rom.close();
	}
	else
	{
		std::cout << "Cannot open : BIOS\\[BIOS] Nintendo Game Boy Boot ROM.gb" << std::endl;
		system("Pause");
	}
}