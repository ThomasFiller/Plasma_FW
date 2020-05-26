#include "global_vars.h"
#include "functions.h"
#include "stm8l15x_flash.h"

void UsartSetup(void);
void TIM2_Setup(void);
//void TIM3_Setup(void);
void TIM4_Setup(void);
void GPIO_Setup(void);
void FirstCall();

void SystemSetup()
{
  CLK->CKDIVR = 0x00;   // adjust prescale for using 16MHz as systemclock
  GPIO_Setup();
  TIM2_Setup();       // setup timer 2 für Timout-handling
//  TIM3_Setup();       // timer 3 für internen Pulsgenerator
  
  TIM4_Setup();       // setup timer 4 für zyklische AD-Wandlung und Mittelung
  UsartSetup();
  if(!(eeucCheckFirstCall==0x8F))
  {//erste Initialisierung EEPROM nach dem Programmieren
    FirstCall();
  }
}

void TIM2_Setup()				                                // timer 2 für Timout-handling
{
  SetBitMask(CLK->PCKENR1,CLK_PCKENR1_TIM2);	
  TIM2->IER = 0x01;//Interrupt enable register
  TIM2->PSCR = 0x06;//Prescaler=64 => 262ms für Timeout
  //TIM2->PSCR = 0x07;//Prescaler=128 => 524ms für Timeout
}

void TIM4_Setup()				                                // timer 4 für zyklische AD-Wandlung
{
  SetBitMask(CLK->PCKENR1,CLK_PCKENR1_TIM4);	
  //TIM4->ARR = 195;//3; //Reload-Register
  TIM4->ARR = 100;//3; //Reload-Register
  TIM4->IER = 0x01; //Update Interrupt enable (Overflow-Interrupt)
  //TIM4->PSCR = 0x0C;//Prescaler (0..0x0F); 0x0C => 50ms
  //TIM4->PSCR = 0x09;//Prescaler (0..0x0F); 0x0C => 50ms
  TIM4->PSCR = 0x08;//Prescaler (0..0x0F); 0x0C => 50ms
  TIM4->SMCR = 0x00; //Clock kommt vom System
}

void FirstCall()//wird nur beim ersten mal nach dem Programmieren aufgerufen, um EEPROM zu definieren
{
  FLASH->DUKR=0xAE;
  FLASH->DUKR=0x56;

  eeucSetupNo = 0;
  for(gucSetupNo = 0; gucSetupNo<NO_OF_SETUPS; gucSetupNo++)
  {
    eeuiVoltages[gucSetupNo] = 0;
    for(u8 u8CntDac = 0; u8CntDac<4; u8CntDac++)
    {
      eeucDacHiByte[u8CntDac][gucSetupNo] = 0;
      eeucDacLoByte[u8CntDac][gucSetupNo] = 0;
    }
  }
  
  eeucStorageBytes[STORAGE_1] = 0;
  eeucStorageBytes[STORAGE_2] = 40;
  eeucStorageBytes[STORAGE_3] = 0;
  gucSetupNo = 8;
  eeucMyUartAddress= DEFAULT_UART_ADDRESS;
  eeucCheckFirstCall = 0x8F;    
}

void ReadSetupFromEeprom()//Parameter aus EEPROM einlesen
{
  gstVoltages.ulLong = eeuiVoltages[gucSetupNo]; 
  for(u8 u8CntDac = 0; u8CntDac<4; u8CntDac++)
  {
    gstDAC[u8CntDac].stBytes.ucHighByte = eeucDacHiByte[u8CntDac][gucSetupNo]; 
    gstDAC[u8CntDac].stBytes.ucLowByte  = eeucDacLoByte[u8CntDac][gucSetupNo]; 
  } 
  gucMyUartAddress              = eeucMyUartAddress;
  guiFlags = FLAG_SET_VDD_REQUIRED; // | FLAG_SET_VGG_REQUIRED;->VGG soll am Anfang nicht auf den Sollwert, sondern auf den Zündwert VGG_SWITCH eingestellt werden
}

void SaveSetupToEeprom(void)//EEPROM - Parameterablage
{
  FLASH->DUKR=0xAE;
  FLASH->DUKR=0x56;
  eeuiVoltages[gucSetupNo]= gstVoltages.ulLong;
  
  for(u8 u8CntDac = 0; u8CntDac<4; u8CntDac++)
  {
    eeucDacHiByte[u8CntDac][gucSetupNo] = gstDAC[u8CntDac].stBytes.ucHighByte  ; 
    eeucDacLoByte[u8CntDac][gucSetupNo]  = gstDAC[u8CntDac].stBytes.ucLowByte   ; 
  }
}

void SetInputFloating(GPIO_TypeDef * GPIO_BASE, unsigned char ucPin)
{
  ClrBit(GPIO_BASE->DDR,ucPin);	//Input
  ClrBit(GPIO_BASE->CR1,ucPin); //no pull-up
  ClrBit(GPIO_BASE->CR2,ucPin); //External interrupt disabled
}

void SetInputWithPullUp(GPIO_TypeDef * GPIO_BASE, unsigned char ucPin)
{
  ClrBit(GPIO_BASE->DDR,ucPin);	//Input
  SetBit(GPIO_BASE->CR1,ucPin); //pull-up
  ClrBit(GPIO_BASE->CR2,ucPin); //External interrupt disabled
}

void SetOutputOpenDrain(GPIO_TypeDef * GPIO_BASE, unsigned char ucPin)
{
  SetBit(GPIO_BASE->DDR,ucPin);	//Output
  ClrBit(GPIO_BASE->CR1,ucPin); //Open Drain
  SetBit(GPIO_BASE->CR2,ucPin); //HighSpeed up to 10 MHz
}

