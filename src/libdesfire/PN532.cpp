/**************************************************************************
    @file     PN532.cpp
    @author   Adafruit Industries, Elmü
    @license  BSD (see license.txt)

	  Driver for NXP's PN532 NFC/13.56MHz RFID Transceiver

	  This is a library for the Adafruit PN532 NFC/RFID breakout board.
	  This library works with the Adafruit NFC breakout
	  ----> https://www.adafruit.com/products/364

	  Check out the links above for our tutorials and wiring diagrams
	  These chips use SPI or I2C to communicate.

	  Adafruit invests time and resources providing this open source code,
	  please support Adafruit and open-source hardware by purchasing products from Adafruit!

    ----------------------------------------------------------
    April 2016, modifications by Elmü:
    The code from Adafruit was a VERY SLOPPY code just for testing and playing around but not usable for production.
    It has been rewritten completely by Elmü.
    IRQ is not required anymore in I2C mode. Now the software handshake is used instead.
    Removed all compiler warnings that appeared when compiling Adafruit code.
    Bugfix: Adafruit used strncmp() to compare binary data which is completey wrong -> replaced with memcmp()
    Bugfix: (Severe bug) Adafruit code does not check for valid return packets. The checksum is completely irgnored. Bytes received before the start code are not skipped!
    Bugfix: (Severe bug) Adafruit code used a timeout = 0 (wait forever). This is completely wrong. If the chip does not respond, the code hangs forever!
    Bugfix: Adafruit code does not allow to distinguish why readPassiveTargetID() returns false. (because there is no card or because of communication problem?)
    Added support for Value blocks (in Classic.cpp)
    Added memory Dump (in Classic.cpp)
    AuthenticateDataBlock(), ReadDataBlock() and WriteDataBlock() rewritten (in Classic.cpp)
    Implemented the correct wake up procedure (sending PN532_WAKEUP) instead of sending getFirmwareVersion.
    Debug output was buggy: The checksum bytes were displayed as 0xFFFFFFFC instead of 0xFC and removed useless "0x" before each byte.
    Detailed debug output was missing. Added display of valid data bytes inside the packet.
    SPI slow speed added (using Software SPI to get 10kHz clock)
    getFirmwareVersion() was a very clumsy and cryptic command -> completely rewritten
    writeGPIO() rewritten -> no warning about wrong usage anymore.
    setPassiveActivationRetries() did not have any error checking at all.
    Ugly code in writecommand() completely rewritten
    Crappy code like this removed:   int offset = mb_UsingSPI ? 5 : 6;
    ----------------------------------------------------------

    Check for a new version on http://www.codeproject.com/Articles/1096861/DIY-electronic-RFID-Door-Lock-with-Battery-Backup
    
**************************************************************************/

#include "PN532.h"
#include "PN532Interface.h"
#include "ControllerFrameWriter.h"
#include "FrameWriterFuncs.h"
#include "TargetDataWriterFuncs.h"
#include "TargetDataValidatorFuncs.h"
#include "NullTargetDataWriter.h"
#include "NullTargetDataValidator.h"

#include <algorithm>
#include <span>

using namespace std;

/**************************************************************************
    Constructor
**************************************************************************/
PN532::PN532(PN532Interface& interface) noexcept : interface{interface}
{
}

/**************************************************************************
    Reset the PN532, wake up and start communication
**************************************************************************/
void PN532::begin() 
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** begin()\r\n");

    interface.DeassertResetAndPowerUp(); // RSTPD_N HIGH
    Utils::DelayMilli(10);
    interface.AssertResetAndPowerDown(); // RSTPD_N LOW
    Utils::DelayMilli(400);
    interface.DeassertResetAndPowerUp(); // RSTPD_N HIGH
    Utils::DelayMilli(10);  // Small delay required before taking other actions after reset. See datasheet section 12.23, page 209.

	interface.StartDataTransport();
}

/**************************************************************************
    Enable / disable debug output to SerialClass
    0 = Off, 1 = high level debug, 2 = low level debug (more details)
**************************************************************************/
void PN532::SetDebugLevel(uint8_t level)
{
    mu8_DebugLevel = level;
}

