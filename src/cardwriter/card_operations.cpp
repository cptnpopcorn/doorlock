#include "card_operations.h"
#include <Desfire.h>
#include <Timer.h>
#include <array>
#include <iomanip>
#include <span>
#include <stdexcept>
#include <string>

using namespace std;
using namespace std::chrono;

CardOperations::CardOperations(Desfire &desfire, const milliseconds &timeout) noexcept :
	desfire{desfire}, timeout{timeout}
{
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

		return;
	}

	throw runtime_error("no card found");
}
