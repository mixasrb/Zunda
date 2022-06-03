#include "PPU.h"
#include "BUS.h"

void PPU::CpuWrite(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0xFF40: // LCD Control
		control.reg = data;

		// I need to add memory select.

		if (control.Window_Tile_Map_Address)
		{
			WIN_Map_lo = 0x9C00;
			WIN_Map_hi = 0x9FFF;
		}
		else
		{
			WIN_Map_lo = 0x9800;
			WIN_Map_hi = 0x9BFF;
		}

		if (control.BG_Tile_Map_Address)
		{
			BG_Map_lo = 0x9C00;
			BG_Map_hi = 0x9FFF;
		}
		else
		{
			BG_Map_lo = 0x9800;
			BG_Map_hi = 0x9BFF;
		}
		if (control.BG_Window_Tile_Data)
		{
			BG_WIN_Addr_Data_lo = 0x8000;
			BG_WIN_Addr_Data_hi = 0x8FFF;
		}
		else
		{
			BG_WIN_Addr_Data_lo = 0x8800;
			BG_WIN_Addr_Data_hi = 0x97FF;
		}

		break;

	case 0xFF41: // STAT

		status.Unused = 1;
		status.Bit6 = (data & 0x40) >> 6;
		status.Bit5 = (data & 0x20) >> 5;
		status.Bit4 = (data & 0x10) >> 4;
		status.Bit3 = (data & 0x08) >> 3;

		break;

	case 0xFF42: // Scroll Y
		Scroll_Y = data;

		break;

	case 0xFF43: // Scroll X
		Scroll_X = data;

		break;

	case 0xFF44: // LCDC_Y_Coordinate
		LY = 0;

		break;

	case 0xFF45: // LY Compare
		LYC = data;

		break;

	case 0xFF46: // DMA Transfer and Start Address
		bDMA = true;
		DMA = data;

		break;

	case 0xFF47: // BG Palette
		BG_Palette = data;

		for (int i = 0; i < 4; i++)
		{
			PaletteBG[i] = Palette[(BG_Palette >> (i * 2)) & 0x03];
		}

		break;

	case 0xFF48: // Object Palette 0 Data
		OBP0 = data;

		for (int i = 0; i < 4; i++)
		{
			PaletteOB[0][i] = Palette[(OBP0 >> (i * 2)) & 0x03];
		}

		break;

	case 0xFF49: // Object Palette 1 Data
		OBP1 = data;

		for (int i = 0; i < 4; i++)
		{
			PaletteOB[1][i] = Palette[(OBP1 >> (i * 2)) & 0x03];
		}

		break;

	case 0xFF4A: // Window Y Position
		WY = data;
		break;

	case 0xFF4B: // Window X Position
		WX = data;
		break;
	}
}

uint8_t PPU::CpuRead(uint16_t addr)
{
	uint8_t data = 0x00;

	switch (addr)
	{
	case 0xFF40: // LCD Control
		data = control.reg;
		break;

	case 0xFF41: // STAT

		data = status.reg | 0x80;
		break;

	case 0xFF42: // Scroll Y
		data = Scroll_Y;
		break;

	case 0xFF43: // Scroll X
		data = Scroll_X;
		break;

	case 0xFF44: // LCDC_Y_Coordinate
		data = LY;
		break;

	case 0xFF45: // LY Compare
		data = LYC;
		break;

	case 0xFF47: // BG Palette
		data = BG_Palette;
		break;

	case 0xFF48: // Object Palette 0 Data
		data = OBP0;
		break;

	case 0xFF49: // Object Palette 1 Data
		data = OBP1;
		break;

	case 0xFF4A: // Window Y Position
		data = WY;
		break;

	case 0xFF4B: // Window X Position
		data = WX;
		break;

	}

	return data;
}

void PPU::PpuWrite(uint16_t addr, uint8_t data)
{
}

uint8_t PPU::PpuRead(uint16_t addr)
{
	uint8_t data = 0;

	if (addr >= 0x8000 && addr <= 0x9FFF)
	{
		data = bus->PpuRead(addr);
	}

	return data;
}


