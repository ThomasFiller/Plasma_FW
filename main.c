#include "global_vars.h"
#include "Setup.h"
#include "functions.h"
#include "stm8l15x_flash.h"
#include "Uart.h"
#include "Switch.h"

unsigned char ucCntOvertemp = 0;
unsigned char ucCntOverIgate = 0;
unsigned char ucCntOverIdrain = 0;

void main(void)
{
  typUnsignedWord uwNull;
  SetBit(LED_PORT->ODR,LED_RED_PIN);
  uwNull.uiWord = 0;
  delay(30000);
  delay(30000);
  SystemSetup();
  if(eeucSetupNo<NO_OF_SETUPS) gucSetupNo = eeucSetupNo; else gucSetupNo = 0;
  ReadSetupFromEeprom();//Parameter aus EEPROM einlesen
  delay(30000);
 
  gucCurrentADChannel = 0;

  typSignedWord iTmp;
  iTmp.iWord = 0xFFFF;
  SetDac(7, 0, iTmp); //Interne Referenz einschalten
  
  SetDac(3, DAC_CTRL1, gstDAC[DAC_CTRL1_SPARK]);  //Bereite Steuerspannungen für den ...
  SetDac(3, DAC_CTRL2, gstDAC[DAC_CTRL2_SPARK]);  //...nächsten Zündvorgang vor
  
  //über VGG den Transistor ausschalten (auf VGG_SWITCH setzen), damit nichts passieren kann
  typUnsignedWord stTmp;
  stTmp.uiWord = VGG_SWITCH;
  SetDigPoti(CMD_SET_POTI_CHANNEL_VGG ,stTmp);//Vgg -> -6V
  SetBit(EN_VGG_PORT->ODR,EN_VGG_PIN);   //VGG an 
  
  enableInterrupts();                                                           //Enable general interrupts

  SetBitMask(TIM2->CR1, TIM_CR1_CEN);                                           //Timer2 (timeout) einschalten, damit der erste Initialisierungsinterrupt abgearbeitet wird. Sonst löst Interrupt bei der ersten Kommunikation aus!!!
//  SetBitMask(TIM3->CR1, TIM_CR1_CEN); 		                                //Timer3 an => interner Pulsgenerator 
  SetBitMask(TIM4->CR1, TIM4_CR1_CEN);			                	//Timer4 an => zyklische AD-Wandlung und Mittelung
  while(1)
  {
    if ((USART1->SR) & 0x20)
    {
      UartReceiveOrTransmit();
    }
    else
    {
      if(guiFlags & FLAG_SWITCH_DEVICE)
      {
        guiFlags &= ~FLAG_SWITCH_DEVICE;
        SwitchDevice();
      }
      else
      {
        if(guiFlags & FLAG_CHANGE_SETUP_REQUIRED)
        {
            guiFlags &= ~FLAG_CHANGE_SETUP_REQUIRED;
            ReadSetupFromEeprom();
        }
        else      
        {
          if(guiFlags & FLAG_SAVE_SETUP_REQUIRED)
          {
              guiFlags &= ~FLAG_SAVE_SETUP_REQUIRED;
              SaveSetupToEeprom();
          }
          else      
          {
            if(guiFlags & FLAG_SET_VDD_REQUIRED)
            {
                guiFlags &= ~FLAG_SET_VDD_REQUIRED;
                SetDigPoti(CMD_SET_POTI_CHANNEL_VDD ,gstVoltages.Values.gstVdd);
            }
            else      
            {
              if(guiFlags & FLAG_SET_VGG_REQUIRED)
              {
                  guiFlags &= ~FLAG_SET_VGG_REQUIRED;
                  SetDigPoti(CMD_SET_POTI_CHANNEL_VGG ,gstVoltages.Values.gstVgg);
              }
              else
              {
                if(guiFlags & FLAG_READ_ADC_AND_SWITCH_REQUIRED)//wird von Timer 4 ausgelöst
                {
                    guiFlags &= ~FLAG_READ_ADC_AND_SWITCH_REQUIRED;
                    ReadAdc();//ADC: Messung der Strömme durch und Spannungen an Gate und Drain sowie Leistungsmessungen                    
                    TestLimit(OVERTEMPERATURE, gstTemperature.uiWord, STATE_OVERTEMP, &ucCntOvertemp);  
                    TestLimit(gstIvdd.uiWord, OVERCURRENT_DRAIN, STATE_IVDD_ERROR, &ucCntOverIdrain);
                    if(gstIvgg.iWord>0)
                    {
                      TestLimit( (unsigned int)(gstIvgg.iWord) , OVERCURRENT_GATE, STATE_IVGG_ERROR, &ucCntOverIgate);
                    }
                    ReadSwitch();
                }
                else
                {
                  if(guiFlags & FLAG_BLINK_LED_REQUIRED)//wird von Timer 4 ausgelöst
                  {
                      guiFlags &= ~FLAG_BLINK_LED_REQUIRED;
                      BlinkLed();
                  }
                  else
                  {
                    if(guiFlags & FLAG_SAVE_DIGPOTI_REQUIRED)
                    {
                      guiFlags &= ~FLAG_SAVE_DIGPOTI_REQUIRED;
                      SetDigPoti(CMD_SAVE_CURRENT_VALUES ,uwNull);
                    }
                    else
                    {
                      if(guiFlags & FLAG_SET_CTRL1_REQUIRED)
                      {
                        guiFlags &= ~FLAG_SET_CTRL1_REQUIRED;
                        if(STATE_ON == gucDeviceState) SetDac(3, DAC_CTRL1, gstDAC[DAC_CTRL1]); else SetDac(3, DAC_CTRL1, gstDAC[DAC_CTRL1_SPARK]);//Wenn Plasmaquelle an ist, dann Betriebssteuerspannung, sonst Zündsteuerspannung
                      }
                      else
                      {
                        if(guiFlags & FLAG_SET_CTRL2_REQUIRED)
                        {
                          guiFlags &= ~FLAG_SET_CTRL2_REQUIRED;
                          if(STATE_ON == gucDeviceState) SetDac(3, DAC_CTRL2, gstDAC[DAC_CTRL2]); else SetDac(3, DAC_CTRL2, gstDAC[DAC_CTRL2_SPARK]);//Wenn Plasmaquelle an ist, dann Betriebssteuerspannung, sonst Zündsteuerspannung                         
                        }
                        else
                        {
                          //Kein Flag gesetzt
                        }
                      }
                    }
                  }
                }                            
              }
            }              
          }          
        }
      }
    }
  }
}