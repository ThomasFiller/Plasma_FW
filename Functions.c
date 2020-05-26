#include "stm8l15x_gpio.h"
#include "functions.h"

void SwitchDeviceOff();
void SwitchDeviceOn();

void SetDac(unsigned char ucCmd, unsigned char ucDacAddress ,typSignedWord iDacValue)
{
unsigned char aucSendByte[3];
  ucCmd         &= 0x07;
  aucSendByte[0]  = ucDacAddress & (0x07);
  aucSendByte[0] |= (ucCmd<<3);
  aucSendByte[1] = iDacValue.stBytes.ucHighByte;
  aucSendByte[2] = iDacValue.stBytes.ucLowByte;
  
  ClrBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   aus (SCLK)
  nop();nop();nop();nop();
  ClrBit(SPI_nCS_DAC_PORT->ODR,SPI_nCS_DAC_PIN);		// Port   aus (nSYNC vom DAC AD5624)
  
  for(unsigned char ucTmp=0; ucTmp<3; ucTmp++)
  {
    for(unsigned char ucBitCnt = 0; ucBitCnt<8; ucBitCnt++)
    {
      SetBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   an (SCLK vom DAC AD5624)
      if(aucSendByte[ucTmp] & (0x01<<(7-ucBitCnt)))
      {
          SetBit(SPI_MOSI_PORT->ODR,SPI_MOSI_PIN);        // Port an (DIN vom DAC AD5624)
      }
      else
      {
          ClrBit(SPI_MOSI_PORT->ODR,SPI_MOSI_PIN);        // Port aus (DIN vom DAC AD5624)       
      }
      ClrBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   an (SCLK vom DAC AD5624)
    }
  }
  
  while(CheckBit1( SPI1->SR, SPI_SR_BSY ));//Warte, bis  transmission of the last data is complete  
  SetBit(SPI_nCS_DAC_PORT->ODR,SPI_nCS_DAC_PIN);        // Port   an (nSYNC vom DAC AD5624) 
}

void SwitchDevice()
{
  if (SWITCH_EVENT_SWITCH_OFF == gucSwitchEvent)
  {
    SwitchDeviceOff();
  }
  else
  {
    SwitchDeviceOn();
  }
}

void SwitchDeviceOff()
{
#define START_SWITCH_OFF 0
#define SWITCH_OFF_VDD 1
#define SWITCH_OFF_VGG 2
static unsigned int uiCntSwitchOff = 2000;
static unsigned char ucSwitchProcessState = START_SWITCH_OFF;
typUnsignedWord stTmp;

  if(uiCntSwitchOff++ > 100)//100=> ca. 150ms zwischen den cases
  {
    uiCntSwitchOff = 0;
    switch(ucSwitchProcessState)
    {
      case START_SWITCH_OFF:        
        stTmp.uiWord = VGG_SWITCH;
        SetDigPoti(CMD_SET_POTI_CHANNEL_VGG ,stTmp);//Vgg -> -6V
        ucSwitchProcessState = SWITCH_OFF_VDD;
        break;
      case SWITCH_OFF_VDD:
        ClrBit(EN_VDD_PORT->ODR,EN_VDD_PIN);   //VDD aus 
        ucSwitchProcessState = SWITCH_OFF_VGG;
        break;
      case SWITCH_OFF_VGG:
 //       ClrBit(EN_VGG_PORT->ODR,EN_VGG_PIN);   //VGG aus -> wird nicht mehr gemacht! VGG soll in den Pausen immer den Transistor ausschalten.
        SetDac(3, DAC_CTRL1, gstDAC[DAC_CTRL1_SPARK]);  //Bereite Steuerspannungen für den ...
        SetDac(3, DAC_CTRL2, gstDAC[DAC_CTRL2_SPARK]);  //...nächsten Zündvorgang vor
        ucSwitchProcessState = START_SWITCH_OFF;
        gucSwitchEvent = SWITCH_EVENT_SWITCH_NOTHING;
        gucDeviceState = STATE_OFF;
        uiCntSwitchOff = 2000;
        break;
      default:
        ucSwitchProcessState = START_SWITCH_OFF;
        gucSwitchEvent = SWITCH_EVENT_SWITCH_NOTHING;
        break;
    }  
  }
}

