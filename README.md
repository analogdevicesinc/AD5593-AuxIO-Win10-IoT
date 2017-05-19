# AuxIO

This simple Win10 IoT command line tool controls the [AD5593R], connected via I2C peripheral bus.

The AD5593R has eight input/output (I/O) pins, which can be independently configured
as digital-to-analog converter (DAC) outputs, analog-to-digital converter (ADC) inputs,
digital outputs, or digital inputs. When an I/O pin is configured as an analog output,
it is driven by a 12-bit DAC. When an I/O pin is configured as an analog input,
it is connected to a 12-bit ADC via an analog multiplexer.
The I/O pins can also be configured to be general-purpose, digital input or output (GPIO) pins.


AuxIO is written in C++ and using the [Windows.Devices.I2c] WinRT APIs only available on Win10.
It also requires [user mode access] using the RhProxy driver.

This tool is open source and released under the [MIT license], it’s source code can be found [here]

Usage:
 ```
AuxIO: Command line auxiliary IO test utility, Version v0.2
Usage: AuxIO_v03.exe -p <Pin Number> -f <Function> (-v <Value> | -u <Voltage>])

Pin Number      IO pin select, number between 0..7
Function        IO function select must be one of following:
                  adc             : ADC input
                  dac             : DAC output
                  dac-adc         : DAC Output and ADC input
                                    This mode is useful if you want to output
                                    a certain voltage while you sense the actual
                                    voltage at the Pin. It might be the case that
                                    the load on the pin is high and the voltage
                                    has dropped.
                  gpio-in         : GPIO input
                  gpio-out        : GPIO output
                  unused-pulldown : Unused pulldown
                                    The pin is not used by any of the functions
                                    above, but is internally Pulled Down by
                                    a resistor.
                  unused-tristate : Unused tristate
                                    The pin is not used by any of the functions
                                    above, but is in high impedance mode
                  unused-low      : Unused low.
                                    The pin is not used by any of the functions
                                    above, but is tied low.
                  unused-high     : Unused high
                                    The pin is not used by any of the functions
                                    above, but is tied high.
Value           Function dependent value 0..4095
Voltage         Function dependent value 0.0 .. 5.0 Volt

Examples:
AuxIO_v03.exe -p 1 -f dac -v 2.5
AuxIO_v03.exe -p 2 -f gpio-out -u 1
 ```

[AD5593R]: http://www.analog.com/AD5593R
[Windows.Devices.I2c]: https://docs.microsoft.com/en-us/uwp/api/windows.devices.i2c
[user mode access]: https://docs.microsoft.com/en-us/windows/uwp/devices-sensors/enable-usermode-access
[here]: https://github.com/analogdevicesinc/AD5593-AuxIO-Win10-IoT
[MIT license]: https://opensource.org/licenses/MIT
