#include <FrameParser.h>
#include <NullTargetDataWriter.h>
#include <NullTargetDataValidator.h>

#include "DataAsserter.h"
#include "TestFrameWriter.h"
#include "TestDataWriter.h"
#include "CppUTest/CommandLineTestRunner.h"

#include <array>
#include <vector>
#include <ranges>

using namespace std;

int main(int ac, char** av)
{
	return CommandLineTestRunner::RunAllTests(ac, av);
}

typedef size_t (*parse_input)(const span<uint8_t const>, FrameParser&);

size_t parse_whole(span<uint8_t const> data, FrameParser& parser)
{
	return parser.Parse(data);
}

size_t parse_octet_by_octet(span<uint8_t const> data, FrameParser& parser)
{
	size_t sum {0};

	while (!data.empty())
	{
		sum += parser.Parse(data.first(1));
		data = data.last(data.size() - 1);
	}

	return sum;
}

size_t parse_as_required(span<uint8_t const> data, FrameParser& parser)
{
	size_t sum {0};

	while (!data.empty())
	{
		const auto required = parser.GetRequiredLength();
		if (required == 0) return sum;

		auto next = min(required, data.size());
		while (next != 0)
		{
			const auto parsed = parser.Parse(data.subspan(0, next));
			CHECK_FALSE(parsed == 0);

			sum += parsed;
			next -= parsed;
			data = data.subspan(parsed);
		}
	}

	return sum;
}

inline constexpr auto dissectors = to_array<parse_input>({&parse_whole, &parse_octet_by_octet, &parse_as_required});

TEST_GROUP(FrameParserTests)
{
};

struct info_frame final
{
	span<uint8_t const> data() const { return span<uint8_t const>{input}.subspan(dataStart, dataLength); }

	vector<uint8_t> input;
	size_t dataStart;
	size_t dataLength;
	size_t parsedLength;
	bool isDcsValid;
};

TEST(FrameParserTests, InformationFrame)
{
	auto veryLong = vector<uint8_t> { 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0x2F, 0xD0, 0xD5 };
	veryLong.insert_range(veryLong.end(), views::repeat(static_cast<uint8_t>(0), 300));
	veryLong.insert_range(veryLong.end(), to_array<uint8_t>({0x01, 0x02, 0x28}));

	for (const auto& frame : to_array<info_frame>({
		{ {0x00, 0xFF, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x28 }, 5, 2, 8, true }, // normal - padding omitted
		{ {0x00, 0x00, 0xFF, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x28, 0x00}, 6, 2, 9, true }, // normal - standard padding
		{ {0x00, 0x00, 0x00, 0x00, 0xFF, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x28 }, 8, 2, 11, true }, // normal - excessively left-padded
		{ {0x00, 0xFF, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x28, 0x00, 0x00, 0x00 }, 5, 2, 8, true }, // normal - excessively right-padded
		{ {0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x28 }, 8, 2, 11, true }, // extended - padding omitted
		{ veryLong, 8, 302, 311, true },
		{ {0x00, 0xFF, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x25 }, 5, 2, 8, false }, // normal - padding omitted
		{ {0x00, 0x00, 0xFF, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x18, 0x00}, 6, 2, 9, false }, // normal - standard padding
		{ {0x00, 0x00, 0x00, 0x00, 0xFF, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0xF9 }, 8, 2, 11, false }, // normal - excessively left-padded
		{ {0x00, 0xFF, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x77, 0x00, 0x00, 0x00 }, 5, 2, 8, false }, // normal - excessively right-padded
		{ {0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x03, 0xFD, 0xD5, 0x01, 0x02, 0x21 }, 8, 2, 11, false }, // extended - padding omitted
		}))
	{
		for (auto dissect : dissectors)
		{
			auto isDcsPresent { false };
			DataAsserter dataAsserter { frame.data() };

			TestDataWriter data {
				[&dataAsserter](auto d) { return dataAsserter.Write(d); },
				[&isDcsPresent, &frame]() { isDcsPresent = true; CHECK_TRUE(frame.isDcsValid); },
				[&isDcsPresent, &frame]() { isDcsPresent = true; CHECK_FALSE(frame.isDcsValid); }};

			TestFrameWriter writer {
				[]() { FAIL("invalid LCS"); },
				[]() { FAIL("ack"); },
				[]() { FAIL("nack"); },
				[](uint8_t) { FAIL("invalid TFI"); },
				[&data]() -> TargetDataWriter& { return data; },
				[](uint8_t) -> TargetDataValidator& { FAIL("error to host"); return NullTargetDataValidator::Default(); },
				[]() { FAIL("incomplete"); }};

			FrameParser parser {writer};
			CHECK_EQUAL(frame.parsedLength, dissect(frame.input, parser));
			CHECK_TRUE(isDcsPresent);
		}
	}
}

struct bad_lcs_frame
{
	vector<uint8_t> input;
	size_t consumed;
};

