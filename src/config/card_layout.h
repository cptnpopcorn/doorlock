#ifndef F59D66CF_C2F4_415E_8FED_DAA916BC1713
#define F59D66CF_C2F4_415E_8FED_DAA916BC1713

#include <cstdint>

enum class KeyNumberPicc : uint8_t {
	Master = 0x00,
};

enum class KeyVersionPicc : uint8_t {
	Master = 0x00,
};

enum class KeyNumberDoorlock : uint8_t {
	Master = 0x00,
	Write = 0x01,
	Read = 0x02,
};

enum class KeyVersionDoorlock : uint8_t {
	Master = 0x00,
	Write = 0x00,
	Read = 0x00,
};

enum class ApplicationId : uint32_t {
	Picc = 0x00000000,
	Doorlock = 0x4B434C44, // "DLCK"
};

#endif /* F59D66CF_C2F4_415E_8FED_DAA916BC1713 */
