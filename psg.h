#pragma once

#include <cstdint>

class psg
{
public:
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t data);

	uint8_t nr10;
	uint8_t nr11;
	uint8_t nr12;
	uint8_t nr13;
	uint8_t nr14;

	uint8_t nr21;
	uint8_t nr22;
	uint8_t nr23;
	uint8_t nr24;

	uint8_t nr30;
	uint8_t nr31;
	uint8_t nr32;
	uint8_t nr33;
	uint8_t nr34;

	uint8_t nr41;
	uint8_t nr42;
	uint8_t nr43;
	uint8_t nr44;

	uint8_t nr50;
	uint8_t nr51;
	uint8_t nr52;

	uint8_t w[0x0F] = { 0 };

	float Clock();
	float Sound_Mode1();
	float Sound_Mode2();

	float fTime_1;
	float fTime_1_1;
	float fTime_1_2;
	float fTime_2;
	float fTime_2_1;
	float fTime_2_2;
	float fTime_1_3;

	uint8_t sweep;
	float fStep;
	float fAmplitude;
	float fAmplitude_2;
	float approxsin(float t);
	float f;
	float f_2;
	float fLenght_1;
	float fLenght_2;

	bool b = true;
	float fSample = 0.0f;
	float fSample_2 = 0.0f;
	uint8_t position_1 = 0;
	uint8_t position_2 = 0;
	uint8_t duty_waweform;
	uint8_t duty_waweform_2;

};

