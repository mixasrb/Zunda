#include "Cartridge.h"

void Cartridge::ReadCart()
{
	std::ifstream cart;
	cart.open(cart_name, std::ifstream::binary);
	if (cart.is_open())
	{
		rom.resize(0x0150);
		cart.read((char*)reserved, 0x0150);
		memcpy(rom.data(), reserved, 0x0150);
		memcpy(&m_header, reserved + 0x0134, sizeof(header));

		char cart_name[16];
		memcpy(cart_name, m_header.title, sizeof(m_header.title));
		cart_name[15] = '\0';


		std::cout << "Title : " << cart_name << std::endl;
		std::cout << "Color : " << (uint16_t)m_header.color_GB << std::endl;
		std::cout << "Licence Code : 0x" << std::uppercase << std::hex << (uint16_t)m_header.licence_code << std::endl;
		std::cout << "Indicator : " << (uint16_t)m_header.indicator << std::endl;
		std::cout << "Cart Type : 0x" << (uint16_t)m_header.cart_type << std::uppercase << std::hex << std::endl;
		std::cout << "ROM Size : 0x" << std::uppercase << std::hex << (uint16_t)m_header.rom_size << std::endl;
		std::cout << "RAM Size : 0x" << std::uppercase << std::hex << (uint16_t)m_header.ram_size << std::endl;
		std::cout << "Not Japan : " << m_header.destination_code << std::endl;
		std::cout << "Old licence Code : 0x" << std::uppercase << std::hex << (uint16_t)m_header.old_licence_code << std::endl;
		std::cout << "Version : " << (uint16_t)m_header.mask_rom_number << std::endl;

		switch (m_header.cart_type)
		{
		case 0x00: // ROM_ONLY

			*mbc = new ROM_ONLY;
			rom.resize(0x8000);
			cart.read((char*)rom.data() + 0x0150, 0x8000 - 0x0150);
			cart.close();
			break;

		case 0x01: // MBC1
		{

			*mbc = new MBC1;

			int newSize = 0x8000;
			if (m_header.rom_size >= 0 && m_header.rom_size <= 6)
				newSize = 0x4000 * pow(2, (1 + m_header.rom_size));

			rom.resize(newSize);

			switch (m_header.ram_size)
			{
			case 0x00:
				//ram.resize(4 * 2048); 
				break;
			case 0x01:
				//ram.resize(2048);
				std::cout << "ram is out of predicted size" << std::endl;
				break;
			case 0x02:
				ram.resize(4 * 2048);
				break;
			case 0x03:
				ram.resize(16 * 2048);
				break;
			default:
				std::cout << "ram is out of predicted size" << std::endl;
			}

			cart.read((char*)rom.data() + 0x0150, newSize - 0x0150);
			cart.close();
			break;
		}

		case 0x02: // MBC1  + RAM
		{
			/*
			*mbc = new MBC1;
			int newSize = 0x8000;
			if (m_header.rom_size >= 0 && m_header.rom_size <= 6)
				newSize = 0x4000 * pow(2, (1 + m_header.rom_size));

			rom.resize(newSize);

			cart.read((char*)rom.data() + 0x0150, newSize - 0x0150);
			cart.close();
			break; */

			* mbc = new MBC1;

			int newSize = 0x8000;
			if (m_header.rom_size >= 0 && m_header.rom_size <= 6)
				newSize = 0x4000 * pow(2, (1 + m_header.rom_size));

			rom.resize(newSize);

			switch (m_header.ram_size)
			{
			case 0x00:
				//ram.resize(4 * 2048);
				break;
			case 0x01:
				//ram.resize(2048);
				std::cout << "ram is out of predicted size" << std::endl;
				break;
			case 0x02:
				ram.resize(4 * 2048);
				break;
			case 0x03:
				ram.resize(16 * 2048);
				break;
			default:
				std::cout << "ram is out of predicted size" << std::endl;
			}

			cart.read((char*)rom.data() + 0x0150, newSize - 0x0150);
			cart.close();
			break;
		}
		case 0x03: // MBC1  + RAM + BATTERY
		{

			*mbc = new MBC1;

			int newSize = 0x8000;
			if (m_header.rom_size >= 0 && m_header.rom_size <= 6)
				newSize = 0x4000 * pow(2, (1 + m_header.rom_size));

			rom.resize(newSize);

			switch (m_header.ram_size)
			{
			case 0x00:
				//ram.resize(4 * 2048);
				break;
			case 0x01:
				//ram.resize(2048);
				std::cout << "ram is out of predicted size" << std::endl;
				break;
			case 0x02:
				ram.resize(4 * 2048);
				break;
			case 0x03:
				ram.resize(16 * 2048);
				break;
			default:
				std::cout << "ram is out of predicted size" << std::endl;
			}

			cart.read((char*)rom.data() + 0x0150, newSize - 0x0150);
			cart.close();
			break;
		}
		}
	}
	else
	{

		*mbc = new ROM_ONLY;

		rom.resize(0x8000);
		for (int i = 0; i < 0x8000; i++)
		{
			rom[i] = 0xFF;
		}
	}
}

void Cartridge::CpuRead(uint16_t addr, uint8_t& data)
{
	switch (m_header.cart_type)
	{
	case 0x00:
		data = (*mbc)->CpuRead(addr, rom, ram);
		break;
	case 0x01:
		data = (*mbc)->CpuRead(addr, rom, ram);
		break;
	case 0x02:
		data = (*mbc)->CpuRead(addr, rom, ram);
		break;
	case 0x03:
		data = (*mbc)->CpuRead(addr, rom, ram);
		break;
	}
}

void Cartridge::CpuWrite(uint16_t addr, uint8_t data)
{
	switch (m_header.cart_type)
	{
	case 0x00:
		// Nothing
		break;
	case 0x01:
		(*mbc)->CpuWrite(addr, data, ram);
		break;
	case 0x02:
		(*mbc)->CpuWrite(addr, data, ram);
		break;
	case 0x03:
		(*mbc)->CpuWrite(addr, data, ram);
		break;
	}
}