void SwitchDeviceOn()
{
#define START_SWITCH_ON 0
#define SWITCH_ON_VGG   1
#define SET_VDD_SPARK   2
#define SWITCH_ON_VDD   3
#define SET_VGG_SPARK   4 
#define SET_VGG         5
#define SET_VDD         6

static unsigned int uiCntSwitchOn = 1;
static unsigned char ucSwitchProcessState = START_SWITCH_ON;
typUnsignedWord	stTmp;
  if(uiCntSwitchOn-- < 2)//100=> ca. 150ms zwischen den cases
  {
    switch(ucSwitchProcessState)
    {
      case START_SWITCH_ON:
        gucDeviceState = STATE_BOOTING;
        stTmp.uiWord = VGG_SWITCH;
        SetDigPoti(CMD_SET_POTI_CHANNEL_VGG ,stTmp);//Vgg -> -6V
        ucSwitchProcessState = SWITCH_ON_VGG;
        uiCntSwitchOn = 50;
        break;
      case SWITCH_ON_VGG:
        SetBit(EN_VGG_PORT->ODR,EN_VGG_PIN);   //VGG an 
        ucSwitchProcessState = SET_VDD_SPARK;
        uiCntSwitchOn = 150;
        break;
      case SET_VDD_SPARK:
        SetDigPoti(CMD_SET_POTI_CHANNEL_VDD ,eestVddSpark);//Set VDD für's Zünden
        ucSwitchProcessState = SWITCH_ON_VDD;
        uiCntSwitchOn = 50;
        break;
      case SWITCH_ON_VDD:
        SetBit(EN_VDD_PORT->ODR,EN_VDD_PIN);   //VDD an 
        ucSwitchProcessState = SET_VGG_SPARK;
        uiCntSwitchOn = 50;
        break;
      case SET_VGG_SPARK:
        SetDigPoti(CMD_SET_POTI_CHANNEL_VGG ,eestVggSpark);//Set VGG für's Zünden
        ucSwitchProcessState = SET_VDD;
        uiCntSwitchOn = eestSparkDuration.uiWord ;//2500;
        break;
      case SET_VDD:
        SetDigPoti(CMD_SET_POTI_CHANNEL_VDD ,gstVoltages.Values.gstVdd);//Set VDD
        ucSwitchProcessState = SET_VGG ;
        uiCntSwitchOn = 100;
        break;
      case SET_VGG:
        SetDigPoti(CMD_SET_POTI_CHANNEL_VGG ,gstVoltages.Values.gstVgg);//Set VGG
        SetDac(3, DAC_CTRL1, gstDAC[DAC_CTRL1]);//Set CTRL1
        SetDac(3, DAC_CTRL2, gstDAC[DAC_CTRL2]);//Set CTRL2
        gucSwitchEvent = SWITCH_EVENT_SWITCH_NOTHING;
        gucDeviceState = STATE_ON;
        ucSwitchProcessState = START_SWITCH_ON;
        uiCntSwitchOn = 1;
        break;
        

      default:
        gucSwitchEvent = SWITCH_EVENT_SWITCH_NOTHING;
        ucSwitchProcessState = START_SWITCH_ON;
        break;
    }  
  }
}

void TestLimit(unsigned int uiValue, unsigned int uiLimit, unsigned char ucErrorDeviceState, unsigned char *ucCntLimit)
{
  if (uiValue > uiLimit)//Grenzwert überschritten: Leite Ausschaltvorgang ein
  {
    if(*ucCntLimit >200)//Wenn Grenzwert öfter hintereinander überschritten wurde, dann schalte alles aus
    {
      gucDeviceState = ucErrorDeviceState;
      if (SWITCH_EVENT_SWITCH_NOTHING == gucSwitchEvent) gucSwitchEvent = SWITCH_EVENT_SWITCH_OFF;
    }
    else
    {
      (*ucCntLimit) ++;
    }
  }
  else
  {
    (*ucCntLimit) = 0;
  }
}

void delay(unsigned int us)
{
  while (us--)
  {
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
  };
}

void RGB(bool bRed, bool bGreen, bool bBlue)
{
  if (bRed) SetBit(LED_PORT->ODR,LED_RED_PIN); else ClrBit(LED_PORT->ODR,LED_RED_PIN);
  if (bGreen) SetBit(LED_PORT->ODR,LED_GREEN_PIN); else ClrBit(LED_PORT->ODR,LED_GREEN_PIN);
  if (bBlue) SetBit(LED_PORT->ODR,LED_BLUE_PIN); else ClrBit(LED_PORT->ODR,LED_BLUE_PIN);
}

void SetLedColor()
{
  switch(gucSetupNo)
  {
    case 0:
      RGB(FALSE,TRUE,FALSE);
      break;
    case 1:
      RGB(FALSE,FALSE,TRUE);
      break;
    /*case 2:
      RGB();
      break;
    case 3:
      RGB(TRUE,FALSE,TRUE);
      break;*/
    default:
      RGB(TRUE,FALSE,FALSE);
      break;
  }  
}

