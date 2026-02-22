#ifndef A0AA2569_7694_4014_B37B_C7EE43FE43A0
#define A0AA2569_7694_4014_B37B_C7EE43FE43A0

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include <string>

class OpenMessageBuilder final : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, OpenMessageBuilder>
{
public:

	OpenMessageBuilder() noexcept;

	bool Key(const char* str, rapidjson::SizeType length, bool);
	bool String(const char* str, rapidjson::SizeType length, bool);
	bool Bool(bool b);

private:

	void unexpected_bool(const bool&);
	void unexpected_string(const std::string_view& s);
	void parse_open(const bool& b);
	void parse_user(const std::string_view& s);

	void (OpenMessageBuilder::*parse_bool)(const bool& b);
	void (OpenMessageBuilder::*parse_string)(const std::string_view& s);	

	bool hasOpen;
	bool isOpen;
	std::string user;

	static constexpr std::string_view open_tag { "open" };
	static constexpr std::string_view user_tag { "user" };
};

#endif /* A0AA2569_7694_4014_B37B_C7EE43FE43A0 */
