#include "Ft232h.h"
#include "Timer.h"
#include <libmpsse_i2c.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

using namespace std;
using namespace std::chrono;

Ft232h::Ft232h() noexcept : isValid{false}, handle{}, i2cAddr{}
{
}

Ft232h::Ft232h(FT_HANDLE handle, uint8_t i2cAddr, uint8_t gpioMask, const milliseconds& timeout) noexcept :
	isValid{true},
	handle{handle},
	i2cAddr{i2cAddr},
	gpioMask{gpioMask},
	timeout{timeout}
{
}

Ft232h::Ft232h(Ft232h &&other) : Ft232h()
{
	Swap(other);
}

Ft232h &Ft232h::operator=(Ft232h &&other)
{
	Swap(other);
	return *this;
}

void error(FT_STATUS s) { throw runtime_error { "FTDI error " + to_string(s) }; }

size_t Ft232h::WriteI2C(const span<uint8_t const> &data, DWORD options) const
{
	DWORD size {0};

	for (Timer t {timeout}; t.IsRunning(); t.Update())
	{
		const auto status = I2C_DeviceWrite(
			handle,
			i2cAddr,
			static_cast<DWORD>(data.size()),
			const_cast<uint8_t*>(data.data()),
			&size,
			options);
		
		switch (status)
		{
			case FT_OK:
			case FT_FAILED_TO_WRITE_DEVICE:
				return size;

			case FT_DEVICE_NOT_FOUND:
				this_thread::sleep_for(1ms);
				continue;
			
			default:
				error(status);
		}
	}

	error(FT_DEVICE_NOT_FOUND);
	return 0;
}

size_t Ft232h::ReadI2C(const std::span<uint8_t> &data, DWORD options) const
{
	DWORD size {0};
	uint8_t safeZero;

	for (Timer t {timeout}; t.IsRunning(); t.Update())
	{
		const auto status = I2C_DeviceRead(
			handle,
			i2cAddr,
			static_cast<DWORD>(data.size()),
			data.size() == 0 ? &safeZero : data.data(),
			&size,
			options);

		switch (status)
		{
			case FT_OK:
				return size;

			case FT_DEVICE_NOT_FOUND:
				this_thread::sleep_for(1ms);
				continue;
			
			default:
				error(status);
		}
	}

	error(FT_DEVICE_NOT_FOUND);
	return 0;
}

void Ft232h::WriteGPIO(uint8_t bits) const
{
	FT_WriteGPIO(handle, gpioMask, bits);
}

Ft232h::~Ft232h()
{
	if (!isValid) return;
	isValid = false;
}

void Ft232h::Swap(Ft232h &other) noexcept
{
	swap(isValid, other.isValid);
	swap(handle, other.handle);
	swap(i2cAddr, other.i2cAddr);
	swap(timeout, other.timeout);
}
