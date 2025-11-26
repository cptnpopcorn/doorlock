#ifndef A11F1F2B_A461_487F_BE2B_2B247D0B271F
#define A11F1F2B_A461_487F_BE2B_2B247D0B271F

#include <AES128.h>
#include <chrono>
#include <ostream>
#include <span>

class Desfire;

class CardOperations final
{
public:
	CardOperations(Desfire& desfire, const std::chrono::milliseconds& timeout) noexcept;

	void GetInformation(std::ostream& out);
	void WriteUserId(const std::span<const uint8_t>& id, std::ostream& out);
	void DeleteUserId(std::ostream& out);

private:
	Desfire& desfire;
	std::chrono::milliseconds timeout;
	AES piccMasterKey;
	AES doorlockMasterKey;
	AES doorlockWriteKey;
	AES doorlockReadKey;
};

#endif /* A11F1F2B_A461_487F_BE2B_2B247D0B271F */
