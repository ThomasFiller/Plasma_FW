#include "global_vars.h"
#include "stm8l15x_usart.h"
#include "stm8l15x_gpio.h"
#include "functions.h"

#pragma vector = EXTI0_vector//8
__interrupt void ExternInterrrupt0_IRQHandler(void)
{

  EXTI->SR1 = 0x01;//Lösche das extern interrupt-Flag
}

///////Timer 4-Interrupt für zyklische AD-Wandlung und Mittelung/////////////////////////////////////
#pragma vector = TIM4_TIF_UIF_vector//25
__interrupt void TIM4_Update_Overflow_Trigger_COM_IRQHandler(void)
{
 static unsigned char ucCntBlinkLed=0;
  ClrBitMASK(TIM4->CR1, TIM4_CR1_CEN); //Timer4 disabled	
  TIM4->CNTR = 0;
  TIM4->SR1 = 0; //Clear all interrupt flags
  if (ucCntBlinkLed++ > 50) {  guiFlags |= FLAG_BLINK_LED_REQUIRED; ucCntBlinkLed=0;}

  switch(gucSwitchEvent)
  {
    case SWITCH_EVENT_SWITCH_NOTHING:
      guiFlags |= FLAG_READ_ADC_AND_SWITCH_REQUIRED;
      break;
    case SWITCH_EVENT_SWITCH_OFF:
      guiFlags |= FLAG_SWITCH_DEVICE;
      break;
    case SWITCH_EVENT_SWITCH_ON:
      guiFlags |= FLAG_SWITCH_DEVICE;
      break;
    default:
      guiFlags |= FLAG_READ_ADC_AND_SWITCH_REQUIRED;
      break;
  }  

  SetBitMask(TIM4->CR1, TIM4_CR1_CEN); //Timer4 enabled				
}


/////////SPI////////////////////////////////////////////////////////////////////
#pragma vector = SPI_vector
__interrupt void SPI_IRQHandler(void)
{
  //unsigned char uiReceivedByte;
  //uiReceivedByte = ((uint8_t)SPI1->DR);
    SPI1->SR = 0;//Lösche alle SPI-Interrrupt-Flags
}

//////////USART/////////////////////////////////////////////////////////////////USART
#define countof(a)   (sizeof(a) / sizeof(*(a)))

u8 TxBuffer[] = "1234";
u8 TxCounter = 0;

#define TX_BUFFER_SIZE (countof(TxBuffer) - 1)

#pragma vector = USART_R_vector//28
__interrupt void USART1_RX_IRQHandler(void)
{
  u8 temp;

  /* Read one byte from the receive data register and send it back */
  temp = (USART_ReceiveData8(USART1) & 0x7F);
  USART_SendData8(USART1, temp);
}


#pragma vector = USART_T_vector//27
__interrupt void USART1_TX_IRQHandler(void)	//Transmission complete/transmit data register empty
{
   USART_ClearITPendingBit(USART1, USART_IT_TC);
 
/*  // Write one byte to the transmit data register 
//  USART_SendData8(USART1, TxBuffer[TxCounter++]);
  TxCounter++;
  USART_ClearITPendingBit(USART1, USART_IT_TC);
  //USART_ClearITPendingBit(USART1, USART_IT_TXE);
  USART1->SR &= ~0x80; // Lösche TXE-Bit
  if (TxCounter >= TX_BUFFER_SIZE)
  {
    // Disable the USART Transmit Complete interrupt 
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    USART_ITConfig(USART1, USART_IT_TC, DISABLE);
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    //USART1->CR2 &= (uint8_t)(~0x40);
  }*/
}
///////////TIMER 2 für Timeout-Handling/////////////////////////////////////////////////////////////////////
#pragma vector = TIM2_OVR_UIF_vector //TIMEOUT
__interrupt void TIM2_Update_Overflow_Trigger_Break_IRQHandler(void)
{
  TIM2->SR1 = 0; //Clear all interrupt flags
  TIM2->SR2 = 0; //Clear all interrupt flags
  gucTimeout = 1;
  ClrBitMASK(TIM2->CR1, TIM_CR1_CEN);//Timer2 ausschalten
//  UartSendError(1);  
}

///////////TIMER 3 für internen Trigger/////////////////////////////////////////////////////////////////////
#pragma vector = TIM3_OVR_UIF_vector
__interrupt void TIM3_Update_Overflow_Trigger_Break_IRQHandler(void)
{
  ClrBitMASK(TIM3->CR1, TIM_CR1_CEN);//Timer 3 ausschalten
  TIM3->SR1 = 0; //Clear all interrupt flags
  TIM3->SR2 = 0; //Clear all interrupt flags
  SetBitMask(TIM3->CR1, TIM_CR1_CEN);//Timer 3 einschalten
}

#pragma vector = TIM3_CAPCOM_TIF_vector
__interrupt void TIM3_Capture_Compare_IRQHandler(void)
{
  TIM3->SR1 = 0; //Clear all interrupt flags
  TIM3->SR2 = 0; //Clear all interrupt flags
}

////////////ADC/////////////////////////////////////////////////////////////////////////////////////////////////
#pragma vector = COMP_EF1_EF2_vector//18
__interrupt void COMP_ADC_IRQHandler(void)
{
  ClrBitMASK(ADC1->SR, ADC_SR_EOC);//Lösche EOC-Flag
//  ClrBit(GPIOC->ODR,(0));		// Port C0 aus Zur Anzeige ob's funktioniert
}

////////////DMA/////////////////////////////////////////////////////////////////////////////////////////////////
#pragma vector = DMA1_CH0_CH1_vector//2
__interrupt void DMA1_channels_0_1_IRQHandler(void)
{
  ClrBitMASK(DMA1_Channel0->CSPR, DMA_CSPR_TCIF);	//Transaction Complete Interrupt Flag*/
}