void PPU::clock()
{
	// TO DO status, read and write block
	auto ChooseMode = [&]()
	{
		if (cycles < 20 && lines < 144) // OAM_Search
		{
			if (status.Bit5)
				bCurrent = true;
			else
				bCurrent = false;

			bOAMCpuBlock = true;

			if (cycles >= 1)
			{
				status.ModeBit1 = 1;
				status.ModeBit0 = 0;
			}
		}
		else if (cycles >= 20 && cycles < 63 && lines < 144) // Transfering_Data
		{
			bOAMCpuBlock = true;
			bVramCpuBlock = true;

			status.ModeBit1 = 1;
			status.ModeBit0 = 1;
		}
		else if (cycles >= 63 && lines < 144) // H_Blank
		{
			if (status.Bit3)
				bCurrent = true;
			else
				bCurrent = false;

			bOAMCpuBlock = false;
			bVramCpuBlock = false;

			status.ModeBit1 = 0;
			status.ModeBit0 = 0;
		}
		else if (lines >= 144) // V_Blank
		{
			if (status.Bit4 || status.Bit5)
				bCurrent = true;
			else
				bCurrent = false;

			bOAMCpuBlock = false;
			bVramCpuBlock = false;

			status.ModeBit1 = 0;
			status.ModeBit0 = 1;
		}
	};

	//////////////////////////////////////////////////////////////////////////

	// Background
	auto Fetch = [&](uint8_t b = 0)
	{
		tile_addr = BG_Map_lo + horz + (vert << 5);
		tile_id = PpuRead(tile_addr);
		addr_lo = BG_WIN_Addr_Data_lo + (tile_id << 4) + (vertical << 1) + 0;

		if (!control.BG_Window_Tile_Data)
			addr_lo = BG_WIN_Addr_Data_lo + ((128 + (int8_t)tile_id) << 4) + (vertical << 1) + 0;

		tmp_fifo_lo = PpuRead(addr_lo);
		addr_hi = BG_WIN_Addr_Data_lo + (tile_id << 4) + (vertical << 1) + 1;

		if (!control.BG_Window_Tile_Data)
			addr_hi = BG_WIN_Addr_Data_lo + (128 + ((int8_t)tile_id) << 4) + (vertical << 1) + 1;

		tmp_fifo_hi = PpuRead(addr_hi);
		// Idling

		if (b == 1)
		{
			fifo_lo = 0x0000;
			fifo_hi = 0x0000;
			fifo_lo |= (uint16_t)tmp_fifo_lo << 8;
			fifo_hi |= (uint16_t)tmp_fifo_hi << 8;
			horz++;
		}
		else if (b == 2)
		{
			fifo_lo |= (uint16_t)tmp_fifo_lo;
			fifo_hi |= (uint16_t)tmp_fifo_hi;
			horz++;
		}
	};

	auto FillFIFO = [&]()
	{
		Fetch();
		fifo_lo |= (uint16_t)tmp_fifo_lo;
		fifo_hi |= (uint16_t)tmp_fifo_hi;
		horz++;
	};

	auto ClockFIFO = [&](bool bDraw)
	{
		bg_pixel_lo = (fifo_lo & 0x8000) > 0;
		bg_pixel_hi = (fifo_hi & 0x8000) > 0;

		fifo_lo <<= 1;
		fifo_hi <<= 1;

		counter++;
		if (counter >= 8)
		{
			counter = 0;
			FillFIFO();
		}

		if (bDraw)
		{
			bg_pixel = (bg_pixel_hi << 1) | bg_pixel_lo;
			bg_color = PaletteBG[bg_pixel];
		}
	};

	// Window

	auto WINFetch = [&](uint8_t b = 0)
	{
		win_tile_addr = WIN_Map_lo + win_horz + (win_vert << 5);
		win_tile_id = PpuRead(win_tile_addr);
		win_addr_lo = BG_WIN_Addr_Data_lo + (win_tile_id << 4) + (win_vertical << 1) + 0;

		if (!control.BG_Window_Tile_Data)
			win_addr_lo = BG_WIN_Addr_Data_lo + (128 + ((int8_t)win_tile_id) << 4) + (win_vertical << 1) + 0;

		win_tmp_fifo_lo = PpuRead(win_addr_lo);
		win_addr_hi = BG_WIN_Addr_Data_lo + (win_tile_id << 4) + (win_vertical << 1) + 1;

		if (!control.BG_Window_Tile_Data)
			win_addr_hi = BG_WIN_Addr_Data_lo + (128 + ((int8_t)win_tile_id) << 4) + (win_vertical << 1) + 1;

		win_tmp_fifo_hi = PpuRead(win_addr_hi);

		// Idling

		if (b == 1)
		{
			win_fifo_lo = 0x0000;
			win_fifo_hi = 0x0000;
			win_fifo_lo |= (uint16_t)win_tmp_fifo_lo << 8;
			win_fifo_hi |= (uint16_t)win_tmp_fifo_hi << 8;
			win_horz++;
		}
		else if (b == 2)
		{
			win_fifo_lo |= (uint16_t)win_tmp_fifo_lo;
			win_fifo_hi |= (uint16_t)win_tmp_fifo_hi;
			win_horz++;
		}
	};

	auto WINFillFIFO = [&]()
	{
		WINFetch();
		win_fifo_lo |= (uint16_t)win_tmp_fifo_lo;
		win_fifo_hi |= (uint16_t)win_tmp_fifo_hi;
		win_horz++;
	};

	auto WINClockFIFO = [&](bool bDraw)
	{
		bg_pixel_lo = (win_fifo_lo & 0x8000) > 0;
		bg_pixel_hi = (win_fifo_hi & 0x8000) > 0;

		win_fifo_lo <<= 1;
		win_fifo_hi <<= 1;

		win_counter++;
		if (win_counter >= 8)
		{
			win_counter = 0;
			WINFillFIFO();
		}

		if (bDraw)
		{
			bg_pixel = (bg_pixel_hi << 1) | bg_pixel_lo;
			bg_color = PaletteBG[bg_pixel];
		}
	};

	auto TransferWindowData = [&](uint8_t offset)
	{
		uint8_t win_y = lines;
		uint8_t win_x = ((cycles - 20) << 2) + offset;
		uint8_t tmp_WX = (WX >= 7) ? WX - 7 : 0;

		if (win_y >= WY && win_x == tmp_WX)
		{
			win_horz = 0;
			win_counter = 0;
			WINFetch(1);
			WINFetch(2);
		}
		if (win_y >= WY && win_x >= tmp_WX)
		{
			WINClockFIFO(true);

			if ((WX >= 0 && WX <= 166) && (WY >= 0 && WY <= 143))
				if (cycles == 63 && offset == 0)
				{
					if (win_vertical == 7)
					{
						win_vertical = 0;
						win_vert++;
					}
					else
						win_vertical++;
				}
		}

	};

	// Sprites

	auto OAMSearch = [&]()
	{
		if ((status.reg & 0x03) == 0x02)
		{
			memcpy(sprites, bus->OAM, 0xA0);

			if (cycles == 19)
			{
				j = 0;
				for (uint8_t i = 0; i < 40; i++)
				{
					uint8_t diff = sprites[i].y - lines;
					if (diff <= 16 && diff > (control.OBJ_Size ? 0 : 8)) // ??
					{
						memcpy(screen_sprites + j, sprites + i, sizeof(Sprite));
						j++;
						if (j > 9)
							break;
					}
				}
			}
		}
	};

	auto FlipObjectTile = [=](const Sprite& obj, uint8_t& pixel, uint8_t& dify, uint8_t byte)
	{
		if (!control.OBJ_Size) // 8x8 Sprites
		{
			// Flipping verticaly
			if (obj.Flags & 0x40)
			{
				pixel = PpuRead(0x8000 + (obj.id << 4) + ((7 - 16 + dify) << 1) + byte);
			}

			// Flipping horizontaly
			if (obj.Flags & 0x20)
			{
				uint8_t temp = 0x00;
				for (uint8_t i = 0; i < 8; i++)
				{
					temp |= ((pixel >> i) & 0x01) << (7 - i);
				}
				pixel = temp;
			}
		}
		else // 8x16 Sprites
		{
			// Flipping verticaly
			uint8_t id = obj.id;
			if (obj.Flags & 0x40)
			{
				id = obj.id & 0xFE;
				pixel = PpuRead(0x8000 + (id << 4) + ((7 - 16 + dify + 8) << 1) + byte);

				if (dify >= 8)
				{
					id++;
					pixel = PpuRead(0x8000 + (id << 4) + ((7 - 16 + dify) << 1) + byte);
				}
			}

			// Flipping horizontaly
			if (obj.Flags & 0x20)
			{
				uint8_t temp = 0x00;
				for (uint8_t i = 0; i < 8; i++)
				{
					temp |= ((pixel >> i) & 0x01) << (7 - i);
				}
				pixel = temp;
			}
		}
	};

	auto GetOBJPixelColor = [&](const Sprite& obj, uint16_t x, uint16_t y, uint8_t difx)
	{
		if (!control.OBJ_Size) // 8x8 Sprites
		{
			uint8_t id = obj.id;
			uint8_t dify = obj.y - lines;
			uint8_t pixel_lo = PpuRead(0x8000 + (id << 4) + ((16 - dify) << 1) + 0);
			uint8_t pixel_hi = PpuRead(0x8000 + (id << 4) + ((16 - dify) << 1) + 1);

			FlipObjectTile(obj, pixel_lo, dify, 0);
			FlipObjectTile(obj, pixel_hi, dify, 1);

			uint8_t pixel = (((pixel_hi << difx) & 0x80) | (((pixel_lo << difx) & 0x80) >> 1)) >> 6;

			if (pixel)
			{
				fg_pixel = (((pixel_hi << difx) & 0x80) | (((pixel_lo << difx) & 0x80) >> 1)) >> 6;
				fg_color = PaletteOB[(obj.Flags & 0x10) >> 4][fg_pixel];
				fg_priority = obj.Flags & 0x80;
			}
		}
		else // 8x16 Sprites
		{
			uint8_t id = obj.id & 0xFE;
			uint8_t dify = obj.y - lines;

			uint8_t pixel_lo = PpuRead(0x8000 + (id << 4) + ((16 - dify) << 1) + 0);
			uint8_t pixel_hi = PpuRead(0x8000 + (id << 4) + ((16 - dify) << 1) + 1);

			if (dify < 8)
			{
				id++;
				pixel_lo = PpuRead(0x8000 + (id << 4) + ((16 - dify - 8) << 1) + 0);
				pixel_hi = PpuRead(0x8000 + (id << 4) + ((16 - dify - 8) << 1) + 1);
			}

			FlipObjectTile(obj, pixel_lo, dify, 0);
			FlipObjectTile(obj, pixel_hi, dify, 1);

			uint8_t pixel = (((pixel_hi << difx) & 0x80) | (((pixel_lo << difx) & 0x80) >> 1)) >> 6;

			if (pixel > 0x00)
			{
				fg_pixel = (((pixel_hi << difx) & 0x80) | (((pixel_lo << difx) & 0x80) >> 1)) >> 6;
				fg_color = PaletteOB[(obj.Flags & 0x10) ? 1 : 0][fg_pixel];
				fg_priority = obj.Flags & 0x80;
			}
		}
	};

	auto TransferObjectData = [&](uint8_t offset)
	{
		for (int i = j - 1; i >= 0; i--)
		{
			uint16_t difx = ((cycles - 20) << 2) + offset + 8 - screen_sprites[i].x;
			if (difx < 8 && difx >= 0)
			{
				uint16_t x = ((cycles - 20) << 2) + offset;
				GetOBJPixelColor(screen_sprites[i], x, lines, difx);
			}
		}
	};

	auto Priority = [&]()
	{
		uint8_t x = ((cycles - 20) << 2) + offset;
		uint8_t y = lines;

		if (control.LCD_Display_Enable)
		{
			if (control.BG_Enable)
			{
				if (bg_pixel == 0 && fg_pixel == 0)
				{
					screen.SetPixel(x, y, PaletteBG[0]);
				}
				// Transparency
				else if (bg_pixel > 0 && fg_pixel == 0)
				{
					screen.SetPixel(x, y, bg_color);
				}
				else if (bg_pixel > 0 && fg_pixel > 0)
				{
					if (!fg_priority)
					{
						screen.SetPixel(x, y, fg_color);
					}
					else
					{
						screen.SetPixel(x, y, bg_color);
					}
				}
				else if (bg_pixel == 0 && fg_pixel > 0)
				{
					screen.SetPixel(x, y, fg_color);
				}
			}
			else
			{
				screen.SetPixel(x, y, PaletteBG[0]);
			}
		}
		else
		{
			//screen.SetPixel(x, y, olc::Pixel(0xFF, 0xFF, 0xFF));
			screen.SetPixel(x, y, PaletteBG[0]);
		}
	};

	// Setting apropriate PPU mode
	ChooseMode();

	//Rendering

	if (cycles >= 0 && cycles < 20 && lines < 144)
	{
		OAMSearch();
	}

	if (cycles >= 20 && cycles <= 63 && lines < 144)
	{
		if (cycles == 20 && lines == 0)
		{
			win_vertical = 0;
			win_vert = 0;
		}

		vert = (lines >> 3) + (Scroll_Y >> 3);
		vertical = (lines & 7) + (Scroll_Y & 7);

		if (vertical > 7)
		{
			vert++;
			vertical &= 7;
		}
		if (vert > 31)
		{
			vert &= 31;
		}

		if (cycles == 20)
		{
			horz = Scroll_X / 8;

			if (horz >= 32)
				horz = 0;

			Fetch(1);

			if (horz >= 32)
				horz = 0;

			Fetch(2);
		}

		if (horz >= 32)
			horz = 0;

		offset = 0;

		if (cycles == 20)
		{
			fifo_lo <<= Scroll_X & 7;
			fifo_hi <<= Scroll_X & 7;
			counter = Scroll_X & 7;
		}

		if (!control.Window_Enable)
		{
			win_horz = 0;
			win_vert = 0;
			win_vertical = 0;
		}

		for (uint8_t i = 0; i < 4; i++)
		{
			fg_pixel = 0;
			bg_pixel = 0;
			ClockFIFO(true);
			if (control.OBJ_Enable)
				TransferObjectData(offset);
			if (control.Window_Enable)
				TransferWindowData(offset);
			// Rendering
			Priority();
			offset++;
		}

	}

	/////////////////////////////////////////////////////////////

	// Setting LYC Interrupt Flag
	if ((LYC == LY) && (status.Bit6))
		bCurrent = true;
	else
		bCurrent = false;
	if (!bLast && bCurrent)
		bus->IF.LCDC = 1;
	bLast = bCurrent;

	if (LYC == LY)
		status.Coincidence_Flag = 1;
	else
		status.Coincidence_Flag = 0;

	// Setting V-Blank Interrrupt Flag
	if ((lines == 144) && (cycles == 0) && control.LCD_Display_Enable)
	{
		bus->IF.V_Blank = 1;
	}

	// Scanline and Cycles Progress
	if (cycles == 114 - 1) // ?! - 1 
	{
		if (lines == 154 - 1)
		{
			lines = 0;
			bFrameComplete = true;
			frame_buffer = screen;

			//frameBuffers.resize(frameBuffers.size() + 100);
			/*frameBuffers.insert(frameBuffers.begin(), screen.Duplicate());*/
		}
		else lines++;

		LY = lines;

		cycles = 0;
	}
	else cycles++;

	if (lines == 153 && cycles >= 1)
	{
		LY = 0;
	}
}