/**************************************************************************
    Gets the firmware version of the PN5xx chip
    returns:
    pIcType = Version of the IC. For PN532, this byte is 0x32
    pVersionHi, pVersionLo = Firmware version
    pFlags, bit 0 = Support of ISO 14443A
    pFlags, bit 1 = Support of ISO 14443B
    pFlags, bit 2 = Support of ISO 18092
**************************************************************************/
bool PN532::GetFirmwareVersion(uint8_t* pIcType, uint8_t* pVersionHi, uint8_t* pVersionLo, uint8_t* pFlags) 
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** GetFirmwareVersion()\r\n");
    
    mu8_PacketBuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    if (!SendCommandCheckAck(mu8_PacketBuffer, 1))
        return 0;

    uint8_t len = ReadData(mu8_PacketBuffer, 5);
    if (len != 5 || mu8_PacketBuffer[0] != PN532_COMMAND_GETFIRMWAREVERSION + 1)
    {
        Utils::Print("GetFirmwareVersion failed\r\n");
        return false;
    }

    *pIcType    = mu8_PacketBuffer[1];
    *pVersionHi = mu8_PacketBuffer[2];
    *pVersionLo = mu8_PacketBuffer[3];
    *pFlags     = mu8_PacketBuffer[4];
    return true;
}

/**************************************************************************
    Configures the SAM (Secure Access Module)
**************************************************************************/
bool PN532::SamConfig(void)
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** SamConfig()\r\n");
  
    mu8_PacketBuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    mu8_PacketBuffer[1] = 0x01; // normal mode;
    mu8_PacketBuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    mu8_PacketBuffer[3] = 0x01; // use IRQ pin!
  
    if (!SendCommandCheckAck(mu8_PacketBuffer, 4))
        return false;
  
    uint8_t len = ReadData(mu8_PacketBuffer, 1);
    if (len != 1 || mu8_PacketBuffer[0] != PN532_COMMAND_SAMCONFIGURATION + 1)
    {
        Utils::Print("SamConfig failed\r\n");
        return false;
    }
    return true;
}

/**************************************************************************
    Sets the amount of reties that the PN532 tries to activate a target
**************************************************************************/
bool PN532::SetPassiveActivationRetries() 
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** SetPassiveActivationRetries()\r\n");
  
    mu8_PacketBuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    mu8_PacketBuffer[1] = 5;    // Config item 5 (MaxRetries)
    mu8_PacketBuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
    mu8_PacketBuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
    mu8_PacketBuffer[4] = 3;    // one retry is enough for Mifare Classic but Desfire is slower (if you modify this, you must also modify PN532_TIMEOUT!)
    
    if (!SendCommandCheckAck(mu8_PacketBuffer, 5))
        return false;
  
    uint8_t len = ReadData(mu8_PacketBuffer, 1);
    if (len != 1 || mu8_PacketBuffer[0] != PN532_COMMAND_RFCONFIGURATION + 1)
    {
        Utils::Print("SetPassiveActivationRetries failed\r\n");
        return false;
    }
		
	return true;
}

/**************************************************************************
    Turns the RF field off.
    When the field is on, the PN532 consumes approx 110 mA
    When the field is off, the PN532 consumes approx 18 mA
    The RF field is turned on again by ReadPassiveTargetID().
**************************************************************************/
bool PN532::SwitchOffRfField() 
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** SwitchOffRfField()\r\n");
  
    mu8_PacketBuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    mu8_PacketBuffer[1] = 1; // Config item 1 (RF Field)
    mu8_PacketBuffer[2] = 0; // Field Off
    
    if (!SendCommandCheckAck(mu8_PacketBuffer, 3))
        return false;
  
    uint8_t len = ReadData(mu8_PacketBuffer, 1);
    if (len != 1 || mu8_PacketBuffer[0] != PN532_COMMAND_RFCONFIGURATION + 1)
    {
        Utils::Print("SwitchOffRfField failed\r\n");
        return false;
    }
    return true;
}

