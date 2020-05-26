//#pragma SYMBOLS

#include "global_vars.h"

signed int iMittelwert;

unsigned char gucSetupNo;
unsigned char gucMyUartAddress;

typVoltages gstVoltages;
typUnsignedWord gstTemperature;
typUnsignedWord gstIvdd;
typSignedWord gstIvgg;
typUnsignedWord gstMwPwrTo;
typUnsignedWord gstMwPwrBack;
typUnsignedWord gstVddMeasured;
typSignedWord gstVggMeasured;
typSignedWord	gstDAC[4];
bool bDeviceSwitchedOnByUart = 0;

unsigned char DataDirection;

unsigned char gucSwitchEvent = SWITCH_EVENT_SWITCH_NOTHING;

uint8_t gucCurrentADChannel;

unsigned char gucTimeout;

unsigned int guiFlags = 0;
unsigned char gucDeviceState=STATE_OFF;

//EEPROM - Parameterablage
__no_init __eeprom unsigned char eeucSetupNo;// = 0;
__no_init __eeprom unsigned char eeucMyUartAddress;// = DEFAULT_UART_ADDRESS;
__no_init __eeprom unsigned char eeucStorageBytes[16];
__no_init __eeprom unsigned char eeucCheckFirstCall;//= 0;
__no_init __eeprom typUnsignedWord eestVddSpark;
__no_init __eeprom typUnsignedWord eestVggSpark;
__no_init __eeprom typUnsignedWord eestSparkDuration;

__no_init __eeprom u32 eeuiVoltages[NO_OF_SETUPS];
__no_init __eeprom unsigned char eeucDacLoByte[4][NO_OF_SETUPS];
__no_init __eeprom unsigned char eeucDacHiByte[4][NO_OF_SETUPS];










