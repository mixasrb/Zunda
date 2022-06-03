
#define LOG 0

#include "BUS.h"

#include <iostream>
#include <sstream>

#include <math.h>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

class Engine : public olc::PixelGameEngine
{
public:
	Engine()
	{
		sAppName = "Zunda";
	}
public:

	BUS Gameboy;
	BUS Gameboy_a;
	static Engine* pInstance;
	const char* path;
#if DEBUG
	bool PauseEmulation = true;
#endif // DEBUG

#if !DEBUG
	bool PauseEmulation = false;
#endif // !DEBUG

	bool ByInstructionComplete = false;
	bool ByScanline = false;
	bool ByCycle = false;

	float fTime = 0.0f;

	std::string Convert(uint16_t d)
	{
		std::string str;
		std::stringstream ss;
		ss << d;
		ss >> str;

		return str;
	}

	std::string hex(uint16_t integer, uint8_t number_of_bits)
	{
		const char* chex = "0123456789ABCDEF";
		char* tmp = (char*)malloc(number_of_bits / 4 * sizeof(char));

		if (number_of_bits == 8)
		{
			tmp[0] = chex[integer / (int)pow(16, 1) % 16];
			tmp[1] = chex[integer / (int)pow(16, 0) % 16];
			tmp[2] = '\0';
		}
		else if (number_of_bits == 16)
		{
			tmp[0] = chex[integer / (int)pow(16, 3) % 16];
			tmp[1] = chex[integer / (int)pow(16, 2) % 16];
			tmp[2] = chex[integer / (int)pow(16, 1) % 16];
			tmp[3] = chex[integer / (int)pow(16, 0) % 16];
			tmp[4] = '\0';
		}

		return tmp;
	}

	void CpuDebug()
	{
		DrawString(162, 5, "Status: Z N H C _ _ _ _", olc::WHITE);

		if (Gameboy.cpu.f & 0x80)
			DrawString(226, 5, "Z", olc::GREEN);
		else
			DrawString(226, 5, "Z", olc::RED);
		if (Gameboy.cpu.f & 0x40)
			DrawString(242, 5, "N", olc::GREEN);
		else
			DrawString(242, 5, "N", olc::RED);
		if (Gameboy.cpu.f & 0x20)
			DrawString(258, 5, "H", olc::GREEN);
		else
			DrawString(258, 5, "H", olc::RED);
		if (Gameboy.cpu.f & 0x10)
			DrawString(274, 5, "C", olc::GREEN);
		else
			DrawString(274, 5, "C", olc::RED);

		DrawString(162, 20, "PC:0x", olc::WHITE);
		DrawString(204, 20, hex(Gameboy.cpu.pc, 16), olc::WHITE);

		DrawString(162, 30, "SP:0x", olc::WHITE);
		DrawString(204, 30, hex(Gameboy.cpu.sp, 16), olc::WHITE);

		DrawString(162, 40, "AF:0x", olc::WHITE);
		DrawString(204, 40, hex(Gameboy.cpu.af, 16), olc::WHITE);

		DrawString(162, 50, "BC:0x", olc::WHITE);
		DrawString(204, 50, hex(Gameboy.cpu.bc, 16), olc::WHITE);

		DrawString(162, 60, "DE:0x", olc::WHITE);
		DrawString(204, 60, hex(Gameboy.cpu.de, 16), olc::WHITE);

		DrawString(162, 70, "HL:0x", olc::WHITE);
		DrawString(204, 70, hex(Gameboy.cpu.hl, 16), olc::WHITE);

		DrawString(162, 90, Gameboy.cpu.str_opcode, olc::WHITE);

		DrawString(162, 100, "IF:0x" + hex(Gameboy.IF.data, 8), olc::WHITE);
		DrawString(162, 110, "IE:0x" + hex(Gameboy.IE.data, 8), olc::WHITE);

		// Interrupts

		DrawString(160, 130, "IE", olc::WHITE);

		if (Gameboy.IE.V_Blank)
			DrawString(162, 140, "V-Blank", olc::GREEN);
		else
			DrawString(162, 140, "V-Blank", olc::RED);
		if (Gameboy.IE.LCDC)
			DrawString(162, 150, "LCDC", olc::GREEN);
		else
			DrawString(162, 150, "LCDC", olc::RED);
		if (Gameboy.IE.Timer_Overflow)
			DrawString(162, 160, "Timer", olc::GREEN);
		else
			DrawString(162, 160, "Timer", olc::RED);
		if (Gameboy.IE.Serial_I_O)
			DrawString(162, 170, "Serial", olc::GREEN);
		else
			DrawString(162, 170, "Serial", olc::RED);
		if (Gameboy.IE.P10_P13)
			DrawString(162, 180, "Hi-Lo", olc::GREEN);
		else
			DrawString(162, 180, "Hi-Lo", olc::RED);

		DrawString(240, 130, "IF", olc::WHITE);

		if (Gameboy.IF.V_Blank)
			DrawString(240, 140, "V-Blank", olc::GREEN);
		else
			DrawString(240, 140, "V-Blank", olc::RED);
		if (Gameboy.IF.LCDC)
			DrawString(240, 150, "LCDC", olc::GREEN);
		else
			DrawString(240, 150, "LCDC", olc::RED);
		if (Gameboy.IF.Timer_Overflow)
			DrawString(240, 160, "Timer", olc::GREEN);
		else
			DrawString(240, 160, "Timer", olc::RED);
		if (Gameboy.IF.Serial_I_O)
			DrawString(240, 170, "Serial", olc::GREEN);
		else
			DrawString(240, 170, "Serial", olc::RED);
		if (Gameboy.IF.P10_P13)
			DrawString(240, 180, "Hi-Lo", olc::GREEN);
		else
			DrawString(240, 180, "Hi-Lo", olc::RED);
	}