void SetOutputPushPullOff(GPIO_TypeDef * GPIO_BASE, unsigned char ucPin)
{
  SetBit(GPIO_BASE->DDR,ucPin);	//Output
  SetBit(GPIO_BASE->CR1,ucPin); //Push Pull
  SetBit(GPIO_BASE->CR2,ucPin); //HighSpeed up to 10 MHz
  ClrBit(GPIO_BASE->ODR,ucPin); //Set Off
}

void SetOutputPushPullOn(GPIO_TypeDef * GPIO_BASE, unsigned char ucPin)
{
  SetBit(GPIO_BASE->DDR,ucPin);	//Output
  SetBit(GPIO_BASE->CR1,ucPin); //Push Pull
  SetBit(GPIO_BASE->CR2,ucPin); //HighSpeed up to 10 MHz
  SetBit(GPIO_BASE->ODR,ucPin); //Set On
}

void SetInputNoPullupInterrupt(GPIO_TypeDef* gpioPort, unsigned int ucPin)
{
   ClrBit(gpioPort->DDR,(ucPin));	//Input
   ClrBit(gpioPort->CR1,(ucPin));	//kein pull-up	
   SetBit(gpioPort->CR2,(ucPin));	//External interrupt enabled
}

void GPIO_Setup()
{
  SetOutputPushPullOn(SPI_nCS_POT_PORT, SPI_nCS_POT_PIN); 
  SetOutputPushPullOn(SPI_nCS_ADC1_PORT, SPI_nCS_ADC1_PIN); 
  SetOutputPushPullOn(SPI_nCS_ADC2_PORT, SPI_nCS_ADC2_PIN); 
  SetOutputPushPullOn(SPI_nCS_DAC_PORT, SPI_nCS_DAC_PIN); 
  SetOutputPushPullOn(SPI_SCK_PORT, SPI_SCK_PIN);
  SetOutputPushPullOff(SPI_MOSI_PORT, SPI_MOSI_PIN);
  SetInputFloating(SPI_MISO_PORT, SPI_MISO_PIN);

//******************************Spannungsversorgung VDD
  SetOutputPushPullOff(EN_VDD_PORT,EN_VDD_PIN);
  SetInputWithPullUp(VDD_GOOD_PORT, VDD_GOOD_PIN);

//******************************Spannungsversorgung VGG
  SetOutputPushPullOff(EN_VGG_PORT, EN_VGG_PIN);

//******************************Spannungsversorgung 5V für uC (über IC60->3V3) und BIAS IC20
  SetInputWithPullUp(V5V_GOOD_PORT, V5V_GOOD_PIN);

//******************************Display-LED
  SetOutputPushPullOff(LED_PORT, LED_RED_PIN);
  SetOutputPushPullOff(LED_PORT, LED_GREEN_PIN);
  SetOutputPushPullOff(LED_PORT, LED_BLUE_PIN);
}

void UsartSetup(void)
{ 
  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);                     //Enable Usart clock

  SYSCFG->RMPCR1 &= ~0x70;
  SYSCFG->RMPCR1 |= 0x40;	                                                //USART1_CK mapped on PA0
  SYSCFG->RMPCR1 |= 0x00;	                                                //USART1_TX on PC3 and USART1_RX on PC2
  GPIOC->DDR |= 0x08;                                                           //USART1_TX on PC3 => OUTPUT
 
  // SYSCFG->RMPCR1 |= 0x20;	                                                //USART1_TX on PC5 and USART1_RX on PC6
 // GPIOC->DDR |= 0x20;                                                           //USART1_TX on PC5 => OUTPUT
   
  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3 | GPIO_Pin_2, ENABLE);            //Set the USART RX and USART TX at high level
  USART_DeInit(USART1);
  USART1->CR1 &= (uint8_t)(~(USART_CR1_PCEN | USART_CR1_PS | USART_CR1_M));
  //USART1->CR1 |= (uint8_t)(USART_WordLength_8b | USART_Parity_No);
  //USART1->CR1 |= (uint8_t)(USART_WordLength_8b | USART_Parity_Even);
    
  USART1->CR3 &= (uint8_t)(~USART_CR3_STOP);                                    // Clear the STOP bits
  USART1->CR3 |= (uint8_t)(USART_StopBits_1);                                   // Clear the STOP bits

  /* Set Baudrate */
  //USART1->BRR2 = 0x0B;//115kBaud
  //USART1->BRR1 = 0x08;//115kBaud
  //USART1->BRR2 = 0x03;//9,6kBaud
  //USART1->BRR1 = 0x68;//9,6kBaud
  //USART1->BRR2 = 0x01;//19,2kBaud
  //USART1->BRR1 = 0x34;//19,2kBaud
  //USART1->BRR2 = 0x06;//57,6kBaud
  //USART1->BRR1 = 0x11;//57,6kBaud
  //USART1->BRR2 = 0x1B;//2,4kBaud
  //USART1->BRR1 = 0xA0;//2,4kBaud
  USART1->BRR2 = 0x01;                                                          //38400Baud
  USART1->BRR1 = 0x1A;                                                          //38400Baud
    
  USART1->CR2 &= (uint8_t)~(USART_CR2_TEN | USART_CR2_REN);                     // Disable the Transmitter and Receiver
  USART1->CR2 |= (uint8_t)(USART_Mode_Tx | USART_Mode_Rx);                      // Set TEN and REN bits according to USART_Mode value 
  USART_Cmd(USART1, ENABLE);                                                    // Enable USART 
  //enableInterrupts();
  USART1->SR &= ~0x80; // Lösche TXE-Bit 
}


