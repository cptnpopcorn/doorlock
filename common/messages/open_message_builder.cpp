#include "open_message_builder.h"

using namespace std;

OpenMessageBuilder::OpenMessageBuilder() noexcept :
	parse_bool {&OpenMessageBuilder::unexpected_bool},
	parse_string {&OpenMessageBuilder::unexpected_string},
	hasOpen{false},
	user{}
{
}

bool OpenMessageBuilder::Key(const char *str, rapidjson::SizeType length, bool)
{
	parse_bool = &OpenMessageBuilder::unexpected_bool;
	parse_string = &OpenMessageBuilder::unexpected_string;

	const string_view key {str, length};
	if (key == open_tag)
	{
		parse_bool = &OpenMessageBuilder::parse_open;
	}
	else if (key == user_tag)
	{
		parse_string = &OpenMessageBuilder::parse_user;
	}

	return true;
}

bool OpenMessageBuilder::String(const char *str, rapidjson::SizeType length, bool)
{
	(this->*parse_string)({str, length});
	return true;
}

bool OpenMessageBuilder::Bool(bool b)
{
	(this->*parse_bool)(b);
	return true;
}

bool OpenMessageBuilder::is_valid() const noexcept
{
	return hasOpen;
}

bool OpenMessageBuilder::get_open() const noexcept
{
	return isOpen;
}

string_view OpenMessageBuilder::get_user() const noexcept
{
	return user;
}

void OpenMessageBuilder::unexpected_bool(const bool &)
{
}

void OpenMessageBuilder::unexpected_string(const string_view &s)
{
}

void OpenMessageBuilder::parse_open(const bool &b)
{
	hasOpen = true;
	isOpen = b;
}

void OpenMessageBuilder::parse_user(const string_view &s)
{
	user = s;
}