/**************************************************************************/
/*!
    Writes an 8-bit value that sets the state of the PN532's GPIO pins

    All pins that can not be used as GPIO should ALWAYS be left high
    (value = 1) or the system will become unstable and a HW reset
    will be required to recover the PN532.

    pinState[0] (01) = P30   Can be used as GPIO
    pinState[1] (02) = P31   Can be used as GPIO
    pinState[2] (04) = P32   *** RESERVED (Must be set) ***
    pinState[3] (08) = P33   Can be used as GPIO
    pinState[4] (10) = P34   *** RESERVED (Must be set) ***
    pinState[5] (20) = P35   Can be used as GPIO

    This function is not used. The original intention was to drive a LED that 
    is connected to the PN532 board. But the pins deliver so few current 
    that a connected LED is very dark. (even if connected without resistor!)
    Additionally the red LED cannot be connected to the PN532 because it should
    flash if there is a communication error with the PN532. But if there is a
    communication problem the command WRITEGPIO will never arrive at the PN532
    and the red LED would never flash.
*/
/**************************************************************************/
bool PN532::WriteGPIO(bool P30, bool P31, bool P33, bool P35)
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** WriteGPIO()\r\n");
  
    uint8_t pinState = (P30 ? PN532_GPIO_P30 : 0) |
                    (P31 ? PN532_GPIO_P31 : 0) |
                           PN532_GPIO_P32      |
                    (P33 ? PN532_GPIO_P33 : 0) |
                           PN532_GPIO_P34      |
                    (P35 ? PN532_GPIO_P35 : 0);

    mu8_PacketBuffer[0] = PN532_COMMAND_WRITEGPIO;
    mu8_PacketBuffer[1] = PN532_GPIO_VALIDATIONBIT | pinState;  // P3 Pins
    mu8_PacketBuffer[2] = 0x00;                                 // P7 GPIO Pins (not used ... taken by SPI)
                    
    if (!SendCommandCheckAck(mu8_PacketBuffer, 3))
        return false;
  
    uint8_t len = ReadData(mu8_PacketBuffer, 1);
    if (len != 1 || mu8_PacketBuffer[0] != PN532_COMMAND_WRITEGPIO + 1)
    {
        Utils::Print("WriteGPIO failed\r\n");
        return false;
    }
    return true;
}


