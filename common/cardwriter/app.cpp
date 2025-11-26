#include <Desfire.h>
#include <PN532OverFT232HSPI.h>
#include "card_operations.h"
#include <Timer.h>
#include <libmpsse_spi.h>
#include <CLI/CLI.hpp>
#include <cstdint>
#include <cctype>
#include <stdexcept>
#include <iostream>
#include <string>

using namespace std;
using namespace std::chrono;

FT_STATUS log_status(FT_STATUS status, const char *comment)
{
    if (status != FT_OK) cerr << comment << " failed, error code: " << status << endl;
    return status;
}

class channel_guard final
{
public:
    channel_guard(FT_HANDLE channel) : channel {channel} {}
    channel_guard(const channel_guard&) = delete;
    channel_guard& operator =(const channel_guard&) = delete;
    ~channel_guard() { log_status(SPI_CloseChannel(channel), "close channel"); }

private:
    FT_HANDLE channel;
};

template<class T> static void printhex(const T& bytes)
{
    for (const auto c : bytes)
    {
        cout << ' ' << hex << setw(2) << setfill('0') << (int)c;
    }
}

int main(int argc, char** argv)
{
	CLI::App cli{"BAM card programmer"};

	Init_libMPSSE();

	DWORD numChannels;
    if (log_status(SPI_GetNumChannels(&numChannels), "get number of I2C channels") != FT_OK) return 1;
    if (numChannels == 0)
    {
        cerr << "no channel available, exiting" << endl;
        return 1;
    }

	FT_HANDLE channel {};
    if (log_status(SPI_OpenChannel(0, &channel), "open channel") != FT_OK) return 1;

	try
	{
		channel_guard guard_handle { channel };

		ChannelConfig config {
			.ClockRate = 100'000,
			.LatencyTimer = 2,
			.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW };

		if (log_status(SPI_InitChannel(channel, &config), "configure channel") != FT_OK) return 1;

		PN532OverFT232HSPI pn532 { channel, 0, 500ms };
		Desfire cardreader { pn532 };
		cardreader.SetDebugLevel(0);
		cardreader.begin();
		
		// byte IC, VersionHi, VersionLo, Flags;
		uint8_t ic, ver_hi, ver_lo, flags;
		if (!cardreader.GetFirmwareVersion(&ic, &ver_hi, &ver_lo, &flags))
		{
			cout << "cannot read firmware version" << endl;
			return 1;
		}

		auto cout_flags = cout.flags();
		cout << "found card reader PN532" << hex << setw(2) << setfill('0') << static_cast<int>(ic) << endl;
		cout.flags(cout_flags);

		if (!cardreader.SetPassiveActivationRetries())
		{
			cout << "could not set passive activation retries" << endl;
			return 1;
		}

		if (!cardreader.SamConfig())
		{
			cout << "could not configure secure access module" << endl;
			return 1;
		}

		CardOperations ops {cardreader, 10s};

		cli.add_subcommand("info", "read card information")->callback([&ops]{ ops.GetInformation(cout); });

		auto &add_cmd = *cli.add_subcommand("write", "write user ID");

		string user_id_formatted {};

		auto user_id_option = add_cmd.add_option(
			"-u,--userid",
			user_id_formatted,
			"define user ID to write (10 bytes random number as 20 hex digits)");

		user_id_option->mandatory();
		user_id_option->check([](const auto option){
			if (ranges::count_if(option, [](const auto &c){ return isxdigit(c); }) != 20) return "expecting 20 hex digits for user ID";
			return ""; });
		
		add_cmd.callback([&ops, &user_id_formatted]{
			array<uint8_t, 10> user_id {};
			auto bit = 4;
			auto octet = user_id.begin();
			for (const auto& c : user_id_formatted)
			{
				if (octet == user_id.cend()) break;

				const auto nibble = c - (c >= 'a' ? 'a' - 10 : c >= 'A' ? 'A' - 10 : '0');
				if (nibble < 0 || nibble > 0xF) continue;
				
				*octet |= nibble << bit;

				const auto next_bit = bit - 4;
				octet += (next_bit >> 3) & 1;
				bit = next_bit & 7;
			}

			ops.WriteUserId(user_id, cout);
		});

		cli.add_subcommand("delete", "delete user ID")->callback([&ops, &user_id_option]{ ops.DeleteUserId(cout); });

		cli.require_subcommand();

		CLI11_PARSE(cli, argc, argv);
	}
	catch (const exception& err)
	{
		cout << "exception caught: " << err.what() << endl;
		return 1;
	}

	Cleanup_libMPSSE();
	return 0;
}