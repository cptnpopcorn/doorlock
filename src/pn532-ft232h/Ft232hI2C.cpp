#include "Ft232hI2C.h"
#include <Timer.h>
#include <libmpsse_i2c.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

using namespace std;
using namespace std::chrono;

Ft232hI2c::Ft232hI2c() noexcept : handle{}, i2cAddr{}
{
}

Ft232hI2c::Ft232hI2c(FT_HANDLE handle, uint8_t i2cAddr, uint8_t gpioMask, const milliseconds& timeout) noexcept :
	handle{handle},
	i2cAddr{i2cAddr},
	gpioMask{gpioMask},
	timeout{timeout}
{
}

Ft232hI2c::Ft232hI2c(Ft232hI2c &&other) noexcept : Ft232hI2c()
{
	Swap(other);
}

Ft232hI2c &Ft232hI2c::operator=(Ft232hI2c &&other) noexcept
{
	Swap(other);
	return *this;
}

inline void error(FT_STATUS s) { throw runtime_error { "FTDI error " + to_string(s) }; }

size_t Ft232hI2c::Write(const span<uint8_t const> &data, DWORD options) const
{
	DWORD size {0};
	uint8_t safeZero {};
	const auto buffer = data.size() == 0 ? &safeZero : const_cast<uint8_t*>(data.data());

	for (Timer t {timeout}; t.IsRunning(); t.Update())
	{
		const auto status = I2C_DeviceWrite(
			handle,
			i2cAddr,
			static_cast<DWORD>(data.size()),
			buffer,
			&size,
			options);
		
		switch (status)
		{
			case FT_OK:
			case FT_FAILED_TO_WRITE_DEVICE:
				return size;

			case FT_DEVICE_NOT_FOUND:
				this_thread::sleep_for(5ms);
				continue;
			
			default:
				error(status);
		}
	}

	error(FT_DEVICE_NOT_FOUND);
	return 0;
}

size_t Ft232hI2c::Read(const std::span<uint8_t> &data, DWORD options) const
{
	DWORD size {0};
	uint8_t safeZero;
	const auto buffer = data.size() == 0 ? &safeZero : data.data();

	for (Timer t {timeout}; t.IsRunning(); t.Update())
	{
		const auto status = I2C_DeviceRead(
			handle,
			i2cAddr,
			static_cast<DWORD>(data.size()),
			buffer,
			&size,
			options);

		switch (status)
		{
			case FT_OK:
				return size;

			case FT_DEVICE_NOT_FOUND:
				this_thread::sleep_for(5ms);
				continue;
			
			default:
				error(status);
		}
	}

	error(FT_DEVICE_NOT_FOUND);
	return 0;
}

void Ft232hI2c::WriteGPIO(uint8_t bits) const
{
	const auto status = FT_WriteGPIO(handle, gpioMask, bits);
	if (status != FT_OK) error(status);
}

void Ft232hI2c::Swap(Ft232hI2c &other) noexcept
{
	swap(handle, other.handle);
	swap(i2cAddr, other.i2cAddr);
	swap(timeout, other.timeout);
}
