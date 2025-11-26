#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <chrono>
#include <thread>

#define LF  "\r\n" // LineFeed 

// USB connection to Terminal program (Teraterm) on PC via COM port
// You can leave all functions empty and only redirect Print() to printf().
class SerialClass
{  
public:
    // Create a COM connection via USB.
    // Teensy ignores the baudrate parameter (only for older Arduino boards)
    static inline void Begin(uint32_t u32_Baud) 
    {
        // Serial.begin(u32_Baud);
    }
    // returns how many characters the user has typed in the Terminal program on the PC which have not yet been read with Read()
    static inline int Available()
    {
        return 0;
        // return Serial.available();
    }
    // Get the next character from the Terminal program on the PC
    // returns -1 if no character available
    static inline int Read()
    {
        return -1;
        //return Serial.read();
    }
    // Print text to the Terminal program on the PC
    // On Windows/Linux use printf() here to write debug output an errors to the Console.
    static inline void Print(const char* s8_Text)
    {
        //Serial.print(s8_Text);
    }
};

// -------------------------------------------------------------------------------------------------------------------

class Utils
{
public:
    // returns the current tick counter
    // If you compile on Visual Studio see WinDefines.h
    static inline uint32_t GetMillis()
    {
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }

    // If you compile on Visual Studio see WinDefines.h
    static inline void DelayMilli(int s32_MilliSeconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(s32_MilliSeconds));
    }

    // This function is only required for Software SPI mode.
    // If you compile on Visual Studio see WinDefines.h
    static inline void DelayMicro(int s32_MicroSeconds)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(s32_MicroSeconds));
    }

    static uint64_t GetMillis64();
    static void     Print(const char*   s8_Text,  const char* s8_LF=NULL);
    static void     PrintDec  (int      s32_Data, const char* s8_LF=NULL);
    static void     PrintHex8 (uint8_t     u8_Data,  const char* s8_LF=NULL);
    static void     PrintHex16(uint16_t u16_Data, const char* s8_LF=NULL);
    static void     PrintHex32(uint32_t u32_Data, const char* s8_LF=NULL);
    static void     PrintHexBuf(const uint8_t* u8_Data, const uint32_t u32_DataLen, const char* s8_LF=NULL, int s32_Brace1=-1, int S32_Brace2=-1);
    static void     PrintInterval(uint64_t u64_Time, const char* s8_LF=NULL);
    static void     GenerateRandom(uint8_t* u8_Random, int s32_Length);
    static void     RotateBlockLeft(uint8_t* u8_Out, const uint8_t* u8_In, int s32_Length);
    static void     BitShiftLeft(uint8_t* u8_Data, int s32_Length);
    static void     XorDataBlock(uint8_t* u8_Out,  const uint8_t* u8_In, const uint8_t* u8_Xor, int s32_Length);    
    static void     XorDataBlock(uint8_t* u8_Data, const uint8_t* u8_Xor, int s32_Length);
    static uint16_t CalcCrc16(const uint8_t* u8_Data,  int s32_Length);
    static uint32_t CalcCrc32(const uint8_t* u8_Data1, int s32_Length1, const uint8_t* u8_Data2=NULL, int s32_Length2=0);
    static int      strnicmp(const char* str1, const char* str2, uint32_t u32_MaxCount);
    static int      stricmp (const char* str1, const char* str2);

private:
    static uint32_t CalcCrc32(const uint8_t* u8_Data, int s32_Length, uint32_t u32_Crc);
};

#endif // UTILS_H