	void MainDraw()
	{

		if (GetKey(olc::Key::P).bPressed)
			Gameboy.ppu.ChangePaletts();

		Gameboy.ppu.Get_BG_Tiles();
		DrawSprite(10, 154, Gameboy.ppu.GetBGT(), 1);

		olc::Sprite BG = olc::Sprite(256, 256);

		Gameboy.ppu.GetBGMap();
		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				uint8_t id = Gameboy.ppu.BG_Map[y * 32 + x];
				uint8_t ox = (id % 16) * 8;
				uint8_t oy = (id / 16) * 8 + ((Gameboy.ppu.control.BG_Window_Tile_Data) ? 0 : 128);

				if (!Gameboy.ppu.control.BG_Window_Tile_Data)
				{
					if (id >= 0x80)
					{
						oy = (id / 16) * 8;
					}
				}
				SetDrawTarget(&BG);
				DrawPartialSprite(x * 8, y * 8, Gameboy.ppu.GetBGT(), ox, oy, 8, 8);
				SetDrawTarget(nullptr);
			}
		}

		DrawSprite(350, 50, &BG);

#if DEBUG

		for (int i = 0; i < 20; i++)
		{
			std::string str = "Spr." + Convert(i) + " "
				+ "(" + Convert(Gameboy.ppu.sprites[i].x) + ","
				+ Convert(Gameboy.ppu.sprites[i].y) + ") "
				+ hex(Gameboy.ppu.sprites[i].id, 8);
			DrawString(160, 200 + i * 10, str);
		}

		for (int j = 0; j < 10; j++)
		{
			std::string str = "ScrS." + Convert(j) + " "
				+ "(" + Convert(Gameboy.ppu.screen_sprites[j].x) + ","
				+ Convert(Gameboy.ppu.screen_sprites[j].y) + ") "
				+ hex(Gameboy.ppu.screen_sprites[j].id, 8);
			DrawString(350, 310 + j * 10, str);
		}