/**************************************************************************
    Waits for an ISO14443A target to enter the field.
    If the RF field has been turned off before, this command switches it on.

    param u8_UidBuffer  Pointer to an 8 byte buffer that will be populated with the card's UID (4 or 7 bytes)
    param pu8_UidLength Pointer to the variable that will hold the length of the card's UID.
    param pe_CardType   Pointer to the variable that will hold if the card is a Desfire card
    
    returns false only on error!
    returns true and *UidLength = 0 if no card was found
    returns true and *UidLength > 0 if a card has been read successfully
**************************************************************************/
bool PN532::ReadPassiveTargetID(uint8_t* u8_UidBuffer, uint8_t* pu8_UidLength, eCardType* pe_CardType) 
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** ReadPassiveTargetID()\r\n");
      
    *pu8_UidLength = 0;
    *pe_CardType   = CARD_Unknown;
    std::memset(u8_UidBuffer, 0, 8);
      
    mu8_PacketBuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    mu8_PacketBuffer[1] = 1;  // read data of 1 card (The PN532 can read max 2 targets at the same time)
    mu8_PacketBuffer[2] = CARD_TYPE_106KB_ISO14443A; // This function currently does not support other card types.
  
    if (!SendCommandCheckAck(mu8_PacketBuffer, 3))
        return false; // Error (no valid ACK received or timeout)
  
    /* 
    ISO14443A card response:
    mu8_PacketBuffer Description
    -------------------------------------------------------
    b0               4B (always) (PN532_COMMAND_INLISTPASSIVETARGET + 1)
    b1               Amount of cards found
    b2               Tag number (always 1)
    b3,4             SENS_RES (ATQA = Answer to Request Type A)
    b5               SEL_RES  (SAK  = Select Acknowledge)
    b6               UID Length
    b7..Length       UID (4 or 7 bytes)
    nn               ATS Length     (Desfire only)
    nn..Length-1     ATS data bytes (Desfire only)
    */ 
    uint8_t len = ReadData(mu8_PacketBuffer, 27);
    if (len < 2 || mu8_PacketBuffer[0] != PN532_COMMAND_INLISTPASSIVETARGET + 1)
    {
        Utils::Print("ReadPassiveTargetID failed\r\n");
        return false;
    }   

    uint8_t cardsFound = mu8_PacketBuffer[1]; 
    if (mu8_DebugLevel > 0)
    {
        Utils::Print("Cards found: "); 
        Utils::PrintDec(cardsFound, LF); 
    }
    if (cardsFound != 1)
        return true; // no card found -> this is not an error!

    uint8_t u8_IdLength = mu8_PacketBuffer[6];
    if (u8_IdLength != 4 && u8_IdLength != 7)
    {
        Utils::Print("Card has unsupported UID length: ");
        Utils::PrintDec(u8_IdLength, LF); 
        return true; // unsupported card found -> this is not an error!
    }   

    std::memcpy(u8_UidBuffer, mu8_PacketBuffer + 7, u8_IdLength);    
    *pu8_UidLength = u8_IdLength;

    // See "Mifare Identification & Card Types.pdf" in the ZIP file
    uint16_t u16_ATQA = ((uint16_t)mu8_PacketBuffer[3] << 8) | mu8_PacketBuffer[4];
    uint8_t     u8_SAK   = mu8_PacketBuffer[5];

    if (u8_IdLength == 7 && u8_UidBuffer[0] != 0x80 && u16_ATQA == 0x0344 && u8_SAK == 0x20) *pe_CardType = CARD_Desfire;
    if (u8_IdLength == 4 && u8_UidBuffer[0] == 0x80 && u16_ATQA == 0x0304 && u8_SAK == 0x20) *pe_CardType = CARD_DesRandom;
    
    if (mu8_DebugLevel > 0)
    {
        Utils::Print("Card UID:    ");
        Utils::PrintHexBuf(u8_UidBuffer, u8_IdLength, LF);

        // Examples:              ATQA    SAK  UID length
        // MIFARE Mini            00 04   09   4 bytes
        // MIFARE Mini            00 44   09   7 bytes
        // MIFARE Classic 1k      00 04   08   4 bytes
        // MIFARE Classic 4k      00 02   18   4 bytes
        // MIFARE Ultralight      00 44   00   7 bytes
        // MIFARE DESFire Default 03 44   20   7 bytes
        // MIFARE DESFire Random  03 04   20   4 bytes
        // See "Mifare Identification & Card Types.pdf"
        char s8_Buf[80];
        sprintf(s8_Buf, "Card Type:   ATQA= 0x%04X, SAK= 0x%02X", u16_ATQA, u8_SAK);

        if (*pe_CardType == CARD_Desfire)   strcat(s8_Buf, " (Desfire Default)");
        if (*pe_CardType == CARD_DesRandom) strcat(s8_Buf, " (Desfire RandomID)");
            
        Utils::Print(s8_Buf, LF);
    }
    return true;
}

/**************************************************************************
    The goal of this command is to select the target. (Initialization, anti-collision loop and Selection)
    If the target is already selected, no action is performed and Status OK is returned. 
**************************************************************************/
bool PN532::SelectCard()
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** SelectCard()\r\n");
  
    mu8_PacketBuffer[0] = PN532_COMMAND_INSELECT;
    mu8_PacketBuffer[1] = 1; // Target 1

    if (!SendCommandCheckAck(mu8_PacketBuffer, 2))
        return false;
  
    uint8_t len = ReadData(mu8_PacketBuffer, 9);
    if (len < 2 || mu8_PacketBuffer[0] != PN532_COMMAND_INSELECT + 1)
    {
        Utils::Print("Select failed\r\n");
        return false;
    }

    return CheckPN532Status(mu8_PacketBuffer[1]);
}