TEST(FrameParserTests, BadLcs)
{
	auto veryLong = vector<uint8_t> { 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0x2F, 0xC0, 0xD4 };
	veryLong.insert_range(veryLong.end(), views::repeat(static_cast<uint8_t>(0), 300));
	veryLong.insert_range(veryLong.end(), to_array<uint8_t>({0x01, 0x02, 0x29}));

	for (const auto& frame : to_array<bad_lcs_frame>({
		{{0x00, 0x00, 0xFF, 0x03, 0xFC, 0xD5, 0x01, 0x02, 0x28, 0x00}, 5},
		{veryLong, 7}}))
	{
		for (auto dissect : dissectors)
		{
			bool isLcsInvalid { false };

			TestFrameWriter writer {
				[&isLcsInvalid]() { isLcsInvalid = true; },
				[]() { FAIL("ack"); },
				[]() { FAIL("nack"); },
				[](uint8_t) { FAIL("invalid TFI"); },
				[]() -> TargetDataWriter& { FAIL("data to host"); return NullTargetDataWriter::Default(); },
				[](uint8_t) -> TargetDataValidator& { FAIL("error to host"); return NullTargetDataValidator::Default(); },
				[]() { FAIL("incomplete"); }};

			FrameParser parser {writer};
			CHECK_EQUAL(frame.consumed, dissect(frame.input, parser));
		}
	}
}

TEST(FrameParserTests, Ack)
{
	for (auto dissect : dissectors)
	{
		auto isAck {false };

		TestFrameWriter writer {
			[]() { FAIL("invalid LCS"); },
			[&isAck]() { isAck = true; },
			[]() { FAIL("nack"); },
			[](uint8_t) { FAIL("invalid TFI"); },
			[]() -> TargetDataWriter& { FAIL("data to host"); return NullTargetDataWriter::Default(); },
			[](uint8_t) -> TargetDataValidator& { FAIL("error to host"); return NullTargetDataValidator::Default(); },
			[]() { FAIL("incomplete"); }};

		FrameParser parser {writer};
		dissect(to_array<uint8_t>({0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00}), parser);
		CHECK_TRUE(isAck);
	}
}

TEST(FrameParserTests, Nack)
{
	for (auto dissect : dissectors)
	{
		auto isNack {false };

		TestFrameWriter writer {
			[]() { FAIL("invalid LCS"); },
			[]() { FAIL("ack"); },
			[&isNack]() { isNack = true; },
			[](uint8_t) { FAIL("invalid TFI"); },
			[]() -> TargetDataWriter& { FAIL("data to host"); return NullTargetDataWriter::Default(); },
			[](uint8_t) -> TargetDataValidator& { FAIL("error to host"); return NullTargetDataValidator::Default(); },
			[]() { FAIL("incomplete"); }};
	
		FrameParser parser {writer};
		dissect(to_array<uint8_t>({0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00}), parser);
		CHECK_TRUE(isNack);
	}
}

TEST(FrameParserTests, Error)
{
	const auto frame = to_array<uint8_t>({0x00, 0x00, 0xFF, 0x01, 0xFF, 0x7F, 0x81, 0x00});

	for (auto dissect : dissectors)
	{
		auto isError { false };
		auto isDcsPresent { false };

		TestDataWriter data {
			[&](auto d) { FAIL("data"); return d.size(); },
			[&isDcsPresent, &frame]() { isDcsPresent = true; },
			[&isDcsPresent, &frame]() { isDcsPresent = true; FAIL("DCS invalid"); }};

		TestFrameWriter writer {
			[]() { FAIL("invalid LCS"); },
			[]() { FAIL("ack"); },
			[]() { FAIL("nack"); },
			[](uint8_t) { FAIL("invalid TFI"); },
			[]() -> TargetDataWriter& { FAIL("data to host"); return NullTargetDataWriter::Default(); },
			[&isError, &data](uint8_t err) -> TargetDataValidator& { CHECK_EQUAL(0x7F, err); isError = true; return data.Validator();},
			[]() { FAIL("incomplete"); }};

		FrameParser parser {writer};
		dissect(frame, parser);
		CHECK_TRUE(isError);
		CHECK_TRUE(isDcsPresent);
	}
}

TEST(FrameParserTests, ErrorBadDcs)
{
	const auto frame = to_array<uint8_t>({0x00, 0x00, 0xFF, 0x01, 0xFF, 0x7F, 0x91, 0x00});

	for (auto dissect : dissectors)
	{
		auto isError { false };
		auto isDcsPresent { false };

		TestDataWriter data {
			[&](auto d) { FAIL("data"); return d.size(); },
			[&isDcsPresent, &frame]() { isDcsPresent = true; FAIL("DCS valid"); },
			[&isDcsPresent, &frame]() { isDcsPresent = true; }};

		TestFrameWriter writer {
			[]() { FAIL("invalid LCS"); },
			[]() { FAIL("ack"); },
			[]() { FAIL("nack"); },
			[](uint8_t) { FAIL("invalid TFI"); },
			[]() -> TargetDataWriter& { FAIL("data to host"); return NullTargetDataWriter::Default(); },
			[&isError, &data](uint8_t err) -> TargetDataValidator& { CHECK_EQUAL(0x7F, err); isError = true; return data.Validator(); },
			[]() { FAIL("incomplete"); }};

		FrameParser parser {writer};
		dissect(frame, parser);
		CHECK_TRUE(isError);
		CHECK_TRUE(isDcsPresent);
	}
}