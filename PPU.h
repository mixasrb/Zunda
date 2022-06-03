#pragma once
#include <cstdint>

#include "olcPixelGameEngine.h"

class BUS;

class PPU
{
public:
	PPU()
	{

		// Black-White
		Palette[0] = olc::Pixel(0xE0, 0xDB, 0xCD);
		Palette[1] = olc::Pixel(0xA8, 0x9F, 0x94);
		Palette[2] = olc::Pixel(0x70, 0x6B, 0x66);
		Palette[3] = olc::Pixel(0x2B, 0x2B, 0x26);
		//Green
		//Palette[0] = olc::Pixel(0x9B, 0xBC, 0x0F);
		//Palette[1] = olc::Pixel(0x8B, 0xAC, 0x0F);
		//Palette[2] = olc::Pixel(0x30, 0x62, 0x30);
		//Palette[3] = olc::Pixel(0x0F, 0x38, 0x0F);

		for (uint8_t x = 0; x < 160; x++)
		{
			for (uint8_t y = 0; y < 144; y++)
			{
				blank_screen.SetPixel(x, y, Palette[0]);
			}
		}

	}

	BUS* bus;

	void ConnectPPU(BUS* pointer)
	{
		bus = pointer;
	}

public:

	void CpuWrite(uint16_t addr, uint8_t data);
	uint8_t CpuRead(uint16_t addr);

	void PpuWrite(uint16_t addr, uint8_t data);
	uint8_t PpuRead(uint16_t addr);

public:

	void clock();

	// REGISTERS

	union LCD_Control   // (R/W)
	{
		struct
		{
			uint8_t BG_Enable : 1;
			uint8_t OBJ_Enable : 1;
			uint8_t OBJ_Size : 1;
			uint8_t BG_Tile_Map_Address : 1;
			uint8_t BG_Window_Tile_Data : 1;
			uint8_t Window_Enable : 1;
			uint8_t Window_Tile_Map_Address : 1;
			uint8_t LCD_Display_Enable : 1;
		};
		uint8_t reg;
	}control;

	union LCD_Status   // (R/W)
	{
		struct
		{
			uint8_t ModeBit0 : 1;
			uint8_t ModeBit1 : 1;
			uint8_t Coincidence_Flag : 1;
			uint8_t Bit3 : 1;
			uint8_t Bit4 : 1;
			uint8_t Bit5 : 1;
			uint8_t Bit6 : 1;
			uint8_t Unused : 1;
		};
		uint8_t reg = 0x80;
	}status;

	bool bOAMCpuBlock = true;
	bool bVramCpuBlock = true;

	uint8_t Scroll_Y = 0x00; // (R/W)
	uint8_t Scroll_X = 0x00; // (R/W)
	uint8_t LY = 0x00; // (R)
	uint8_t LYC = 0x00; // (R/W)
	uint8_t DMA = 0x00; // (W)
	uint8_t BG_Palette = 0x00; // (R/W)
	uint8_t OBP0 = 0x00; // (R/W)
	uint8_t OBP1 = 0x00; // (R/W)
	uint8_t WY = 0x00; // (R/W)
	uint8_t WX = 0x00; // (R/W)

	// Sprite

	struct Sprite
	{
		uint8_t y;
		uint8_t x;
		uint8_t id;
		uint8_t Flags;
	};

public:
	Sprite sprites[40];
	Sprite screen_sprites[10];
	uint8_t j = 0;

	olc::Pixel PaletteOB[2][4];
	olc::Sprite* GetSprScreen();

	// Helper Variables

	olc::Sprite blank_screen = olc::Sprite(160, 144);

	uint16_t BG_WIN_Addr_Data_lo;
	uint16_t BG_WIN_Addr_Data_hi;

	uint16_t BG_Map_lo;
	uint16_t BG_Map_hi;

	uint16_t WIN_Map_lo;
	uint16_t WIN_Map_hi;

	uint8_t tile_id;
	uint16_t tile_addr;
	uint8_t fetch_cycles;
	uint8_t tmp_fifo_lo;
	uint8_t tmp_fifo_hi;
	uint16_t fifo_lo;
	uint16_t fifo_hi;
	uint8_t bg_pixel_lo;
	uint8_t bg_pixel_hi;

	uint8_t win_tile_id;
	uint16_t win_tile_addr;
	uint8_t win_fetch_cycles;
	uint8_t win_tmp_fifo_lo;
	uint8_t win_tmp_fifo_hi;
	uint16_t win_fifo_lo;
	uint16_t win_fifo_hi;

	uint8_t win_vertical;

	uint16_t win_addr_lo;
	uint16_t win_addr_hi;

	uint16_t win_horz;
	uint16_t win_vert;

	uint8_t win_counter;

	olc::Pixel bg_color;
	uint8_t bg_pixel;
	olc::Pixel fg_color;
	uint8_t fg_pixel;
	uint8_t fg_priority;

	uint16_t addr_lo;
	uint16_t addr_hi;

	uint8_t horz;
	uint8_t vert;
	uint8_t vertical;

	bool flag = false;
	bool flag2 = false;

	uint8_t offset;
	uint8_t counter;

	olc::Sprite screen = olc::Sprite(160, 144);
	olc::Sprite* GetScreen();

	int cycles = 0;
	int lines = 0;
	bool bLYC = false;
	bool bDMA = false;

	// Debugging Staff 

private:

	olc::Sprite BG_Tiles = olc::Sprite(128, 192);

public:

	olc::Sprite sprScreen = olc::Sprite(160, 144);

	olc::Sprite* GetBGT();

	void Get_BG_Tiles();

	olc::Pixel Palette[4];
	olc::Pixel PaletteBG[4];

	std::array<uint8_t, 1024> BG_Map = { 0 };

	void GetBGMap();

	bool bGreen = true;
	void ChangePaletts();

	bool bLast = false;
	bool bCurrent = false;

	bool b;
	bool bFrameComplete = false;

	olc::Sprite frame_buffer;

	std::vector<olc::Sprite*> frameBuffers;
	int frameCount = 0;

};