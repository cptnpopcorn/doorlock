
#ifndef CLASSIC_H
#define CLASSIC_H

#include "PN532.h"

// ---------------------------------------------------------------

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_STORE                    (0xC2)
#define MIFARE_ULTRALIGHT_CMD_WRITE         (0xA2)

// ---------------------------------------------------------------

class Classic : public PN532
{
  public:
    bool DumpCardMemory(char s8_KeyType, const uint8_t* u8_Keys, bool b_ShowAccessBits);
    bool AuthenticateDataBlock(uint8_t u8_Block, char s8_KeyType, const uint8_t* u8_KeyData, const uint8_t* u8_Uid, uint8_t u8_UidLen);
    bool ReadDataBlock (uint8_t u8_Block, uint8_t* u8_Data);
    bool WriteDataBlock(uint8_t u8_Block, uint8_t* u8_Data);
    bool GetValue(uint8_t* u8_Data, uint32_t* pu32_Value, uint8_t* pu8_Address);
    void SetValue(uint8_t* u8_Data, uint32_t   u32_Value, uint8_t u8_Address);
   
 private:
    bool DataExchange(uint8_t u8_Command, uint8_t u8_Block, uint8_t* u8_Data, uint8_t u8_DataLen);
    void ShowAccessBits(uint8_t u8_Block, uint8_t u8_Byte7, uint8_t u8_Byte8);    
};

#endif
