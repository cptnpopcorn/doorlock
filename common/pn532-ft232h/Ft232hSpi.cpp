#include "Ft232hSpi.h"
#include <libmpsse_spi.h>
#include <thread>
#include <utility>

using namespace std;
using namespace std::chrono;

Ft232hSpi::Ft232hSpi() noexcept : handle{}, gpioMask{}
{
}

Ft232hSpi::Ft232hSpi(FT_HANDLE handle, milliseconds wakeupLatency, uint8_t gpioMask) noexcept :
	handle{handle}, wakeupLatency{wakeupLatency}, gpioMask{gpioMask}
{
}

Ft232hSpi::Ft232hSpi(Ft232hSpi &&other) noexcept : Ft232hSpi()
{
	Swap(other);
}

Ft232hSpi &Ft232hSpi::operator=(Ft232hSpi &&other) noexcept
{
	Swap(other);
	return *this;
}

inline void error(FT_STATUS s) { throw runtime_error { "FTDI error " + to_string(s) }; }

size_t Ft232hSpi::Read(const std::span<uint8_t> &read, bool assertChipSelect, bool deassertChipSelect)
{
	if (assertChipSelect) AssertChipSelect();

	DWORD size {0};

	const auto status = SPI_Read(
		handle,
		read.data(),
		read.size(),
		&size,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES
		| SPI_TRANSFER_OPTIONS_LSB_FIRST
		| (deassertChipSelect ? SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE : 0));

	if (status != FT_OK) error(status);

	return size;
}

size_t Ft232hSpi::Write(const std::span<uint8_t const> &write, bool assertChipSelect, bool deassertChipSelect)
{
	if (assertChipSelect) AssertChipSelect();

	DWORD size {0};

	const auto status = SPI_Write(
		handle,
		const_cast<uint8_t*>(write.data()),
		write.size(),
		&size,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES
		| SPI_TRANSFER_OPTIONS_LSB_FIRST
		| (deassertChipSelect ? SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE : 0));

	if (status != FT_OK) error(status);

	return size;
}

void Ft232hSpi::DeassertChipSelect()
{
	const auto status = SPI_ToggleCS(handle, false);
	if (status != FT_OK) error(status);
}

void Ft232hSpi::AssertChipSelect()
{
	const auto status = SPI_ToggleCS(handle, true);
	if (status != FT_OK) error(status);
	Flush();
	this_thread::sleep_for(wakeupLatency);
}

void Ft232hSpi::WriteGPIO(uint8_t bits) const
{
	const auto status = FT_WriteGPIO(handle, gpioMask, bits);
	if (status != FT_OK) error(status);
}

void Ft232hSpi::Flush()
{
	DWORD rxBytes {};
	FT_GetQueueStatus(handle, &rxBytes);
}

void Ft232hSpi::Swap(Ft232hSpi &other) noexcept
{
	swap(handle, other.handle);
}