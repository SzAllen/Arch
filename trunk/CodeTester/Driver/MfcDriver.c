#if _WINDOWS

#include "Global.h"

uint32 m_GpioIn  = 0;
uint32 m_Ptt = 0;
uint32 m_GpioOut = 0;

void PTT_Set_Level(uint8 channel,uint8 isHigh)
{
	if(isHigh)
	{
		Uint32Set(&m_Ptt, 0xFFFFFFFF, 1 << channel);
	}
	else
	{
		Uint32Set(&m_Ptt, 0, 1 << channel);
	}
}

void GPIO_Set_OutLevel(uint8 channel,uint8 isHigh)
{
	if(isHigh)
	{
		Uint32Set(&m_GpioOut, 0xFFFFFFFF, 1 << channel);
	}
	else
	{
		Uint32Set(&m_GpioOut, 0, 1 << channel);
	}
}

uint8 GPIO_Get_InLevel(uint8 channel)
{
	return BIT(m_GpioOut, channel);
}


void GPIO_Set_InLevel(uint8 channel, uint8 value)
{
	if(value)
	{
		Uint32Set(&m_GpioOut, 0xFFFFFFFF, 1 << channel);
	}
	else
	{
		Uint32Set(&m_GpioOut, 0, 1 << channel);
	}
}

#endif