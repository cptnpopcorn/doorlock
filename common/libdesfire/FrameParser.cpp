#include "FrameParser.h"
#include "NullTargetDataWriter.h"
#include "NullTargetDataValidator.h"
#include "TargetFrameWriter.h"
#include "Tfi.h"

#include <algorithm>
#include <tuple>

using namespace std;

FrameParser::FrameParser(TargetFrameWriter &writer) noexcept :
	numLastOctets{0},
	lastOctets{},
	dataLength{},
	dataSum{0},
	state{&FrameParser::StartLength, &FrameParser::ParseStartCode},
	writer{writer},
	dataWriter{&NullTargetDataWriter::Default()},
	dataValidator{&NullTargetDataValidator::Default()}
{
}

size_t FrameParser::GetRequiredLength() const noexcept { return (*this.*state.Length)(); }
size_t FrameParser::Parse(const span<uint8_t const> &data) { return (*this.*state.Parse)(data); }

FrameParser::~FrameParser()
{
	if (GetRequiredLength() != 0) writer.FrameIncomplete();
}

size_t FrameParser::StartLength() const noexcept { return 5; } // shortest possible is ACK

constexpr tuple<uint8_t, uint8_t> start_code { 0x00, 0xFF };

size_t FrameParser::ParseStartCode(const span<uint8_t const> &data)
{
	auto it = data.cbegin();
	if (it == data.cend()) return 0;

	if (numLastOctets == 0)
	{
		numLastOctets = 1;
		lastOctets[0] = *it++;
	}

	while (it != data.cend())
	{
		const auto seq = make_tuple(lastOctets[0], *it++);
		
		if (seq == start_code)
		{
			ContinueWithLength();
			return it - data.cbegin() + ParseLength(data.last(data.cend() - it));
		}

		lastOctets[0] = get<1>(seq);
	}

	return data.size();
}

void FrameParser::ContinueWithLength() noexcept
{
	state = { &FrameParser::LengthLength, &FrameParser::ParseLength };
	numLastOctets = 0;
}

size_t FrameParser::LengthLength() const noexcept { return 3 - numLastOctets; } // we might stop at bad LCS for short format or 2 bytes ACK or NACK plus postamble

inline constexpr tuple<uint8_t, uint8_t> ack_code { start_code }; // same ;)
inline constexpr tuple<uint8_t, uint8_t> nack_code { 0xFF, 0x00 };
inline constexpr tuple<uint8_t, uint8_t> extended_code { 0xFF, 0xFF };

size_t FrameParser::ParseLength(const std::span<uint8_t const> &data)
{
	auto it = data.cbegin();
	if (it == data.cend()) return 0;

	if (numLastOctets == 0)
	{
		numLastOctets = 1;
		lastOctets[0] = *it++;
	}

	if (it == data.cend()) return data.size();

	const auto seq = make_tuple(lastOctets[0], *it++);
	const auto consumed = it - data.cbegin();
	const auto left = data.cend() - it;

	if (seq == ack_code)
	{
		writer.Ack();
		ContinueWithPostamble();
		return consumed + ParsePostamble(data.last(left));
	}

	if (seq == nack_code)
	{
		writer.Nack();
		ContinueWithPostamble();
		return consumed + ParsePostamble(data.last(left));
	}

	if (seq == extended_code)
	{
		ContinueWithExtendedLength();
		return consumed + ParseExtendedLength(data.last(left));
	}

	const auto& len = get<0>(seq);
	const auto& lcs = get<1>(seq);

	if ((len + lcs) & 0xFF != 0x00) // len + lcs corrupted
	{
		writer.LcsInvalid();
		Done();
		return consumed;
	}

	if (len == 1)
	{
		ContinueWithError();
		return consumed + ParseError(data.last(left));
	}

	ContinueWithTfi(len - 1);
	return consumed + ParseTfi(data.last(left));
}

void FrameParser::ContinueWithExtendedLength() noexcept
{
	state = { &FrameParser::ExtendedLengthLength, &FrameParser::ParseExtendedLength };
	numLastOctets = 0;
}

size_t FrameParser::ExtendedLengthLength() const noexcept { return 4 - numLastOctets; } // at least lenM, lenL and FCS and postamble

