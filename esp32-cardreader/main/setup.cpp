#include "setup.h"
#include <interaction_control.h>
#include <iostream>>

using namespace std;

setup::setup(interaction &quit) noexcept : quit{quit}
{
}

void setup::start(interaction_control &control)
{
	cout << "card reader setup.." << endl;
    cout << "q - quit" << endl;

    switch (cin.get())
    {
		case 'q':
			control.set(quit);
			return;
	}
}