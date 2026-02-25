#ifndef B958E73E_7113_4102_B7BE_31EAB265EB11
#define B958E73E_7113_4102_B7BE_31EAB265EB11

#include <cstdint>
#include <span>

class ConstStream final
{
public:
	ConstStream(const std::span<const uint8_t>& chars) noexcept;

	typedef char Ch;
	Ch Peek() const;
	Ch Take();
	size_t Tell() const;
	Ch* PutBegin();
 	void Put(Ch);
 	void Flush();
 	size_t PutEnd(Ch*);

private:
	size_t i;
	const std::span<const uint8_t>& chars;
};

#endif /* B958E73E_7113_4102_B7BE_31EAB265EB11 */