/**************************************************************************
    The goal of this command is to deselect the target. 
    The PN532 keeps all the information relative to this target (also certain error status).  
    This function is required due to a stupid behaviour with Mifare Classic:
    When AuthenticateDataBlock() has failed for a sector, you also get an
    authentication error for the next sector although you have passed the correct key.
    So, after an authentication error you must first deselect the card before
    authenticating a new sector!
**************************************************************************/
bool PN532::DeselectCard()
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** DeselectCard()\r\n");
  
    mu8_PacketBuffer[0] = PN532_COMMAND_INDESELECT;
    mu8_PacketBuffer[1] = 0; // Deselect all cards

    if (!SendCommandCheckAck(mu8_PacketBuffer, 2))
        return false;
  
    uint8_t len = ReadData(mu8_PacketBuffer, 9);
    if (len < 2 || mu8_PacketBuffer[0] != PN532_COMMAND_INDESELECT + 1)
    {
        Utils::Print("Deselect failed\r\n");
        return false;
    }

    return CheckPN532Status(mu8_PacketBuffer[1]);
}

/**************************************************************************
    The goal of this command is to release the target.
    Releasing a target means that the host controller has finished the communication with 
    the target, so the PN532 erases all the information relative to it. 
**************************************************************************/
bool PN532::ReleaseCard()
{
    if (mu8_DebugLevel > 0) Utils::Print("\r\n*** ReleaseCard()\r\n");
  
    mu8_PacketBuffer[0] = PN532_COMMAND_INRELEASE;
    mu8_PacketBuffer[1] = 0; // Deselect all cards

    if (!SendCommandCheckAck(mu8_PacketBuffer, 2))
        return false;
  
    uint8_t len = ReadData(mu8_PacketBuffer, 9);
    if (len < 2 || mu8_PacketBuffer[0] != PN532_COMMAND_INRELEASE + 1)
    {
        Utils::Print("Release failed\r\n");
        return false;
    }

    return CheckPN532Status(mu8_PacketBuffer[1]);
}

/**************************************************************************
    This function is private
    It checks the status byte that is returned by some commands.
    See chapter 7.1 in the manual.
    u8_Status = the status byte
**************************************************************************/
bool PN532::CheckPN532Status(uint8_t u8_Status)
{
    // Bits 0...5 contain the error code.
    u8_Status &= 0x3F;

    if (u8_Status == 0)
        return true;

    char s8_Buf[50];
    sprintf(s8_Buf, "PN532 Error 0x%02X: ", u8_Status);
    Utils::Print(s8_Buf);

    switch (u8_Status)
    {
        case 0x01: 
            Utils::Print("Timeout\r\n");
            return false;
        case 0x02: 
            Utils::Print("CRC error\r\n");
            return false;
        case 0x03: 
            Utils::Print("Parity error\r\n");
            return false;
        case 0x04: 
            Utils::Print("Wrong bit count during anti-collision\r\n");
            return false;
        case 0x05: 
            Utils::Print("Framing error\r\n");
            return false;
        case 0x06: 
            Utils::Print("Abnormal bit collision\r\n");
            return false;
        case 0x07: 
            Utils::Print("Insufficient communication buffer\r\n");
            return false;
        case 0x09: 
            Utils::Print("RF buffer overflow\r\n");
            return false;
        case 0x0A: 
            Utils::Print("RF field has not been switched on\r\n");
            return false;
        case 0x0B: 
            Utils::Print("RF protocol error\r\n");
            return false;
        case 0x0D: 
            Utils::Print("Overheating\r\n");
            return false;
        case 0x0E: 
            Utils::Print("Internal buffer overflow\r\n");
            return false;
        case 0x10: 
            Utils::Print("Invalid parameter\r\n");
            return false;
        case 0x12: 
            Utils::Print("Command not supported\r\n");
            return false;
        case 0x13: 
            Utils::Print("Wrong data format\r\n");
            return false;
        case 0x14:
            Utils::Print("Authentication error\r\n");
            return false;
        case 0x23:
            Utils::Print("Wrong UID check byte\r\n");
            return false;
        case 0x25:
            Utils::Print("Invalid device state\r\n");
            return false;
        case 0x26:
            Utils::Print("Operation not allowed\r\n");
            return false;
        case 0x27:
            Utils::Print("Command not acceptable\r\n");
            return false;
        case 0x29:
            Utils::Print("Target has been released\r\n");
            return false;
        case 0x2A:
            Utils::Print("Card has been exchanged\r\n");
            return false;
        case 0x2B:
            Utils::Print("Card has disappeared\r\n");
            return false;
        case 0x2C:
            Utils::Print("NFCID3 initiator/target mismatch\r\n");
            return false;
        case 0x2D:
            Utils::Print("Over-current\r\n");
            return false;
        case 0x2E:
            Utils::Print("NAD msssing\r\n");
            return false;
        default:
            Utils::Print("Undocumented error\r\n");
            return false;
    }
}

