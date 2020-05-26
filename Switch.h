#ifndef __Service_H_included
#define __Service_H_included

#include "stm8l15x.h"

enum voltage_source {
  voltage_source_accu,
  voltage_source_5v_usb,
  voltage_source_24v
};

extern unsigned char readSwitchResult;
extern enum voltage_source voltage_source;
//#pragma inline=forced
extern void fill_accu_voltage_array(void);
extern  void WriteRtc();//schreibe RTC mit dem Inhalt des Array


extern void ReadInitAndEvaluateAdc(void);
extern void SwitchLed(unsigned char ucLedNumber);
extern void ReadSwitch(void);
extern void LedDeviceType(unsigned char ucOldType, unsigned char ucType);
extern void LedConnectionType(unsigned char ucOldType, unsigned char ucType);
extern void ReadRtc();//lies Real Time Clock und schreibe Ergebnisse ins Array
extern void GoSleep();
extern void Halt_Init(void);
extern void Halt_DeInit(void);
void set_accu_led(void);
void LedOff(unsigned char ucLed);

#endif
