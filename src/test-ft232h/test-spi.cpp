#include <Ft232hSpi.h>
#include <Timer.h>

#include <ftd2xx.h>
#include <libmpsse_spi.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <thread>

using namespace std;

FT_STATUS log_status(FT_STATUS status, const char *comment)
{
    if (status == FT_OK)
    {
        cout << comment << " succeeded" << endl;
    }
    else
    {
        cerr << comment << " failed, error code: " << status << endl;
    }

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

int main(int, char**)
{
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

	{
		channel_guard guard_handle { channel };

		ChannelConfig config {
			.ClockRate = 100'000,
			.LatencyTimer = 2,
			.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW };

		if (log_status(SPI_InitChannel(channel, &config), "initialize SPI channel") != FT_OK) return 1;

		Ft232hSpi spi(channel, 2ms, 0b0000001);

		const auto write_get_firmware = to_array<uint8_t>({
			0b01,
			0x00,
			0x00, 0xFF,
			0x02,
			0xFE,
			0xD4,
			0x02,
			0x2A,
			0x00,
		});

		array<uint8_t, 16> response {};

		if (spi.Write(write_get_firmware) != write_get_firmware.size())
		{
			cerr << "could not write complete get firmware command" << endl;
			return 1;
		}

		for (auto rsp_length : to_array({5, 12}))
		{
			Timer t{200ms};
			for (; t.IsRunning(); t.Update())
			{
				const array<uint8_t, 3> write_read_status {0b10};
				if (spi.Write(write_read_status) != write_read_status.size())
				{
					cerr << "could not write read status byte" << endl;
					return 1;
				}

				array<uint8_t, 1> rdy {0};
				if (spi.Read(rdy) != 1)
				{
					cerr << "could not read RDY" << endl;
					return 1;
				}

				if ((rdy[0] & 0b1) == 0b1) break;

				this_thread::sleep_for(1ms);
			}

			if (!t.IsRunning())
			{
				cerr << "timeout while reading RDY status" << endl;
				return 1;
			}

			cout << "response RDY" << endl;

			const array<uint8_t, 1> write_read_data {0b11};
			if (spi.Write(write_read_data, true, false) != write_read_data.size())
			{
				cerr << "could not write read data" << endl;
				return 1;
			}

			auto trimmed_response = span{response}.subspan(0, rsp_length);

			if (spi.Read(trimmed_response, false, true) != trimmed_response.size())
			{
				cerr << "could not read response" << endl;
				return 1;
			}

			cout << "response: ";
			printhex(trimmed_response);
			cout << endl;
		}
	}

	Cleanup_libMPSSE();
	return 0;
}