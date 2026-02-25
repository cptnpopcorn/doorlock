#include "const_stream.h"
#include "open_message_builder.h"

#include <span>
#include <string>

#include "CppUTest/CommandLineTestRunner.h"

using namespace std;

int main(int ac, char** av)
{
	return CommandLineTestRunner::RunAllTests(ac, av);
}

template <size_t N> constexpr std::span<const uint8_t> to_bytes(const char (&arr)[N])
{
	return { reinterpret_cast<const uint8_t*>(arr), N - 1 };
}

TEST_GROUP(OpenMessageBuilderTests)
{
};

TEST(OpenMessageBuilderTests, OpenForJohn)
{
	ConstStream stream{to_bytes(R"({ "open" : true, "user" : "John" })")};
	OpenMessageBuilder msg {};
	rapidjson::Reader{}.Parse(stream, msg);
	CHECK(msg.is_valid());
	CHECK(msg.get_open());
	CHECK(msg.get_user() == "John");
}

TEST(OpenMessageBuilderTests, CloseForJohn)
{
	ConstStream stream{to_bytes(R"({ "open" : false, "user" : "John" })")};
	OpenMessageBuilder msg {};
	rapidjson::Reader{}.Parse(stream, msg);
	CHECK(msg.is_valid());
	CHECK(!msg.get_open());
	CHECK(msg.get_user() == "John");
}

TEST(OpenMessageBuilderTests, NothingForJohn)
{
	ConstStream stream{to_bytes(R"({ "user" : "John" })")};
	OpenMessageBuilder msg {};
	rapidjson::Reader{}.Parse(stream, msg);
	CHECK(!msg.is_valid());
}

TEST(OpenMessageBuilderTests, OpenForNobody)
{
	ConstStream stream{to_bytes(R"({ "open" : true })")};
	OpenMessageBuilder msg {};
	rapidjson::Reader{}.Parse(stream, msg);
	CHECK(msg.is_valid());
	CHECK(msg.get_open());
	CHECK(msg.get_user() == "");
}

TEST(OpenMessageBuilderTests, CloseForNobody)
{
	ConstStream stream{to_bytes(R"({ "open" : false })")};
	OpenMessageBuilder msg {};
	rapidjson::Reader{}.Parse(stream, msg);
	CHECK(msg.is_valid());
	CHECK(!msg.get_open());
	CHECK(msg.get_user() == "");
}

TEST(OpenMessageBuilderTests, WrongProerties)
{
	ConstStream stream{to_bytes(R"({ "zcgzwc" : 2662, "oooox" : "John" })")};
	OpenMessageBuilder msg {};
	rapidjson::Reader{}.Parse(stream, msg);
	CHECK(!msg.is_valid());
}

TEST(OpenMessageBuilderTests, SurplusPropertiesOpenForJohn)
{
	ConstStream stream{to_bytes(R"({ "age" : 25, "open" : true, "valid" : false, "user" : "John" })")};
	OpenMessageBuilder msg {};
	rapidjson::Reader{}.Parse(stream, msg);
	CHECK(msg.is_valid());
	CHECK(msg.get_open());
	CHECK(msg.get_user() == "John");
}

TEST(OpenMessageBuilderTests, MixedUpOpenForJohn)
{
	ConstStream stream{to_bytes(R"({ "open" : "John", "user" : true })")};
	OpenMessageBuilder msg {};
	rapidjson::Reader{}.Parse(stream, msg);
	CHECK(!msg.is_valid());
}
