#include "global_vars.h"

void SetDigPoti(unsigned char ucCmd ,typUnsignedWord uiDigPotValue);
void ReadAdc();
void ReadSettings();
void Communication();
extern void delay(unsigned int us);
void BlinkLed();
extern void SwitchDevice();
extern void TestLimit(unsigned int uiValue, unsigned int uiLimit, unsigned char ucErrorDeviceState, unsigned char *ucCntLimit);
void SetDac(unsigned char ucCmd, unsigned char ucDacAddress ,typSignedWord iDacValue);
