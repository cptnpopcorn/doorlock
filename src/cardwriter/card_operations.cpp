#include "card_operations.h"
#include <Desfire.h>
#include <Timer.h>
#include <card_layout.h>
#include <secrets.h>
#include <algorithm>
#include <array>
#include <iomanip>
#include <span>
#include <stdexcept>
#include <string>

using namespace std;
using namespace std::chrono;

static inline void set_key(AES& aes, const array<uint8_t, 16>& key, uint8_t version)
{
	aes.SetKeyData(key.data(), key.size(), version);
}

CardOperations::CardOperations(Desfire &desfire, const milliseconds &timeout) noexcept :
	desfire{desfire}, timeout{timeout}, piccMasterKey{}
{
	set_key(piccMasterKey, secrets::CARD_PICC_MASTER_AES_KEY, to_underlying(KeyVersionPicc::Master));
	set_key(doorlockMasterKey, secrets::CARD_DOORLOCK_MASTER_AES_KEY, to_underlying(KeyVersionDoorlock::Master));
	set_key(doorlockWriteKey, secrets::CARD_DOORLOCK_WRITE_AES_KEY, to_underlying(KeyVersionDoorlock::Write));
	set_key(doorlockReadKey, secrets::CARD_DOORLOCK_READ_AES_KEY, to_underlying(KeyNumberDoorlock::Read));
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

		if (!desfire.SelectApplication(to_underlying(ApplicationId::Picc)))
		{
			out << "this is not a \"DESFire EV\" card, cannot be used" << endl;
			return;
		}

		if (desfire.Authenticate(to_underlying(KeyNumberPicc::Master), &desfire.DES2_DEFAULT_KEY))
		{
			out << "card is configured with default factory key, needs to be initialized first" << endl;
			return;
		}

		if (!desfire.Authenticate(to_underlying(KeyNumberPicc::Master), &piccMasterKey))
		{
			out << "card was already secured by another application, not usable" << endl;
			return;
		}

		if (!desfire.SelectApplication(to_underlying(ApplicationId::Doorlock)))
		{
			out << "no doorlock application data present / no user ID written" << endl;
			return;
		}

		out << "doorlock application found" << endl;

		if (!desfire.Authenticate(to_underlying(KeyNumberDoorlock::Read), &doorlockReadKey))
		{
			out << "could not access doorlock data for reading, security key not matching" << endl;
			return;
		}

		out << "doorlock application authenticated for reading" << endl;

		return;
	}

	throw runtime_error{"no card found"};
}

void CardOperations::WriteUserId(const span<const uint8_t> &id, ostream& out)
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

		if (!desfire.SelectApplication(to_underlying(ApplicationId::Picc)))
		{
			out << "this is not a \"DESFire EV\" card, cannot be used" << endl;
			return;
		}

		if (!desfire.Authenticate(to_underlying(KeyNumberPicc::Master), &piccMasterKey))
		{
			out << "card is not configured for doorlock yet, installing PICC master key" << endl; 

			if (!desfire.Authenticate(to_underlying(KeyNumberPicc::Master), &desfire.DES2_DEFAULT_KEY))
			{
				out << "card was already secured by another application, not usable" << endl;
				return;
			}

			if (!desfire.ChangeKey(to_underlying(KeyNumberPicc::Master), &piccMasterKey, nullptr))
			{
				out << "failed to install PICC master key" << endl;
				return;
			}

			if (!desfire.Authenticate(to_underlying(KeyNumberPicc::Master), &piccMasterKey))
			{
				out << "failed to re-authenticate after installing PICC master key" << endl;
				return;
			}
		}

		out << "card is configured for doorlock" << endl;

		if (!desfire.SelectApplication(to_underlying(ApplicationId::Doorlock)))
		{
			out << "doorlock application not created yet, creating.." << endl;

			if (!desfire.CreateApplication(
				to_underlying(ApplicationId::Doorlock),
				static_cast<DESFireKeySettings>(KS_ALLOW_CHANGE_MK | KS_CHANGE_KEY_WITH_MK),
				3,
				DESFireKeyType::DF_KEY_AES))
			{
				out << "error adding doorlock application to card" << endl;
				return;
			}

			if (!desfire.SelectApplication(to_underlying(ApplicationId::Doorlock)))
			{
				out << "error using freshly added doorlock application data" << endl;
				return;
			}
		}

		out << "found doorlock application data" << endl;

		if (!desfire.Authenticate(to_underlying(KeyNumberDoorlock::Master), &doorlockMasterKey))
		{
			if (!desfire.Authenticate(to_underlying(KeyNumberDoorlock::Master), &desfire.AES_DEFAULT_KEY))
			{
				out << "doorlock application data was created with another security key, cannot access" << endl;
				return;
			}

			out << "doorlock application key not set, configuring keys" << endl;

			if (!desfire.ChangeKey(to_underlying(KeyNumberDoorlock::Master), &doorlockMasterKey, nullptr))
			{
				out << "error setting doorlock application master key" << endl;
				return;
			}

			if (!desfire.Authenticate(to_underlying(KeyNumberDoorlock::Master), &doorlockMasterKey))
			{
				out << "error re-authenticating with doorlock application master key" << endl;
				return;
			}
		}

		out << "doorlock application authenticated" << endl;
		return;
	}

	throw runtime_error{"no card found"};
}

void CardOperations::DeleteUserId(ostream& out)
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

		if (!desfire.SelectApplication(to_underlying(ApplicationId::Picc)))
		{
			out << "this is not a \"DESFire EV\" card, cannot be used" << endl;
			return;
		}

		if (!desfire.Authenticate(to_underlying(KeyNumberPicc::Master), &piccMasterKey))
		{
			out << "card was already secured by another application, not usable" << endl;
			return;
		}

		if (!desfire.DeleteApplication(to_underlying(ApplicationId::Doorlock)))
		{
			out << "no doorlock application data present, nothing to do" << endl;
			return;
		}

		out << "doorlock application data deleted" << endl;
		return;
	}

	throw runtime_error{"no card found"};
}