#endif

		//DrawString(550, 310, "DIV : " + Convert(Gameboy.DIV), olc::RED); */
	}

	bool GetKeys(BUS& Gameboy)
	{
		Gameboy.p1 |= 0xCF;

		if (Gameboy.p1 == 0xEF)
		{
			if (GetKey(olc::Key::RIGHT).bHeld)
			{
				Gameboy.bButtonPressed = true;
				Gameboy.p1 &= 0xFE; // RIGHT
				Gameboy.IF.P10_P13 = 1;
			}
			if (GetKey(olc::Key::LEFT).bHeld)
			{
				Gameboy.bButtonPressed = true;
				Gameboy.p1 &= 0xFD; // LEFT
				Gameboy.IF.P10_P13 = 1;
			}
			if (GetKey(olc::Key::UP).bHeld)
			{
				Gameboy.bButtonPressed = true;
				Gameboy.p1 &= 0xFB; // UP
				Gameboy.IF.P10_P13 = 1;
			}
			if (GetKey(olc::Key::DOWN).bHeld)
			{
				Gameboy.bButtonPressed = true;
				Gameboy.p1 &= 0xF7; // DOWN
				Gameboy.IF.P10_P13 = 1;
			}
		}
		else if (Gameboy.p1 == 0xDF)
		{
			if (GetKey(olc::Key::X).bHeld)
			{
				Gameboy.bButtonPressed = true;
				Gameboy.p1 &= 0xFE; // A
				Gameboy.IF.P10_P13 = 1;
			}
			if (GetKey(olc::Key::Z).bHeld)
			{
				Gameboy.bButtonPressed = true;
				Gameboy.p1 &= 0xFD; // B
				Gameboy.IF.P10_P13 = 1;
			}
			if (GetKey(olc::Key::SHIFT).bHeld)
			{
				Gameboy.bButtonPressed = true;
				Gameboy.p1 &= 0xFB; // SELECT
				Gameboy.IF.P10_P13 = 1;
			}
			if (GetKey(olc::Key::ENTER).bHeld)
			{
				Gameboy.bButtonPressed = true;
				Gameboy.p1 &= 0xF7; // START
				Gameboy.IF.P10_P13 = 1;
			}
		}
		return true;
	}

	static float SoundOut(int nChanel, float fGlobalTime, float fTimeStep)
	{
		while ((!pInstance->Gameboy_a.Clock()) && (pInstance->GetKeys(pInstance->Gameboy_a)));
		return pInstance->Gameboy_a.fSample * 0.5;
	}

	bool OnUserCreate() override
	{
		Gameboy_a.InsertCartridge(path);
		Gameboy_a.start();

		char cart_name[16];
		memcpy(cart_name, Gameboy_a.cart.m_header.title, sizeof(Gameboy_a.cart.m_header.title));
		cart_name[15] = '\0';

		sAppName += " - " + (std::string)cart_name;


		pInstance = this;

		olc::SOUND::InitialiseAudio(44100, 1, 8, 512);
		olc::SOUND::SetUserSynthFunction(SoundOut);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

#if !DEBUG
		Clear(olc::WHITE);
#endif

#if DEBUG
		CpuDebug();
#endif

		/*

		if (GetKey(olc::Key::I).bPressed)
			ByInstructionComplete = true;

		if (ByInstructionComplete)
		{
			do
			{
				Gameboy.Clock();
			} while (!Gameboy.cpu.InstructionDone);
			Gameboy.cpu.InstructionDone = false;
			ByInstructionComplete = !ByInstructionComplete;
		}
		if (GetKey(olc::Key::L).bPressed)
			ByScanline = ~ByScanline;

		if (ByScanline)
		{
			int c = 114;
			do
			{
				Gameboy.Clock();
				c--;

			} while (c);
			ByScanline = false;

		}

		if (GetKey(olc::Key::C).bPressed)
			ByCycle = ~ByCycle;

		if (ByCycle)
		{
			int c = 1;
			do
			{
				Gameboy.Clock();
				c--;

			} while (c);
			ByCycle = false;

		}
		//*/
		//if (GetKey(olc::Key::SPACE).bPressed)
		//	PauseEmulation = !PauseEmulation;
		//
		//// Main Loop
		/*if (1)
		{
			fTime += fElapsedTime;
			while (fTime >= 1.0f / 60.0f)
			{
				do
				{
					GetKeys(Gameboy);
					Gameboy.Clock();

				} while (!Gameboy.ppu.bFrameComplete);
				Gameboy.ppu.bFrameComplete = false;
				fTime -= 1.0f / 60.0f;
			}
		}*/
		if(Gameboy_a.ppu.bFrameComplete)
		DrawSprite(0, 0, Gameboy_a.ppu.GetScreen());


		// Main Draw

#if DEBUG
		MainDraw();
#endif

		return true;
	}

	bool OnUserDestroy()
	{
		olc::SOUND::DestroyAudio();

		return true;
	}
	};

Engine* Engine::pInstance = nullptr;

int main(int argc, char* argv[])
{
	Engine demo;

	if (argc == 2)
		demo.path = argv[1];
	else
		demo.path = "Games\\super mario land.gb";

#if DEBUG
	if (demo.Construct(160 + 450, 144 + 270, 2, 2, false, false))
#endif
#if !DEBUG
		if (demo.Construct(160, 144, 6, 6, false, true))
#endif // !DEBUG
		{
			demo.Start();
		}
	return 0;
}