void BlinkLed()
{
  static unsigned char ucCntFlash = 9;

  switch(gucDeviceState)
    {
      case STATE_OVERTEMP:
      case STATE_IVDD_ERROR:
      case STATE_IVGG_ERROR:
        if (ucCntFlash++ > 4) 
        {
          RGB(TRUE,FALSE,FALSE);
          ucCntFlash = 0;
        }
        else
        {
          RGB(FALSE,FALSE,FALSE);
        }
        break;
      case STATE_OFF:
        //if (ucCntFlash++ > 10) 
        {
          RGB(TRUE,FALSE,FALSE);
          ucCntFlash = 0;
        }
        /*else
        {
          RGB(FALSE,FALSE,FALSE);
        }*/
        break;
      case STATE_BOOTING:
        RGB(TRUE,TRUE,FALSE);
/*        if (ucCntFlash++ > 5) 
        {
          SetLedColor();
          ucCntFlash = 0;
        }
        else
        {     
          RGB(FALSE,FALSE,FALSE);
        }*/
        break;
      case STATE_READY:
        RGB(TRUE,TRUE,FALSE);
/*        if (ucCntFlash++ > 5) 
        {
          RGB(FALSE,FALSE,FALSE);
          ucCntFlash = 0;
        }
        else
        {               
          SetLedColor();
        }*/
        break;
      case STATE_ON:
        SetLedColor();
        break;
      default:
        gucDeviceState=0;
        break;
    }    
}

unsigned int WriteAndReadAdc(unsigned int uiConfigCmd, GPIO_TypeDef * GPIO_BASE_nCS, unsigned char ucPinNcs)
{
    //disableInterrupts();                                                           //disable general interrupts
//#define READ_CONFIG_REG

  typUnsignedWord uiResult;
  uint8_t ucDataPort[16];
 
    uiResult.uiWord = 0;
    
    ClrBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   aus (SCLK)
    nop();nop();nop();nop();

    ClrBit(GPIO_BASE_nCS->ODR,ucPinNcs);		// Port   aus (no Chip Select)
    for(unsigned char ucBitCnt = 0; ucBitCnt<16; ucBitCnt++)
    {
      SetBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   an 
      nop();nop();nop();nop();
      //SPI_MOSI_PORT->ODR = (uint8_t)ucMosiPort[ucBitCnt];
      if(uiConfigCmd & (0x01<<(15-ucBitCnt)))
      {
          SetBit(SPI_MOSI_PORT->ODR,SPI_MOSI_PIN);        // Port an 
      }
      else
      {
          ClrBit(SPI_MOSI_PORT->ODR,SPI_MOSI_PIN);        // Port aus        
      }
      nop();nop();nop();nop();
      ClrBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   aus (Falling edge of SCLK => Date->Reg)
      ucDataPort[ucBitCnt]=SPI_MISO_PORT->IDR;
    }
    
#ifndef READ_CONFIG_REG
    SetBit(GPIO_BASE_nCS->ODR,ucPinNcs);		// Port   an (no Chip Select)
    //ClrBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   aus (clk low =>To reset the serial interface, hold SCLK low for 28ms)
#endif   
    for(unsigned char ucBitCnt = 0; ucBitCnt<8; ucBitCnt++)
    {
      if(ValBit(ucDataPort[ucBitCnt],SPI_MISO_PIN))
      {
        //SetBit(uiResult,15-ucBitCnt);
        SetBit(uiResult.stBytes.ucHighByte,7-ucBitCnt);
      }
      if(ValBit(ucDataPort[ucBitCnt+8],SPI_MISO_PIN))
      {
        SetBit(uiResult.stBytes.ucLowByte,7-ucBitCnt);
      }
    }
    
#ifdef READ_CONFIG_REG
unsigned int uiConfigReg = 0;
    SetBit(GPIO_BASE_nCS->ODR,ucPinNcs);		// Port   an (no Chip Select)
    //ClrBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   aus (clk low =>To reset the serial interface, hold SCLK low for 28ms)
#endif
    //enableInterrupts();                                                           //Enable general interrupts
    return uiResult.uiWord;
}

