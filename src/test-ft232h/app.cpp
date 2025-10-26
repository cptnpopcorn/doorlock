#include <PN532OverFT232H.h>
#include <FrameWriterFuncs.h>
#include <NullTargetDataWriter.h>
#include <NullTargetDataValidator.h>
#include <TargetDataWriterFuncs.h>
#include <TargetDataValidatorFuncs.h>

#include <ftd2xx.h>
#include <libmpsse_i2c.h>

#include <array>
#include <iostream>
#include <stdexcept>

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
    ~channel_guard() { log_status(I2C_CloseChannel(channel), "close channel"); }

private:
    FT_HANDLE channel;
};

template<class T> void printhex(const T& bytes)
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
    if (log_status(I2C_GetNumChannels(&numChannels), "get number of I2C channels") != FT_OK) return 1;
    cout << "num channels = " << numChannels << endl;

    if (numChannels == 0)
    {
        cerr << "no channel available, exiting" << endl;
        return 1;
    }

	FT_HANDLE channel {};
    if (log_status(I2C_OpenChannel(0, &channel), "open channel") != FT_OK) return 1;

	{
		channel_guard guard_handle { channel };

		ChannelConfig config { .ClockRate { I2C_CLOCK_STANDARD_MODE }, .LatencyTimer { 16 } };
		if (log_status(I2C_InitChannel(channel, &config), "configure channel") != FT_OK) return 1;

		PN532OverFT232H pn532 { channel, 0x24, 0, 100ms };

		// this is how we would define our response parsing, the very modern way..
		auto validate = make_target_data_validator(
			[]{},
			[]{});

		auto writeResponseData = make_target_data_writer(
			[](const std::span<uint8_t const>& data){ printhex(data); cout << endl << flush; return data.size(); },
			[&validate] -> auto&& { return validate; });

		auto writeResponse = make_frame_writer(
			[]{},
			[]{},
			[]{},
			[]{},
			[&writeResponseData] -> auto&& { return writeResponseData; },
			[&validate] -> auto&& { return validate; },
			[]{});

		try
		{
			const auto getFirmwareVersion = to_array<uint8_t>({0x02});
			pn532.WriteFrame().DataFromHost(getFirmwareVersion);
			
			auto ack_writer = make_frame_writer(
				[]{ throw runtime_error{"LCS invalid"}; },
				[]{ cout << "ACK received" << endl << flush; },
				[]{ throw runtime_error{"NACKed"}; },
				[]{ throw runtime_error{"TFI invalid"}; },
				[] -> auto&& { return NullTargetDataWriter::Default(); },
				[] -> auto&& { return NullTargetDataValidator::Default(); },
				[]{});

			pn532.ReadFrame(ack_writer);

			auto frame_writer = make_frame_writer(
				[]{ throw runtime_error{"LCS invalid"}; },
				[]{ throw runtime_error{"unexpected ACK"};},
				[]{ throw runtime_error{"NACKed"}; },
				[]{ throw runtime_error{"TFI invalid"}; },
				[&writeResponseData] -> auto&& { return writeResponseData; },
				[&validate] -> auto&& { return validate; },
				[]{});

			pn532.ReadFrame(frame_writer);

			pn532.WriteFrame().Ack();
		}
		catch (const runtime_error& e)
		{
			cout << "exception caught: " << e.what() << endl;
		}
	}

	Cleanup_libMPSSE();
}