#include "global_vars.h"
#include "globalconsts.h"


void ReadSwitch()
{
#define CLICK_DELAY       100 //Entprellung

static unsigned int uiCntUntilSwitch = 0;
bool bSwitchState = ValBit(SWITCH_PORT->IDR,SWITCH_PIN);

  switch (gucDeviceState)
  {
    case STATE_OFF:
      if(bSwitchState)//Taster gedrueckt=>POWER_SWITCH_PORT=0
      {//Taster gedrueckt
        if((uiCntUntilSwitch++) > CLICK_DELAY)
        {//Taster wurde ueber einen Zeitraum von CLICK_DELAY (Entprellung) gedrueckt
          gucSwitchEvent = SWITCH_EVENT_SWITCH_ON;
          uiCntUntilSwitch = 0;
        }
      }
      else
      {//Taster nicht gedrueckt
        uiCntUntilSwitch = 0;
      }
      break;
      
    case STATE_ON:
      if((!bSwitchState) & (!bDeviceSwitchedOnByUart))
      {//Taster nicht gedrueckt
        if((uiCntUntilSwitch++) > CLICK_DELAY)
        {//Taster wurde ueber einen Zeitraum von CLICK_DELAY (Entprellung) gedrueckt
          gucSwitchEvent = SWITCH_EVENT_SWITCH_OFF;
          uiCntUntilSwitch = 0;
        }
      }
      else
      {//Taster gedrueckt
        uiCntUntilSwitch = 0;
      }
      break;
   default:
          break;
  }
}