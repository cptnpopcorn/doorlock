#include <FrameFormatter.h>
#include "CppUTest/CommandLineTestRunner.h"
#include <algorithm>

using namespace std;

int main(int ac, char** av)
{
	return CommandLineTestRunner::RunAllTests(ac, av);
}

TEST_GROUP(FrameFormatterTests)
{
};

TEST(FrameFormatterTests, DataHeaderOne)
{
	FrameFormatter formatter{};
	array<uint8_t, 0> data{};
	const auto header = formatter.GetDataHeader(data);
	CHECK(ranges::equal(header, to_array<uint8_t>({0x00, 0x00, 0xFF, 0x01, 0xFF, 0xD4})));
}

TEST(FrameFormatterTests, DataTailOne)
{
	FrameFormatter formatter{};
	array<uint8_t, 0> data{};
	const auto tail = formatter.GetDataTail(data);
	CHECK(ranges::equal(tail, to_array<uint8_t>({0x2C, 0x00})));
}

TEST(FrameFormatterTests, DataHeaderMaxShort)
{
	FrameFormatter formatter{};
	array<uint8_t, 254> data{};
	const auto header = formatter.GetDataHeader(data);
	CHECK(ranges::equal(header, to_array<uint8_t>({0x00, 0x00, 0xFF, 0xFF, 0x01, 0xD4})));
}

TEST(FrameFormatterTests, DataTailMaxShort)
{
	FrameFormatter formatter{};
	array<uint8_t, 254> data{};
	fill(data.begin(), data.end(), 1);
	const auto tail = formatter.GetDataTail(data);
	CHECK(ranges::equal(tail, to_array<uint8_t>({0x2E, 0x00})));
}

TEST(FrameFormatterTests, DataHeaderMinLong)
{
	FrameFormatter formatter{};
	array<uint8_t, 255> data{};
	const auto header = formatter.GetDataHeader(data);
	CHECK(ranges::equal(header, to_array<uint8_t>({0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0xFF, 0xD4})));
}

TEST(FrameFormatterTests, DataTailMinLong)
{
	FrameFormatter formatter{};
	array<uint8_t, 255> data{};
	fill(data.begin(), data.end(), 1);
	const auto tail = formatter.GetDataTail(data);
	CHECK(ranges::equal(tail, to_array<uint8_t>({0x2D, 0x00})));
}

TEST(FrameFormatterTests, DataHeaderMaxLong)
{
	FrameFormatter formatter{};
	array<uint8_t, 264> data{};
	const auto header = formatter.GetDataHeader(data);
	CHECK(ranges::equal(header, to_array<uint8_t>({0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0x09, 0xF6, 0xD4})));
}

TEST(FrameFormatterTests, DataTailMaxLong)
{
	FrameFormatter formatter{};
	array<uint8_t, 264> data{};
	fill(data.begin(), data.end(), 1);
	const auto tail = formatter.GetDataTail(data);
	CHECK(ranges::equal(tail, to_array<uint8_t>({0x24, 0x00})));
}

TEST(FrameFormatterTests, Ack)
{
	FrameFormatter formatter{};
	const auto ack = formatter.GetAck();
	CHECK(ranges::equal(ack, to_array({0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00})));
}

TEST(FrameFormatterTests, Nack)
{
	FrameFormatter formatter{};
	const auto nack = formatter.GetNack();
	CHECK(ranges::equal(nack, to_array({0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00})));
}