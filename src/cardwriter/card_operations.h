#ifndef A11F1F2B_A461_487F_BE2B_2B247D0B271F
#define A11F1F2B_A461_487F_BE2B_2B247D0B271F

#include <chrono>
#include <ostream>

class Desfire;

class CardOperations final
{
public:
	CardOperations(Desfire& desfire, const std::chrono::milliseconds& timeout) noexcept;

	void GetInformation(std::ostream& out);

private:
	Desfire& desfire;
	std::chrono::milliseconds timeout;
};

#endif /* A11F1F2B_A461_487F_BE2B_2B247D0B271F */
