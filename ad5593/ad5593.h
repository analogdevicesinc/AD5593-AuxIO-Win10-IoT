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

#pragma once

#include <stdio.h>
#include <string>

#include <sstream>
#include <iostream>
#include <cwctype>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::I2c;

enum IoFunction : unsigned char
{
	ADC,
	DAC,
	DAC_AND_ADC,
	GPIO,
	UNUSED_HIGH,
	UNUSED_LOW,
	UNUSED_TRISTATE,
	UNUSED_PULLDOWN,
};

enum GpioDirection : unsigned char
{
	GPIO_INPUT,
	GPIO_OUTPUT,
};

enum AD5593Register : unsigned char
{
	AD5593R_REG_NOOP = 0x0,
	AD5593R_REG_DAC_READBACK = 0x1,
	AD5593R_REG_ADC_SEQ = 0x2,
	AD5593R_REG_CTRL = 0x3,
	AD5593R_REG_ADC_EN = 0x4,
	AD5593R_REG_DAC_EN = 0x5,
	AD5593R_REG_PULLDOWN = 0x6,
	AD5593R_REG_LDAC = 0x7,
	AD5593R_REG_GPIO_OUT_EN = 0x8,
	AD5593R_REG_GPIO_SET = 0x9,
	AD5593R_REG_GPIO_IN_EN = 0xA,
	AD5593R_REG_PD = 0xB,
	AD5593R_REG_OPEN_DRAIN = 0xC,
	AD5593R_REG_TRISTATE = 0xD,
	AD5593R_REG_RESET = 0xF,
};

enum AD5593Mode : unsigned char
{
	AD5593R_MODE_CONF = (0 << 4),
	AD5593R_MODE_DAC_WRITE = (1 << 4),
	AD5593R_MODE_ADC_READBACK = (4 << 4),
	AD5593R_MODE_DAC_READBACK = (5 << 4),
	AD5593R_MODE_GPIO_READBACK = (6 << 4),
	AD5593R_MODE_REG_READBACK = (7 << 4),
};

class wexception
{
public:
	explicit wexception(const std::wstring &msg) : msg_(msg) { }
	virtual ~wexception() { /*empty*/ }

	virtual const wchar_t *wwhat() const
	{
		return msg_.c_str();
	}

private:
	std::wstring msg_;
};

class io
{
	friend class ad5593;

	unsigned short gpio_out = 0;
	unsigned short gpio_in = 0;
	unsigned short gpio_set = 0;
	unsigned short adc_en = 0;
	unsigned short dac_en = 0;
	unsigned short tristate = 0;
	unsigned short pulldown = 0xFF;

	unsigned char ident;
	ad5593 *parent;

	bool IsBitSet(unsigned short mask, unsigned char pos)
	{
		return (mask & (1 << pos)) != 0;
	}

	unsigned short ShortIsBitSet(unsigned short mask, unsigned char pos)
	{
		return (mask & (1 << pos)) != 0;
	}

	void SetBit(unsigned short &mask)
	{
		mask |= (1 << this->ident);
	}

	void ClearBit(unsigned short &mask)
	{
		mask &= ~(1 << this->ident);
	}

	void SetOrClear(unsigned short &mask, bool set)
	{
		if (set)
			SetBit(mask);
		else
			ClearBit(mask);
	}

public:
	IoFunction func;
	GpioDirection dir;

	io(ad5593 *parent, unsigned char ident, IoFunction func);
	void function(IoFunction func);
	IoFunction function();
	void direction(GpioDirection dir);
	void direction(GpioDirection dir, unsigned short value);
	GpioDirection direction();
	void set(unsigned short value);
	unsigned short get();
};

class ad5593
{
	friend class io;
	I2cDevice^ i2cDev;
	io *IOs[8];

	void initialize(IoFunction *defaults);

public:
	void i2c_write(unsigned char addr, unsigned short value);
	unsigned short i2c_read(unsigned char addr);

	ad5593(I2cDevice^ dev);
	ad5593(I2cDevice^ dev, IoFunction defaults[]);
	ad5593(I2cDevice^ dev, unsigned pin, IoFunction func);
	void reset();
	void reference_internal();
	void reference_range_high();
	io getPin(unsigned char pin);

};
