#include "card_operations.h"
#include <Desfire.h>
#include <Timer.h>
#include <secrets.h>
#include <array>
#include <iomanip>
#include <span>
#include <stdexcept>
#include <string>

using namespace std;
using namespace std::chrono;

CardOperations::CardOperations(Desfire &desfire, const milliseconds &timeout) noexcept :
	desfire{desfire}, timeout{timeout}, masterKey{}
{
	const auto& piccMaster = secrets::CARD_PICC_MASTER_AES_KEY;
	masterKey.SetKeyData(piccMaster.data(), piccMaster.size(), 0);
}

template<class T> static void printhex(const T& bytes, ostream &out)
{
    for (const auto c : bytes)
    {
        out << ' ' << hex << setw(2) << setfill('0') << (int)c;
    }
}

static const string nameDesFireRandom = "DESFire with random UID";
static const string nameDesFireStatic = "DESFire with static UID";
static const string nameGeneric = "DESFire with static UID";

static inline const string& format_card_type(eCardType cardType)
{
	switch (cardType)
	{
		case eCardType::CARD_DesRandom: return nameDesFireRandom;
		case eCardType::CARD_Desfire: return nameDesFireStatic;
		default: return nameGeneric;
	}
}

void CardOperations::GetInformation(ostream &out)
{
	for (Timer t {timeout}; t.IsRunning(); t.Update())
	{
		array<uint8_t, 8> uid;
		uint8_t uid_length {0};
		eCardType cardType {};

		if (!desfire.ReadPassiveTargetID(uid.data(), &uid_length, &cardType) || uid_length == 0)
		{
			out << '.';
			this_thread::sleep_for(500ms);
			continue;
		}

		out << endl;
		out << "card type: " << format_card_type(cardType) << endl;
		out << "card UID: "; printhex(span(uid).subspan(0, uid_length), out); out << endl;

		constexpr auto msgNotEV = "this is not a DESFire EV1/2/3 card, cannot be used";

		try
		{
			if (!desfire.SelectApplication(0)) throw runtime_error{"cannot select root"};
		}
		catch (runtime_error e)
		{
			out << "this is not a \"DESFire EV\" card, cannot be used" << endl;
			return;
		}

		if (desfire.Authenticate(0, &desfire.DES2_DEFAULT_KEY))
		{
			out << "card is configured with default factory key, needs to be initialized yet" << endl;
			return;
		}

		if (!desfire.Authenticate(0, &masterKey))
		{
			out << "card was already secured by another application, not usable" << endl;
			return;
		}

		return;
	}

	throw runtime_error{"no card found"};
}
