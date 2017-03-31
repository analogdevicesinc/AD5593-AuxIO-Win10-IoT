/********************************************************************************
* Copyright 2017(c)Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met :
*-Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  -Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and / or other materials provided with the
*    distribution.
*  -Neither the name of Analog Devices, Inc.nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*  -The use of this software may or may not infringe the patent rights
*    of one or more patent holders.This license does not release you
*    from the requirement that you obtain separate licenses from these
*    patent holders to use this software.
*  -Use of the software either in source or binary form, must be run
*    on or directly connected to an Analog Devices Inc.component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON - INFRINGEMENT,
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT
* LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "ad5593.h"

void ad5593::i2c_write(unsigned char addr, unsigned short value)
{
	auto i2CWriteBuffer = ref new Platform::Array<uint8>(3);

	i2CWriteBuffer[0] = addr;
	i2CWriteBuffer[1] = (unsigned char)(value >> 8);
	i2CWriteBuffer[2] = (unsigned char)(value & 0xFF);

	//std::wcerr << "write " << std::hex << addr  << " = " << std::hex << value << "\n";

	i2cDev->Write(i2CWriteBuffer);
}

unsigned short ad5593::i2c_read(unsigned char addr)
{
	auto i2CReadBuffer = ref new Platform::Array<uint8>(2);
	auto i2CWriteBuffer = ref new Platform::Array<uint8>(1);

	if (!(addr & 0xF0))
		addr |= AD5593R_MODE_REG_READBACK;

	i2CWriteBuffer[0] = addr;
	i2cDev->WriteRead(i2CWriteBuffer, i2CReadBuffer);

//	std::wcerr << "read " << std::hex << addr << " = " << std::hex << (unsigned short)(i2CReadBuffer[1] << 8 | i2CReadBuffer[0]) << "\n";

	return (unsigned short)(i2CReadBuffer[0] << 8 | i2CReadBuffer[1]);
}

void ad5593::initialize(IoFunction *defaults)
{
	// initialize I2C Port Expander registers
	for (int i = 0; i < 8; i++)
	{
		IOs[i] = new io(this, i, defaults[i]);
	}
}

ad5593::ad5593(I2cDevice^ dev)
{
	i2cDev = dev;
}

ad5593::ad5593(I2cDevice^ dev, IoFunction defaults[])
{
	i2cDev = dev;
	initialize(defaults);
}

ad5593::ad5593(I2cDevice^ dev, unsigned pin, IoFunction func)
{
	if (pin > 7)
		throw wexception(L"Out of Range");

	i2cDev = dev;
	IOs[pin] = new io(this, pin, func);
}

void ad5593::reset()
{
	i2c_write(AD5593R_REG_RESET, 0xdac);
}

void ad5593::reference_internal()
{
	i2c_write(AD5593R_REG_PD, 1 << 9);
}

void ad5593::reference_range_high()
{
	i2c_write(AD5593R_REG_CTRL, (1 << 4) | (1 << 5));
}

io ad5593::getPin(unsigned char pin)
{
	if (pin > 7)
		throw wexception(L"Out of Range");

	if (IOs[pin] == 0)
		throw wexception(L"Uninitialized");

	return *IOs[pin];
}

io::io(ad5593 *parent, unsigned char ident, IoFunction func)
{
	if (ident > 7 || ident < 0)
		throw wexception(L"Out of Range");

	this->parent = parent;
	this->ident = ident;
	this->func = func;

	function(func);
};

void io::function(IoFunction func)
{
	pulldown = parent->i2c_read(AD5593R_REG_PULLDOWN);
	tristate = parent->i2c_read(AD5593R_REG_TRISTATE);
	dac_en = parent->i2c_read(AD5593R_REG_DAC_EN);
	adc_en = parent->i2c_read(AD5593R_REG_ADC_EN);
	gpio_set = parent->i2c_read(AD5593R_REG_GPIO_SET);
	gpio_out = parent->i2c_read(AD5593R_REG_GPIO_OUT_EN);
	gpio_in = parent->i2c_read(AD5593R_REG_GPIO_IN_EN);

	ClearBit(pulldown);
	ClearBit(tristate);

	switch (func)
	{
	case ADC:
		SetBit(adc_en);
		ClearBit(gpio_out);
		ClearBit(gpio_in);
		break;
	case DAC:
		SetBit(dac_en);
		ClearBit(gpio_out);
		ClearBit(gpio_in);
		break;
	case DAC_AND_ADC:
		SetBit(adc_en);
		SetBit(dac_en);
		ClearBit(gpio_out);
		ClearBit(gpio_in);
		break;
	case GPIO:
		ClearBit(adc_en);
		ClearBit(dac_en);
		break;
	case UNUSED_LOW:
		ClearBit(adc_en);
		ClearBit(dac_en);
		ClearBit(gpio_set);
		SetBit(gpio_out);
		ClearBit(gpio_in);
		break;
	case UNUSED_HIGH:
		ClearBit(adc_en);
		ClearBit(dac_en);
		SetBit(gpio_set);
		SetBit(gpio_out);
		ClearBit(gpio_in);
		break;
	case UNUSED_TRISTATE:
		ClearBit(adc_en);
		ClearBit(dac_en);
		ClearBit(gpio_out);
		ClearBit(gpio_in);
		SetBit(tristate);
		break;
	case UNUSED_PULLDOWN:
		ClearBit(adc_en);
		ClearBit(dac_en);
		ClearBit(gpio_out);
		ClearBit(gpio_in);
		SetBit(pulldown);
		break;
	};

	parent->i2c_write(AD5593R_REG_PULLDOWN, pulldown);
	parent->i2c_write(AD5593R_REG_TRISTATE, tristate);
	parent->i2c_write(AD5593R_REG_DAC_EN, dac_en);
	parent->i2c_write(AD5593R_REG_ADC_EN, adc_en);
	parent->i2c_write(AD5593R_REG_GPIO_SET, gpio_set);
	parent->i2c_write(AD5593R_REG_GPIO_OUT_EN, gpio_out);
	parent->i2c_write(AD5593R_REG_GPIO_IN_EN, gpio_in);

	this->func = func;
}

IoFunction io::function()
{
	return this->func;
}

void io::direction(GpioDirection dir)
{
	switch (this->func)
	{
	case GPIO:
		gpio_out = parent->i2c_read(AD5593R_REG_GPIO_OUT_EN);
		gpio_in = parent->i2c_read(AD5593R_REG_GPIO_IN_EN);

		if (dir == GPIO_INPUT)
		{
			SetBit(gpio_in);
			ClearBit(gpio_out);
		}
		else if (dir == GPIO_OUTPUT)
		{
			SetBit(gpio_out);
			ClearBit(gpio_in);
		}
		parent->i2c_write(AD5593R_REG_GPIO_OUT_EN, gpio_out);
		parent->i2c_write(AD5593R_REG_GPIO_IN_EN, gpio_in);
		this->dir = dir;
		return;
	}
}

void io::direction(GpioDirection dir, unsigned short value)
{
	switch (this->func)
	{
	case GPIO:

		gpio_out = parent->i2c_read(AD5593R_REG_GPIO_OUT_EN);
		gpio_in = parent->i2c_read(AD5593R_REG_GPIO_IN_EN);

		if (dir == GPIO_INPUT)
		{
			SetBit(gpio_in);
			ClearBit(gpio_out);
		}
		else if (dir == GPIO_OUTPUT)
		{
			gpio_set = parent->i2c_read(AD5593R_REG_GPIO_SET);

			SetOrClear(gpio_set, value > 0);
			SetBit(gpio_out);
			ClearBit(gpio_in);

			parent->i2c_write(AD5593R_REG_GPIO_SET, gpio_set);
		}

		parent->i2c_write(AD5593R_REG_GPIO_OUT_EN, gpio_out);
		parent->i2c_write(AD5593R_REG_GPIO_IN_EN, gpio_in);
		this->dir = dir;

		return;
	}
}

GpioDirection io::direction()
{
	return this->dir;
}

void io::set(unsigned short value)
{
	switch (this->func)
	{

	case DAC:
	case DAC_AND_ADC:
		parent->i2c_write((unsigned char)(AD5593R_MODE_DAC_WRITE | (unsigned char) this->ident), value);
		break;
	case GPIO:
		gpio_set = parent->i2c_read(AD5593R_REG_GPIO_SET);
		SetOrClear(gpio_set, value > 0);
		parent->i2c_write(AD5593R_REG_GPIO_SET, gpio_set);
		return;
	default:
		throw wexception(L"Error");

	};
};

unsigned short io::get()
{
	switch (this->func)
	{

	case DAC:
		return parent->i2c_read((unsigned char)(AD5593R_REG_DAC_READBACK | (unsigned char) this->ident));
	case ADC:
	case DAC_AND_ADC:
		parent->i2c_write(AD5593R_MODE_CONF | AD5593R_REG_ADC_SEQ, (unsigned short)(1 << this->ident));
		return (unsigned short)((int)parent->i2c_read((unsigned char)AD5593R_MODE_ADC_READBACK) & 0xFFF);
	case GPIO:

		gpio_out = parent->i2c_read(AD5593R_REG_GPIO_OUT_EN);
		if (IsBitSet(gpio_out, this->ident))
		{
			gpio_set = parent->i2c_read(AD5593R_REG_GPIO_SET);
			return ShortIsBitSet(gpio_set, this->ident);
		}
		else
		{
			return ShortIsBitSet(parent->i2c_read(AD5593R_MODE_GPIO_READBACK), this->ident);
		}
		default:
			throw wexception(L"Error");
	};
};