size_t FrameParser::ParseExtendedLength(const std::span<uint8_t const> &data)
{
	auto it = data.cbegin();

	while (numLastOctets != 2)
	{
		if (it == data.cend()) return it - data.cbegin();
		lastOctets[numLastOctets++] = *it++;
	}

	if (it == data.cend()) return data.size();

	const auto& lenM = lastOctets[0];
	const auto& lenL = lastOctets[1];
	const auto& lcs = *it++;
	const auto consumed = it - data.cbegin();

	if ((lenM + lenL + lcs & 0xFF) != 0x00) // len + lcs corrupted
	{
		writer.LcsInvalid();
		Done();
		return consumed;
	}

	ContinueWithTfi((static_cast<size_t>(lenM) << 8 | lenL) - 1);
	return consumed + ParseTfi(data.last(data.cend() - it));
}

void FrameParser::ContinueWithTfi(size_t dataLength) noexcept
{
	state = { &FrameParser::TfiLength, &FrameParser::ParseTfi };
	this->dataLength = dataLength;
}

size_t FrameParser::TfiLength() const noexcept { return dataLength + 3; } // tfi, data, dcs and postamble

size_t FrameParser::ParseTfi(const std::span<uint8_t const> &data)
{
	auto it = data.cbegin();
	if (it == data.cend()) return 0;

	const auto tfi = *it++;
	dataSum = tfi;

	const auto consumed = it - data.cbegin();
	const auto left = data.cend() - it;

	switch (tfi)
	{
		case to_underlying(Tfi::InfoToController):
			ContinueWithData(writer.DataToHost());
			break;

		default:
			writer.TfiInvalid(tfi);
			Done();
			return consumed;
	}

	return consumed + ParseData(data.last(left));
}

void FrameParser::ContinueWithData(TargetDataWriter &dataWriter) noexcept
{
	state = { &FrameParser::DataLength, &FrameParser::ParseData };
	this->dataWriter = &dataWriter;
}

size_t FrameParser::DataLength() const noexcept { return dataLength + 2; } // actual data + dcs + postamble

size_t FrameParser::ParseData(const std::span<uint8_t const> &data)
{
	const auto consumed = data.first(min(data.size(), dataLength));
	dataLength -= consumed.size();

	dataWriter->Write(consumed);
	for (const auto& d : consumed) dataSum += d;


	if (dataLength != 0) return consumed.size();
	
	ContinueWithDcs(dataWriter->Validator());
	return consumed.size() + ParseDcs(data.last(data.size() - consumed.size()));
}

void FrameParser::ContinueWithError() noexcept
{
	state = { &FrameParser::ErrorLength, &FrameParser::ParseError };
}

size_t FrameParser::ErrorLength() const noexcept { return 3; } // error code, dcs, postamble

size_t FrameParser::ParseError(const std::span<uint8_t const> &data)
{
	auto it = data.cbegin();
	if (it == data.cend()) return 0;

	const auto& err = *it++;

	dataSum = err;
	ContinueWithDcs(writer.Error(err));

	const auto consumed = it - data.cbegin();
	const auto left = data.cend() - it;

	return consumed + ParseDcs(data.last(left));
}

void FrameParser::ContinueWithDcs(TargetDataValidator &dataValidator) noexcept
{
	state = { &FrameParser::DcsLength, &FrameParser::ParseDcs };
	this->dataValidator = &dataValidator;
}

size_t FrameParser::DcsLength() const noexcept { return 2; } // dcs + postamble

size_t FrameParser::ParseDcs(const std::span<uint8_t const> &data)
{
	auto it = data.cbegin();
	if (it == data.cend()) return 0;

	const auto& dcs = *it++;
	const auto consumed = it - data.cbegin();
	const auto left = data.cend() - it;

	if ((dataSum + dcs & 0xFF) == 0x00)
	{
		dataValidator->DcsValid();
	}
	else
	{
		dataValidator->DcsInvalid();
	}

	ContinueWithPostamble();
	return consumed + ParsePostamble(data.last(left));
}

void FrameParser::ContinueWithPostamble() noexcept { state = { &FrameParser::PostambleLength, &FrameParser::ParsePostamble }; }

size_t FrameParser::PostambleLength() const noexcept { return 1; }

size_t FrameParser::ParsePostamble(const std::span<uint8_t const> &data)
{
	auto it = data.cbegin();
	if (it == data.cend()) return 0;

	const auto consumed = ++it - data.cbegin();

	Done();
	return consumed;
}

void FrameParser::Done() { state = { &FrameParser::DoneLength, &FrameParser::ParseDone }; }
size_t FrameParser::DoneLength() const noexcept { return 0; }
size_t FrameParser::ParseDone(const std::span<uint8_t const> &data) { return 0; }