// ########################################################################
// ####                      LOW LEVEL FUNCTIONS                      #####
// ########################################################################

/**************************************************************************
    Sends a command and waits a specified period for the ACK
    param cmd       Pointer to the command buffer
    param cmdlen    The size of the command in bytes

    returns  true  if everything is OK, 
             false if timeout occured before an ACK was recieved
**************************************************************************/
bool PN532::SendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen) 
{
    WriteCommand(cmd, cmdlen);
    return ReadAck();
}

/**************************************************************************
    Writes a command to the PN532, inserting the
    preamble and required frame details (checksum, len, etc.)

    param  cmd       Command buffer
    param  cmdlen    Command length in bytes
**************************************************************************/
void PN532::WriteCommand(uint8_t* cmd, uint8_t cmdlen)
{
	interface.WriteFrame().DataFromHost(span<uint8_t>{cmd, cmdlen});
	this_thread::sleep_for(1ms);
}

/**************************************************************************
    Read the ACK packet (acknowledge)
**************************************************************************/
bool PN532::ReadAck() // TODO: if there is something else than ACK or corruption, we should just abandon everything
{
	try
	{
		auto ack_writer = make_frame_writer(
			[]{ throw runtime_error{"LCS invalid"}; },
			[]{}, // the ACK we are looking for
			[]{ throw runtime_error{"NACKed"}; }, // well: it seems for some operations NACK is expected - that should be specified as a parameter
			[]{ throw runtime_error{"TFI invalid"}; },
			[] -> TargetDataWriter& { throw runtime_error{"unexpected data"}; },
			[] -> TargetDataValidator& { throw runtime_error{"unexpected error"}; },
			[]{});

		interface.ReadFrame(ack_writer);
	}
	catch (exception e)
	{
		return false;
	}

	return true;
}

/**************************************************************************
    Reads n bytes of data from the PN532 via SPI or I2C and checks for valid data.
    param  buff      Pointer to the buffer where data will be written
    param  len       Number of bytes to read
    returns the number of bytes that have been copied to buff (< len) or 0 on error
**************************************************************************/
uint8_t PN532::ReadData(uint8_t* buff, uint8_t len) 
{	
	auto validate = make_target_data_validator(
		[]{},
		[]{ throw runtime_error{"DCS invalid"}; });

	size_t write_pos {0};

	auto write_data = make_target_data_writer(
		[buff = as_const(buff), len = as_const(len), &write_pos](const span<uint8_t const>& data)
		{
			const auto fill_length = min(len - write_pos, data.size());
			ranges::copy(data.subspan(0, fill_length), buff + write_pos);
			write_pos += fill_length;
			return data.size();
		},
		[&validate] -> auto&& { return validate; });

	auto frame_writer = make_frame_writer(
		[]{ throw runtime_error{"LCS invalid"}; },
		[]{}, // the ACK we are looking for
		[]{ throw runtime_error{"NACKed"}; },
		[]{ throw runtime_error{"TFI invalid"}; },
		[&write_data] -> auto& { return write_data; },
		[] -> TargetDataValidator& { throw runtime_error{"unexpected error"}; },
		[]{ throw runtime_error{"frame incomplete"}; });

	if (!interface.ReadFrame(frame_writer)) return 0;

	this_thread::sleep_for(2ms);

	interface.WriteFrame().Ack();

	this_thread::sleep_for(5ms);

	return static_cast<uint8_t>(write_pos);
}
