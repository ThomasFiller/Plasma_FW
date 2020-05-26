#ifndef __Global_Vars_H_included
#define __Global_Vars_H_included

#include "globalconsts.h"

//***************type definitions for a combined 8-bit/16-bit and byte/word access*******************
//used for 16-bit calculations and 8-bit register loading
typedef struct
{
	unsigned char	ucHighByte;
	unsigned char	ucLowByte;
} typDoubleByte;

typedef union
{
	unsigned int	uiWord;
	typDoubleByte	stBytes;
} typUnsignedWord;

typedef union
{
	signed int	iWord;
	typDoubleByte	stBytes;
} typSignedWord;

typedef struct
{		
  typUnsignedWord	gstVdd;
  typUnsignedWord	gstVgg;	
} typVddVgg;

typedef union
{		
  typVddVgg     Values;
  u32           ulLong;
} typVoltages;

//****************************************************************************************************
extern unsigned char gucMyUartAddress;
extern unsigned char gucSetupNo;

extern typVoltages gstVoltages;

extern typUnsignedWord gstTemperature;
extern typUnsignedWord gstIvdd;
extern typUnsignedWord gstVddMeasured;
extern typSignedWord gstIvgg;
extern typSignedWord gstVggMeasured;
extern typUnsignedWord gstMwPwrTo;
extern typUnsignedWord gstMwPwrBack;
extern typSignedWord	gstDAC[4];
extern bool bDeviceSwitchedOnByUart;

extern signed int iMittelwert;
extern unsigned char ucComPossible;
extern unsigned char DataDirection;

extern unsigned char gucSwitchEvent;

extern unsigned char gucHfCtl;
extern uint8_t gucCurrentADChannel;
extern unsigned char gucTimeout;
extern unsigned int guiFlags;
extern unsigned char gucDeviceState;

//EEPROM - Parameterablage
extern __no_init __eeprom unsigned char eeucSetupNo;
extern __no_init __eeprom unsigned char eeucMyUartAddress;
extern __no_init __eeprom unsigned char eeucStorageBytes[16];
extern __no_init __eeprom unsigned char eeucCheckFirstCall;//= 0;
extern __no_init __eeprom typUnsignedWord eestVddSpark;
extern __no_init __eeprom typUnsignedWord eestVggSpark;
extern __no_init __eeprom typUnsignedWord eestSparkDuration;

extern __no_init __eeprom u32 eeuiVoltages[NO_OF_SETUPS];
extern __no_init __eeprom unsigned char eeucDacLoByte[4][NO_OF_SETUPS];
extern __no_init __eeprom unsigned char eeucDacHiByte[4][NO_OF_SETUPS];


#endif