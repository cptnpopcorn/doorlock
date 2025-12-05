#include <Desfire.h>
#include <Pn532BeetleEsp32C6Spi.h>
#include "soc/spi_reg.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

using namespace std;
using namespace std::chrono;

extern "C" void app_main(void)
{
	cout << "running" << endl;
	Pn532BeetleEsp32C6Spi pn_spi{20ms};

	while (true)
	{
		Desfire des{pn_spi};
		des.begin();

		cout << "waiting for 5 s" << endl;
		this_thread::sleep_for(5s);
		cout << "go" << endl;

		try
		{
			uint8_t icType {};
			uint8_t versionHi{};
			uint8_t versionLo{};
			uint8_t flags;
			if (des.GetFirmwareVersion(&icType, &versionHi, &versionLo, &flags))
			{
				cout << "IC Type " << (int)icType << endl;
				cout << "Version maj " << static_cast<int>(versionHi) << endl;
				cout << "Version min " << static_cast<int>(versionLo) << endl;
				cout << "Flags " << flags << endl;
			}
			else
			{
				cout << "error reading FW version" << endl;
			}
		}
		catch(const exception& e)
		{
			cout << e.what() << endl;
			return;
		}

		cout << "releasing" << endl;
		this_thread::sleep_for(1s);
	}

	cout << "ended" << endl;
}
