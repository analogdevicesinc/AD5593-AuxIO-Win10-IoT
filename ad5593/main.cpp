#include <stdio.h>
#include <ppltasks.h>
#include <collection.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cwctype>

#include "ad5593.h"

#define AUXIO_VERSION			"v0.3"
#define I2C_BUS_FRIENDLY_NAME	"I2C3"
#define I2C_SALVE_ID			0x10

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::I2c;

I2cDevice^ GetI2CDevice(int slaveAddress, _In_opt_ String^ friendlyName)
{
	using namespace Windows::Devices::Enumeration;

	String^ aqs;

	if (friendlyName)
		aqs = I2cDevice::GetDeviceSelector(friendlyName);
	else
		aqs = I2cDevice::GetDeviceSelector();

	auto dis = concurrency::create_task(DeviceInformation::FindAllAsync(aqs)).get();
	if (dis->Size < 1) {
		throw wexception(L"I2C controller not found");
	}

	String^ id = dis->GetAt(0)->Id;
	auto device = concurrency::create_task(I2cDevice::FromIdAsync(
		id,
		ref new I2cConnectionSettings(slaveAddress))).get();

	if (!device) {
		std::wostringstream msg;
		msg << L"Slave address 0x" << std::hex << slaveAddress << L" on bus " << id->Data() <<
			L" is in use. Please ensure that no other applications are using I2C.";
		throw wexception(msg.str());
	}

	return device;
}

void PrintUsage(PCWSTR name, const char* version)
{
	wprintf(
        L"AuxIO: Command line auxiliary IO test utility, Version %hs\n"
        L"Usage: %s -p <Pin Number> -f <Function> (-v <Value> | -u <Voltage>])\n"
        L"\n"
        L"Pin Number      IO pin select, number between 0..7\n"
        L"Function        IO function select must be one of following:\n"
        L"                  adc             : ADC input\n"
        L"                  dac             : DAC output\n"
        L"                  dac-adc         : DAC Output and ADC input\n"
        L"                                    This mode is useful if you want to output\n"
        L"                                    a certain voltage while you sense the actual\n"
        L"                                    voltage at the Pin. It might be the case that\n"
        L"                                    the load on the pin is high and the voltage\n"
        L"                                    has dropped.\n"
        L"                  gpio-in         : GPIO input\n"
        L"                  gpio-out        : GPIO output\n"
        L"                  unused-pulldown : Unused pulldown\n"
        L"                                    The pin is not used by any of the functions\n"
        L"                                    above, but is internally Pulled Down by\n"
        L"                                    a resistor.\n"
        L"                  unused-tristate : Unused tristate\n"
        L"                                    The pin is not used by any of the functions\n"
        L"                                    above, but is in high impedance mode\n"
        L"                  unused-low      : Unused low.\n"
        L"                                    The pin is not used by any of the functions\n"
        L"                                    above, but is tied low.\n"
        L"                  unused-high     : Unused high\n"
        L"                                    The pin is not used by any of the functions\n"
        L"                                    above, but is tied high.\n"
        L"Value           Function dependent value 0..4095\n"
        L"Voltage         Function dependent value 0.0 .. 5.0 Volt\n"
        L"\n"
        L"Examples:\n"
        L"%s -p 1 -f dac -v 2.5\n"
        L"%s -p 2 -f gpio-out -u 1\n",
		version, name, name, name);
}

int main(Array<String^>^ args)
{
	IoFunction func = UNUSED_TRISTATE;
	GpioDirection dir = GPIO_INPUT;
	unsigned short value = 0xFFFF;
	unsigned char pin = 0xFF;
	bool use_volatge = 0;
	double fval;

	if (args->Length < 5) {
		PrintUsage(args->get(0)->Data(), AUXIO_VERSION);
		return 0;
	}

	for (unsigned int optind = 1; optind < args->Length; optind += 2) {
		PCWSTR arg1 = args->get(optind)->Data();
		if (!_wcsicmp(arg1, L"-h") || !_wcsicmp(arg1, L"/h") ||
			!_wcsicmp(arg1, L"-?") || !_wcsicmp(arg1, L"/?")) {

			PrintUsage(args->get(0)->Data(), AUXIO_VERSION);
			return 0;
		}

		if (arg1[0] != L'-') {
			std::wcerr << L"Unexpected positional parameter: " << arg1 << L"\n";
			return 1;
		}

		if (args->get(optind)->Length() != 2) {
			std::wcerr << L"Invalid option format: " << arg1 << L"\n";
			return 1;
		}

		if ((optind + 1) >= args->Length) {
			std::wcerr << L"Missing required parameter for option: " << arg1 << L"\n";
			return 1;
		}
		PCWSTR arg2 = args->get(optind + 1)->Data();

		wchar_t *endptr;
		switch (towlower(arg1[1])) {

		case L'p':
			pin = static_cast<unsigned char>(
				wcstoul(arg2, &endptr, 0));

			if (pin < 0 || pin > 7) {
				std::wcerr << L"Invalid Pin Number\n";
				return 1;
			}

			break;
		case L'f':
			if (!_wcsicmp(arg2, L"gpio-in")) {
				func = GPIO;
				dir = GPIO_INPUT;
			}
			else if (!_wcsicmp(arg2, L"gpio-out")) {
				func = GPIO;
				dir = GPIO_OUTPUT;
			}
			else if (!_wcsicmp(arg2, L"adc"))
				func = ADC;
			else if (!_wcsicmp(arg2, L"dac"))
				func = DAC;
			else if (!_wcsicmp(arg2, L"dac-adc"))
				func = DAC_AND_ADC;
			else if (!_wcsicmp(arg2, L"unused-high"))
				func = UNUSED_HIGH;
			else if (!_wcsicmp(arg2, L"unused-low"))
				func = UNUSED_LOW;
			else if (!_wcsicmp(arg2, L"unused-tristate"))
				func = UNUSED_TRISTATE;
			else if (!_wcsicmp(arg2, L"unused-pulldown"))
				func = UNUSED_PULLDOWN;
			else {
				std::wcerr << L"Unknown function " << arg2 << "\n";
				return 1;
			}
			break;
		case L'u':
			use_volatge = true;
			fval = wcstof(arg2, &endptr);

			value = static_cast<unsigned short>(fval * 4096.0 / 5.0);
			if (value > 4095)
				value = 4095;
			break;
		case L'v':
			value = static_cast<unsigned short>(wcstoul(arg2, &endptr, 0));
			break;
		default:
			std::wcerr << L"Unrecognized option: " << arg1 << L"\n";
			return 1;
		}
	}

	try {
		auto I2CDevice = GetI2CDevice(I2C_SALVE_ID, I2C_BUS_FRIENDLY_NAME);

		auto dev = new ad5593(I2CDevice);

		dev->reference_internal();
		dev->reference_range_high();

		io MyPin = io(dev, pin, func);

		if (func == GPIO) {
			MyPin.direction(dir, value);
			use_volatge = false;
		}

		if (value != 0xFFFF && func != ADC)
			MyPin.set(value);

		if (func == ADC || func == DAC_AND_ADC || (func == GPIO && dir == GPIO_INPUT)) {
			if (use_volatge)
				std::wcout << (double)MyPin.get() * 5.0 / 4096.0 << L" Volt\n";
			else
				std::wcout << MyPin.get() << L"\n";
		}

	}
	catch (const wexception& ex) {
		std::wcerr << L"Error: " << ex.wwhat() << L"\n";
		return 1;
	}
	catch (Platform::Exception^ ex) {
		std::wcerr << L"Error: " << ex->Message->Data() << L"\n";
		return 1;
	}

	return 0;
}