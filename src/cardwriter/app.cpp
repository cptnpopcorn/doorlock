#include <Desfire.h>
#include <PN532OverFT232HSPI.h>
#include "card_operations.h"
#include <Timer.h>
#include <libmpsse_spi.h>
#include <CLI/CLI.hpp>
#include <cstdint>
#include <stdexcept>
#include <iostream>

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
		add_cmd.callback([&ops]{ ops.WriteUserId(array<uint8_t, 0>{}, cout); }); // TODO: parse and validate 10 bytes hex string

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