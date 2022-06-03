#include "psg.h"
#include <math.h>
#include <iostream>

uint8_t duty_waweforms[4] =
{
	0x01, 0x03, 0x0f, 0xfc
};

uint8_t psg::Read(uint16_t addr)
{
	uint8_t data = 0xFF;

	return data;
}

void psg::Write(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0xFF10:
		nr10 = data;
		break;
	case 0xFF11:
		nr11 = data;
		fLenght_1 = (64.0 - (float)(nr11 & 0x3F)) / 256.0;
		break;
	case 0xFF12:
		nr12 = data;
		fAmplitude = (float)((nr12 & 0xf0) >> 4) / 15.0;
		break;
	case 0xFF13:
		nr13 = data;
		break;
	case 0xFF14:
		nr14 = data;
		f = (uint16_t)nr13 + (((uint16_t)nr14 & 0x07) << 8);
		f = 131072.0 / (2048.0 - f);
		break;
	case 0xFF16:
		nr21 = data;
		break;
	case 0xFF17:
		nr22 = data;
		fAmplitude_2 = (float)((nr22 & 0xf0) >> 4) / 15.0;
		break;
	case 0xFF18:
		nr23 = data;
		break;
	case 0xFF19:
		nr24 = data;
		f_2 = (uint16_t)nr23 + (((uint16_t)nr24 & 0x07) << 8);
		f_2 = 131072.0 / (2048.0 - f_2);
		break;
	case 0xFF24:
		nr50 = data;
		break;
	case 0xFF25:
		nr51 = data;
		//std::cout << std::hex << (uint16_t)data << std::endl;
		break;
	case 0xFF26:
		nr52 = data;
		break;
		//default:
		//	std::cout << "0x" << std::hex << addr << "  0x"
		//		<< std::hex << (uint16_t)data << std::endl;
	}
}

float psg::approxsin(float t)
{
	float j = t * 0.15915;
	j = j - (int)j;
	return 20.785 * j * (j - 0.5) * (j - 1.0f);
}

float psg::Sound_Mode1()
{

	if (nr10 & 0x70)
	{
		float fStep = (float)((nr10 & 0x70) >> 4) / 128.0;
		float n = (float)(nr10 & 0x07);
		fTime_1_3 += 1.0f / 154.0f / 114.0f / 59.7;
		if (fTime_1_3 >= fStep)
		{
			fTime_1_3 -= fStep;
			if (nr10 & 0x08)
			{
				f += f / pow(2, n);
			}
			else
			{
				f -= f / pow(2, n);
				if (f < 0.0)
					f = 0.0;
			}
		}
	}
	else
		fTime_1_3 = 0.0;

	float fLenght = 1.0 / f;
	if (nr14 & 0x40)
	{
		if (fTime_1_1 < (fLenght * ((nr11 >> 6) ? ((float)(nr11 >> 6) * 2.0) : 1.0) / 8.0))
		{
			fSample = 1.0f;
		}
		else
			fSample = 0.0f;

		if (fLenght_1 <= 0.0)
			fSample = 0.0f;
		fLenght_1 -= 1.0 / 154.0 / 114.0 / 59.7;
	}
	else
	{
		if (fTime_1_1 >= fLenght / 8.0)
		{
			if (position_1 == 0)
			{
				duty_waweform = duty_waweforms[nr11 >> 6];
				position_1 = 0;
			}
			fTime_1_1 -= fLenght / 8.0;
			fSample = (duty_waweform & (0x1 << position_1)) ? 1.0 : 0.0;
			position_1++;
			if (position_1 > 7)
			{
				position_1 = 0;
			}
		}
	}

	if (fTime_1_1 >= fLenght)
	{
		fTime_1_1 -= fLenght;
	}

	fTime_1_1 += 1.0f / 154.0 / 114.0 / 59.7;

	if (nr14 & 0x80)
	{
		fTime_1_1 = 0.0;
		nr14 ^= 0x80;
	}

	float fStep;
	if ((nr12 & 0x07) && (int)f)
	{
		fStep = (float)(nr12 & 0x07) / 64.0;
		fTime_1_2 += 1.0f / 154.0f / 114.0f / 59.7;
		if (fTime_1_2 >= fStep)
		{
			fTime_1_2 -= fStep;
			if (nr12 & 0x08)
			{
				fAmplitude += 1.0 / 15.0;
				if (fAmplitude > 1.0)
					fAmplitude = 1.0;
			}
			else
			{
				fAmplitude -= 1.0 / 15.0;
				if (fAmplitude < 0.0)
					fAmplitude = 0.0;
			}
		}
	}
	else
		fTime_1_2 = 0.0;

	return fAmplitude * fSample / 2.0;
}

float psg::Sound_Mode2()
{
	float fLenght = 1.0 / f_2;
	if (nr24 & 0x40)
	{
		if (fTime_2_1 < (fLenght * ((nr21 >> 6) ? ((float)(nr21 >> 6) * 2.0) : 1.0) / 8.0))
		{
			fSample_2 = 1.0f;
		}
		else
			fSample_2 = 0.0f;

		if (fLenght_2 <= 0.0)
			fSample_2 = 0.0f;
		fLenght_2 -= 1.0 / 154.0 / 114.0 / 59.7;
	}
	else
	{
		if (fTime_2_1 >= fLenght / 8.0)
		{
			if (position_2 == 0)
			{
				duty_waweform_2 = duty_waweforms[nr21 >> 6];
				position_2 = 0;
			}
			fTime_2_1 -= fLenght / 8.0;
			fSample_2 = (duty_waweform_2 & (0x1 << position_2)) ? 1.0 : 0.0;
			position_2++;
			if (position_2 > 7)
			{
				position_2 = 0;
			}
		}
	}

	if (fTime_2_1 >= fLenght)
	{
		fTime_2_1 -= fLenght;
	}

	fTime_2_1 += 1.0f / 154.0 / 114.0 / 59.7;

	if (nr24 & 0x80)
	{
		fTime_2_1 = 0.0;
		nr24 ^= 0x80;
	}

	float fStep;
	if ((nr22 & 0x07) && (int)f_2)
	{
		fStep = (float)(nr22 & 0x07) / 64.0;
		fTime_2_2 += 1.0f / 154.0f / 114.0f / 59.7;
		if (fTime_2_2 >= fStep)
		{
			fTime_2_2 -= fStep;
			if (nr22 & 0x08)
			{
				fAmplitude_2 += 1.0 / 15.0;
				if (fAmplitude_2 > 1.0)
					fAmplitude_2 = 1.0;
			}
			else
			{
				fAmplitude_2 -= 1.0 / 15.0;
				if (fAmplitude_2 < 0.0)
					fAmplitude_2 = 0.0;
			}
		}
	}
	else
		fTime_2_2 = 0.0;

	return fAmplitude_2 * fSample_2 / 2.0;
}

float psg::Clock()
{
	float fSample1 = 0.0f;
	float fSample2 = 0.0f;

	if (nr52 & 0x80)
	{
		//if ((nr50 & 0x80) || (nr50 & 0x08))
		fSample1 = (nr12 >> 4) ? Sound_Mode1() : 0.0f;
		fSample2 = (nr22 >> 4) ? Sound_Mode2() : 0.0f;
	}

	return fSample1 / 4.0 + fSample2 / 4.0;
}
