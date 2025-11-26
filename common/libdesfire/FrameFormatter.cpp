#include "FrameFormatter.h"
#include "Tfi.h"

#include <utility>

using namespace std;

FrameFormatter::FrameFormatter() noexcept : header{0x00, 0x00, 0xFF}, tail{0x00, 0x00}
{
}

std::span<uint8_t const> FrameFormatter::GetDataHeader(const std::span<uint8_t const> &data)
{
	if (data.size() < 255)
	{
		const auto len = static_cast<uint8_t>(data.size() + 1);
		header[3] = len;
		header[4] = static_cast<uint8_t>(0 - len);
		header[5] = to_underlying(Tfi::InfoFromController);
		return{header.data(), 6};
	}
	else
	{
		const auto len = static_cast<uint16_t>(data.size() + 1);
		const auto lenM = static_cast<uint8_t>(len >> 8);
		const auto lenL = static_cast<uint8_t>(len);
		header[3] = 0xFF;
		header[4] = 0xFF;
		header[5] = lenM;
		header[6] = lenL;
		header[7] = static_cast<uint8_t>(0 - lenM - lenL);
		header[8] = to_underlying(Tfi::InfoFromController);
		return {header.data(), 9};
	}
}

std::span<uint8_t const> FrameFormatter::GetDataTail(const std::span<uint8_t const> &data) noexcept
{
	auto dcs = static_cast<uint8_t>(0 - to_underlying(Tfi::InfoFromController));
	for (const auto& d : data) dcs -= d;
	tail[0] = dcs;
	return tail;
}

inline constexpr auto ack = to_array<uint8_t>({0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00});

std::span<uint8_t const> FrameFormatter::GetAck() const noexcept
{
	return ack;
}

inline constexpr auto nack = to_array<uint8_t>({0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00});

std::span<uint8_t const> FrameFormatter::GetNack() const noexcept
{
	return nack;
}