olc::Sprite* PPU::GetScreen()
{
	if (!bFrameComplete)
		return &blank_screen;
	else
	{
		/*olc::Sprite* temp = frameBuffers[frameBuffers.size() - 1];
		frameBuffers.pop_back();*/
		//frameCount--;
		return &frame_buffer;
	}
}


olc::Sprite* PPU::GetSprScreen()
{
	return &sprScreen;
}

// Debugging Utilities

olc::Sprite* PPU::GetBGT()
{
	return &BG_Tiles;
}

void PPU::Get_BG_Tiles()
{
	for (uint8_t Tile_y = 0; Tile_y < 16 + 8; Tile_y++)
	{
		for (uint8_t Tile_x = 0; Tile_x < 16; Tile_x++)
		{
			for (uint8_t fine_y = 0; fine_y < 8; fine_y++)
			{
				uint16_t addr = (Tile_y * 16 + Tile_x) * 16 + fine_y * 2;

				uint8_t pixel_lo = PpuRead(0x8000 + addr + 0);
				uint8_t pixel_hi = PpuRead(0x8000 + addr + 1);

				for (uint8_t fine_x = 0; fine_x < 8; fine_x++)
				{

					uint8_t pixel = (((pixel_hi << fine_x) & 0x80) | (((pixel_lo << fine_x) & 0x80) >> 1)) >> 6;

					BG_Tiles.SetPixel(
						Tile_x * 8 + fine_x,
						Tile_y * 8 + fine_y,
						PaletteBG[pixel]);
				}
			}
		}
	}
}

void PPU::GetBGMap()
{
	for (int i = 0; i < 1024; i++)
	{
		BG_Map[i] = PpuRead(BG_Map_lo + i);
	}
}

void PPU::ChangePaletts()
{
	if (bGreen)
	{
		Palette[0] = olc::Pixel(139, 172, 15);
		Palette[1] = olc::Pixel(48, 98, 48);
		Palette[2] = olc::Pixel(15, 56, 15);
		Palette[3] = olc::Pixel(15, 56, 15);
	}
	bGreen != bGreen;
}