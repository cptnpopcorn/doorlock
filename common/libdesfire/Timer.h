#ifndef C5CB2226_AB2C_4E75_AEA0_71C61F282E5B
#define C5CB2226_AB2C_4E75_AEA0_71C61F282E5B

#include <chrono>

class Timer final
{
public:
	Timer(const std::chrono::milliseconds& duration);

	bool IsRunning() const noexcept;
	void Update();

private:
	Timer(std::chrono::system_clock::time_point now, const std::chrono::milliseconds& duration) noexcept;
	std::chrono::system_clock::time_point now;
	std::chrono::system_clock::time_point deadline;
};

#endif /* C5CB2226_AB2C_4E75_AEA0_71C61F282E5B */