void ReadAdc()
{
#define I_VDD           0
#define VDD             1
#define MW_PWR_TO       2
#define MW_PWR_BACK     3
  
#define I_VGG           4
#define VGG             5
#define NTC             6

  
//Commands for ADC1 (IC70)
#define CMD_I_VDD       0xC2CB
#define CMD_VDD         0xD2CB
#define CMD_MW_PWR_TO   0xE2CB
#define CMD_MW_PWR_BACK 0xF2CB

//Commands for ADC2 (IC80)  
#define CMD_I_VGG       0x82CB
#define CMD_VGG         0xE2CB
#define CMD_NTC         0xF2CB
  
  switch(gucCurrentADChannel)
  {
    case I_VDD: 
      gstIvdd.uiWord = WriteAndReadAdc(CMD_VDD, SPI_nCS_ADC1_PORT, SPI_nCS_ADC1_PIN);//lies Ivdd und initialisiere ADC1 für Messung von VDD
      gucCurrentADChannel = VDD;      
    break;
    
    case VDD: 
      gstVddMeasured.uiWord = WriteAndReadAdc(CMD_MW_PWR_TO, SPI_nCS_ADC1_PORT, SPI_nCS_ADC1_PIN);//lies Vdd und initialisiere ADC1 für Messung von MwPwrTo
      gucCurrentADChannel = MW_PWR_TO;      
    break;
    
    case MW_PWR_TO: 
      gstMwPwrTo.uiWord = WriteAndReadAdc(CMD_MW_PWR_BACK, SPI_nCS_ADC1_PORT, SPI_nCS_ADC1_PIN);//lies MwPwrTo und initialisiere ADC1 für Messung von MwPwrBack
      gucCurrentADChannel = MW_PWR_BACK; 
      
if(gstMwPwrTo.stBytes.ucHighByte > 0x25)
{
  gstMwPwrTo.stBytes.ucLowByte++;
} 
      
      
    break;
    
    case MW_PWR_BACK: 
      gstMwPwrBack.uiWord = WriteAndReadAdc(CMD_I_VDD, SPI_nCS_ADC1_PORT, SPI_nCS_ADC1_PIN);//lies MwPwrBack und initialisiere ADC1 für Messung von Ivdd
      gucCurrentADChannel = I_VGG;      
    break;
   
    case I_VGG: 
      gstIvgg.iWord = WriteAndReadAdc(CMD_VGG, SPI_nCS_ADC2_PORT, SPI_nCS_ADC2_PIN);//lies Ivgg und initialisiere ADC2 für Messung von Vgg
      gucCurrentADChannel = VGG;          
    break;
    
    case VGG: 
      gstVggMeasured.iWord = WriteAndReadAdc(CMD_NTC, SPI_nCS_ADC2_PORT, SPI_nCS_ADC2_PIN);//lies Vgg und initialisiere ADC2 für Messung von Ntc
      gucCurrentADChannel = NTC;          
    break;
    
    case NTC: 
      gstTemperature.uiWord = WriteAndReadAdc(CMD_I_VGG, SPI_nCS_ADC2_PORT, SPI_nCS_ADC2_PIN);//lies Ntc und initialisiere ADC2 für Messung von Ivgg
      gucCurrentADChannel = I_VDD;          
    break;
    

    
    default:
      gucCurrentADChannel = I_VDD;
  }
}

void SetDigPoti(unsigned char ucCmd ,typUnsignedWord uiDigPotValue)
{
  //ucCmd -     0x23 => Copy Wiper Register 1 to NV Register 1 and Copy Wiper Register 2 to NV Register 2 Simultaneously
  //            0x01 => Write Wiper Register 1
  //            0x02 => Write Wiper Register 2
  //            0x11 => Write NV Register 1
  //            0x12 => Write NV Register 20
unsigned char aucSendByte[3];
  aucSendByte[0] = ucCmd;
  aucSendByte[1] = uiDigPotValue.stBytes.ucHighByte;
  aucSendByte[2] = uiDigPotValue.stBytes.ucLowByte;
  
  ClrBit(SPI_nCS_POT_PORT->ODR,SPI_nCS_POT_PIN);		// Port   aus (nCS vom MAX5494)
  
  for(unsigned char ucTmp=0; ucTmp<3; ucTmp++)
  {
    for(unsigned char ucBitCnt = 0; ucBitCnt<8; ucBitCnt++)
    {
      ClrBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   aus (SCLK)
      if(aucSendByte[ucTmp] & (0x01<<(7-ucBitCnt)))
      {
          SetBit(SPI_MOSI_PORT->ODR,SPI_MOSI_PIN);        // Port an (DIN vom MAX5494)
      }
      else
      {
          ClrBit(SPI_MOSI_PORT->ODR,SPI_MOSI_PIN);        // Port aus (DIN vom MAX5494)       
      }
      delay(1);
      SetBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   an (Rising edge of SCLK => Date->Reg)
    }
  }
  SetBit(SPI_nCS_POT_PORT->ODR,SPI_nCS_POT_PIN);        // Port   an (nCS vom MAX5494) 
  nop();nop();nop();nop();
  ClrBit(SPI_SCK_PORT->ODR,SPI_SCK_PIN);        // Port   aus (SCLK)

}