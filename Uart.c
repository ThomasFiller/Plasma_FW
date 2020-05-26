#include "stm8l15x_usart.h"
#include "stm8l15x_gpio.h"
#include "global_vars.h"
#include "functions.h"

unsigned char WaitForReceive();
void WaitForTransmit();
void UartTransmitString(unsigned char *pucSendString);
void ReplyData(unsigned char ucCmd);
void SetParameter(unsigned char ucCmd, unsigned char *pucData, unsigned char ucLenOfFrame);
void WriteCalibrationData();
void ReadCalibrationData();


void Uart_SendErrorMessage(unsigned char ucDataDirection, unsigned char ucErrorCode)
{//Ausgeblendet, um unkontrollierten Datenverkehr auf dem RS485-Bus zu vermeiden
//unsigned char aucData[33];
//  aucData[0]=(gucMyUartAddress);  //1. Byte: 7.Bit=1 (read); other Bytes = Address of LTT-Board  
//  aucData[1]= 3;//2. Byte: Len  
//  aucData[2]=(CMD_ERROR<<4) + ucErrorCode;//3. Byte: Command:Error and Subcommand:Errorcode5-timeout because not enough databytes
//  UartTransmitString(aucData);
}

void UartReceiveOrTransmit(void)
{
unsigned char temp;
unsigned char ucCommand;
unsigned char ucLenOfFrame;
unsigned char ucCntToLen;
unsigned char aucData[33];
unsigned char ucChecksum;
      ClrBitMASK(TIM4->CR1, TIM4_CR1_CEN); //Timer4 disabled	
      temp = (USART_ReceiveData8(USART1) );//0.Byte: Adresse und Datentransportrichtung
      ucChecksum = temp;
      DataDirection = temp >> 7;
      temp &= 0x7F; // delete Read/Write-Bit
      if (temp == gucMyUartAddress)//is it my Address?
      {
        if (!WaitForReceive()) //Warten auf 1.Byte: Length of frame
        {
           ucLenOfFrame = (USART_ReceiveData8(USART1));
           ucChecksum ^= ucLenOfFrame;

           if (!WaitForReceive()) //Warten auf 2.Byte: Command
           {              
              ucCommand = (USART_ReceiveData8(USART1) );
              ucChecksum ^= ucCommand;
              
              for (ucCntToLen=0; ucCntToLen < (ucLenOfFrame-3); ucCntToLen++)
              {
                if (!WaitForReceive()) 
                {
                  aucData[ucCntToLen] = (USART_ReceiveData8(USART1) );
                  ucChecksum ^= aucData[ucCntToLen];
                }
                else
                {//UART-Timeout: DataByte not received
                  ucCntToLen = ucLenOfFrame;//Abbruch
                  Uart_SendErrorMessage(DataDirection, ERROR_NOT_ENOUGH_DATABYTES);
                }
              }  
              if(ucChecksum)
              {//Checksum-Error
//                while(!WaitForReceive())//UART leerlesen
                {
//                              temp = (USART_ReceiveData8(USART1) );
                }
                Uart_SendErrorMessage(DataDirection, ERROR_CHECKSUM);                
              }
              else
              {
                if(DataDirection) 
                  ReplyData(ucCommand); 
                else  
                  SetParameter(ucCommand, aucData, ucLenOfFrame);
              }
            }
            else
            {//UART-Timeout: 2.Byte not received
              Uart_SendErrorMessage(DataDirection, ERROR_TIMEOUT_BEFORE_2nd_BYTE);
            }
          }
          else
          {//UART-Timeout: 1.Byte not received
            Uart_SendErrorMessage(DataDirection, ERROR_TIMEOUT_BEFORE_1st_BYTE);
          }
      }
      SetBitMask(TIM4->CR1, TIM4_CR1_CEN); //Timer4 enabled	
}


void WaitForTransmit()
{
    while(!((USART1->SR) & 0x40));
}

unsigned char WaitForReceive()
{
    gucTimeout = 0;
    TIM2->CNTRL = 0;//Timer2 zurücksetzen
    TIM2->CNTRH = 0;//Timer2 zurücksetzen
    TIM2->SR1 = 0; //Clear all interrupt flags
    TIM2->SR2 = 0; //Clear all interrupt flags

    SetBitMask(TIM2->CR1, TIM_CR1_CEN);//Timer2 einschalten für timeouthandling

    while((!((USART1->SR) & 0x20)) && !gucTimeout);//Warte auf UART-Empfang; bei timeout setzt Timer2-Interrupt timout auf 1

    ClrBitMASK(TIM2->CR1, TIM_CR1_CEN);//Timer2 ausschalten

    return gucTimeout;
}

void UartTransmitString(unsigned char *pucSendString)
{//pucSendString[1] has to contain length of complete frame
unsigned char ucCnt;
unsigned char ucChecksum;
  ucChecksum = 0;
  for (ucCnt=0; ucCnt < pucSendString[1]-1;ucCnt++)
  {
    USART1->DR = pucSendString[ucCnt];
    ucChecksum = ucChecksum ^ pucSendString[ucCnt];
    WaitForTransmit();
  }
  USART1->DR = ucChecksum;
  WaitForTransmit();
}

void ReplyData(unsigned char ucCmd)//DataDirection == 1
{
unsigned char pucStringToSend[40]; 
  pucStringToSend[0]=gucMyUartAddress + 0x80;//1. Byte: Address of Board
  pucStringToSend[2]=ucCmd;
  switch(ucCmd)//Command
  {
    case UART_SETUP_NO://liest eine der Setupnummern, die sich parrallel auch mit dem Schalter einstellen und über die LEDs ablesen lassen
      pucStringToSend[1] = 5;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gucSetupNo;//3. Byte -> 1.Datenbyte
      break;
    case UART_DEVICE_STATE://OFF=0; STATE_BOOTING=1; READY=2; ON=3; OVERTEMP=4
      pucStringToSend[1] = 5;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gucDeviceState;//3. Byte -> 1.Datenbyte
      break;
    case UART_5V_GOOD://Liest, ob 5-V-Versorgungsspannung ok ist (0-nicht ok; 1-ok)
      pucStringToSend[1] = 5;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= (unsigned char) (ValBit(V5V_GOOD_PORT, V5V_GOOD_PIN));//3. Byte -> 1.Datenbyte
      break;
    case UART_STORAGE://liefert alle im internen EEPROM gespeicheicherten Reservebytes zurück
      pucStringToSend[1] = 20;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      for (unsigned char ucCnt=0; ucCnt<15; ucCnt++)     pucStringToSend[ucCnt+3]= eeucStorageBytes[ucCnt];//3.-18. Byte -> Datenbytes
      break;
    case UART_TEMPERATURE:
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
        pucStringToSend[3]= gstTemperature.stBytes.ucHighByte;
        pucStringToSend[4]= gstTemperature.stBytes.ucLowByte;      
      break;
    case UART_READ_ALL:
      pucStringToSend[1] = 24;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      
      pucStringToSend[ 3]= gucDeviceState;
      //pucStringToSend[ 4]= (ValBit(EN_VDD_PORT->ODR,EN_VDD_PIN)) | ((ValBit(EN_VGG_PORT->ODR,EN_VGG_PIN))<<1) | ((ValBit(VDD_GOOD_PORT->IDR, VDD_GOOD_PIN))<<2) | (gucSetupNo<<4);
      //pucStringToSend[ 4]= 0;
      pucStringToSend[ 4]= (gucSetupNo<<4);
      if(ValBit(EN_VDD_PORT->ODR,EN_VDD_PIN)) pucStringToSend[ 4]|= 0x01;  
      if(ValBit(EN_VGG_PORT->ODR,EN_VGG_PIN)) pucStringToSend[ 4]|= 0x02;  
      if(ValBit(VDD_GOOD_PORT->IDR, VDD_GOOD_PIN)) pucStringToSend[ 4]|= 0x04; 
      pucStringToSend[ 5]= gstVoltages.Values.gstVdd.stBytes.ucHighByte;
      pucStringToSend[ 6]= gstVoltages.Values.gstVdd.stBytes.ucLowByte;
      pucStringToSend[ 7]= gstVoltages.Values.gstVgg.stBytes.ucHighByte;
      pucStringToSend[ 8]= gstVoltages.Values.gstVgg.stBytes.ucLowByte;
      pucStringToSend[ 9]= gstIvdd.stBytes.ucHighByte;
      pucStringToSend[10]= gstIvdd.stBytes.ucLowByte;
      pucStringToSend[11]= gstIvgg.stBytes.ucHighByte;
      pucStringToSend[12]= gstIvgg.stBytes.ucLowByte;
      pucStringToSend[13]= gstTemperature.stBytes.ucHighByte;//Temperatur
      pucStringToSend[14]= gstTemperature.stBytes.ucLowByte;//Temperatur
      pucStringToSend[15]= gstMwPwrTo.stBytes.ucHighByte;
      pucStringToSend[16]= gstMwPwrTo.stBytes.ucLowByte;
      pucStringToSend[17]= gstMwPwrBack.stBytes.ucHighByte;
      pucStringToSend[18]= gstMwPwrBack.stBytes.ucLowByte;      
      pucStringToSend[19]= gstVddMeasured.stBytes.ucHighByte;
      pucStringToSend[20]= gstVddMeasured.stBytes.ucLowByte;      
      pucStringToSend[21]= gstVggMeasured.stBytes.ucHighByte;
      pucStringToSend[22]= gstVggMeasured.stBytes.ucLowByte;      
      break;
    case UART_SPARK_DURATION://Dauer, die Vgg,spark gehalten wird
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)      
      pucStringToSend[3]= eestSparkDuration.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= eestSparkDuration.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;

  //VDD			//			
    case UART_EN_VDD://Spannungsreglers IC20 (VDD) eigeschaltet? (0-aus; 1-an)
      pucStringToSend[1] = 5;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= (unsigned char) (ValBit(EN_VDD_PORT->ODR,EN_VDD_PIN));//3. Byte -> 1.Datenbyte
      break;
    case UART_VDD_GOOD://Liest, ob VDD ok ist (0-nicht ok; 1-ok)
      pucStringToSend[1] = 5;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= (unsigned char) (ValBit(VDD_GOOD_PORT->IDR, VDD_GOOD_PIN));//3. Byte -> 1.Datenbyte
      break;
    case UART_VDD://Lesen der eingestellten Spannung VDD
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstVoltages.Values.gstVdd.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstVoltages.Values.gstVdd.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
    case UART_I_VDD://Lesen des Stroms durch VDD
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstIvdd.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstIvdd.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;      
    case UART_READ_VDD://Lesen der gemessenen VDD
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstVddMeasured.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstVddMeasured.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;      
    case UART_VDD_SPARK://Lesen der Spannung, bei der das Plasma zündet. Diese Spannung wird bei der automatischen Zündung kurz nach Einschalten von Vgg kurz an Vdd angelegt
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)      
      pucStringToSend[3]= eestVddSpark.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= eestVddSpark.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
     
  //VGG
    case UART_EN_VGG://Spannungsreglers IC20 (VGG) eigeschaltet? (0-aus; 1-an)
      pucStringToSend[1] = 5;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= (unsigned char) (ValBit(EN_VGG_PORT->ODR,EN_VGG_PIN));//3. Byte -> 1.Datenbyte
      break;
    case UART_VGG://Lesen der eingestellten Spannung VGG
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstVoltages.Values.gstVgg.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstVoltages.Values.gstVgg.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
    case UART_I_VGG://Lesen des Stroms durch VGG
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstIvgg.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstIvgg.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
    case UART_READ_VGG://lesen der gemessenen Spannung VGG 
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstVggMeasured.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstVggMeasured.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
    case UART_VGG_SPARK://Lesen der Spannung, bei der das Plasma zündet. Diese Spannung wird bei der automatischen Zündung kurz nach Einschalten von Vgg kurz an Vdd angelegt
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)      
      pucStringToSend[3]= eestVggSpark.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= eestVggSpark.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
     
  //MW // Mikrowellenparameter
    case UART_MW_PWR_TO://lesen der Leistung, die in die Mikrowelle geht
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstMwPwrTo.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstMwPwrTo.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
    case UART_MW_PWR_BACK://lesen der Leistung, die von der Mikrowelle kommt
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstMwPwrBack.stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstMwPwrBack.stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
      
  //Steuerspannung für Kapazitäten zur Steuerung des Arbeitspunktes						
    case UART_CTRL1://Lesen der eingestellten Spannung CTRL1
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstDAC[DAC_CTRL1].stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstDAC[DAC_CTRL1].stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
    case UART_CTRL2://Lesen der eingestellten Spannung CTRL2
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)
      pucStringToSend[3]= gstDAC[DAC_CTRL2].stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstDAC[DAC_CTRL2].stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
    case UART_VCTRL1_SPARK://Lesen der Steuerspannung CTRL1, welche während des Zündvorgangs angelegt wird
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)      
      pucStringToSend[3]= gstDAC[DAC_CTRL1_SPARK].stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstDAC[DAC_CTRL1_SPARK].stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
    case UART_VCTRL2_SPARK://Lesen der Steuerspannung CTRL1, welche während des Zündvorgangs angelegt wird
      pucStringToSend[1] = 6;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)      
      pucStringToSend[3]= gstDAC[DAC_CTRL2_SPARK].stBytes.ucHighByte;//3. Byte -> 1.Datenbyte
      pucStringToSend[4]= gstDAC[DAC_CTRL2_SPARK].stBytes.ucLowByte;//4. Byte -> 2.Datenbyte
      break;
      
    default:
      pucStringToSend[1]= 4;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes) 
      pucStringToSend[2]=UART_ERROR;
      pucStringToSend[3]= ERROR_WRONG_CMD;
      break;
  }
  delay(3000/*1500*/);
  UartTransmitString(pucStringToSend);
}

void SetParameter(unsigned char ucCmd, unsigned char *pucData, unsigned char ucLenOfFrame) //DataDirection==0
{
unsigned char pucStringToSend[10]; 
  pucStringToSend[0]=(gucMyUartAddress);  //1. Byte: 7.Bit=1 (read); other Bytes = Address of LTT-Board  
  pucStringToSend[1]=4;//2. Byte: Len  
  pucStringToSend[2]=ucCmd; 

  //Flash aufschließen  
  FLASH->DUKR=0xAE;
  FLASH->DUKR=0x56;

  switch(ucCmd)
  {
  //Allgemein; Setup   
      
    case UART_SETUP_NO://liest eine der Setupnummern, die sich parrallel auch mit dem Schalter einstellen und über die LEDs ablesen lassen
      if(pucData[0]<NO_OF_SETUPS) gucSetupNo = pucData[0]; else gucSetupNo = 0;
      FLASH->DUKR=0xAE;
      FLASH->DUKR=0x56;
      eeucSetupNo = gucSetupNo;
      guiFlags |= FLAG_CHANGE_SETUP_REQUIRED;
      break;
    case UART_SAVE_SETUP://liest eine der vier Setupnummern, die sich parrallel auch mit dem Schalter einstellen und über die LEDs ablesen lassen
      guiFlags |= FLAG_SAVE_SETUP_REQUIRED;
      break;
    case UART_STORAGE://16 Reservedaten zum Abspeichern (z.Bsp. Tmax, Tmin…); 1. Byte: Adresse (0..15); zweites Byte: Datum
      FLASH->DUKR=0xAE;
      FLASH->DUKR=0x56;
      eeucStorageBytes[pucData[0]] = pucData[1];
      break;
    case UART_EN_PLASMA://Einschalten oder Ausschalten des Plasmas in geordneter Sequenz (0-aus; 1-an)
      if(pucData[0]==1)
      {
        gucSwitchEvent = SWITCH_EVENT_SWITCH_ON;
        bDeviceSwitchedOnByUart = 1;
      }
      else
      {
        gucSwitchEvent = SWITCH_EVENT_SWITCH_OFF;
        bDeviceSwitchedOnByUart = 0;
      }
      break;           
    case UART_SPARK_DURATION://Dauer, die Vgg,spark gehalten wird
      eestSparkDuration.stBytes.ucHighByte=pucData[0];
      eestSparkDuration.stBytes.ucLowByte=pucData[1];
      break;

      
  //VDD (20..40V; 0..2,5A)			
    case UART_EN_VDD://Ein- bzw. Ausschalten des Spannungsreglers IC20 (VDD) (0-aus; 1-an)
      if(pucData[0]==1)
      {
        if((ValBit(EN_VGG_PORT->ODR,EN_VGG_PIN))&&(gstVoltages.Values.gstVgg.stBytes.ucHighByte < 0x07))               //Einschalten von VDD nur zulassen, wenn VGG < -6V (Transistor OFF)
        {
          SetBit(EN_VDD_PORT->ODR,EN_VDD_PIN);
          SetDac(3, DAC_CTRL1, gstDAC[DAC_CTRL1]);//Set CTRL1
          SetDac(3, DAC_CTRL2, gstDAC[DAC_CTRL2]);//Set CTRL2
          gucDeviceState = STATE_ON;
          bDeviceSwitchedOnByUart = 1;
        }
        else
        {
          pucStringToSend[1]= 5;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)  
          pucStringToSend[2]= UART_ERROR;
          pucStringToSend[3]= ERROR_VGG_NOT_RIGHT;
        }
      }
      else
      {
        ClrBit(EN_VDD_PORT->ODR,EN_VDD_PIN);
        SetDac(3, DAC_CTRL1, gstDAC[DAC_CTRL1_SPARK]);  //Bereite Steuerspannungen für den ...
        SetDac(3, DAC_CTRL2, gstDAC[DAC_CTRL2_SPARK]);  //...nächsten Zündvorgang vor
        gucDeviceState = STATE_OFF;
        bDeviceSwitchedOnByUart = 0;
      }
      break;
    case UART_DIGPOTI_SAVE://Aktuelle Einstellung des Potis für VDD und VGG in das NonVolatile-Register schreiben; beim nächsten PotiStart wird dieser Wert übernommen
      guiFlags |= FLAG_SAVE_DIGPOTI_REQUIRED;
      break;
    case UART_VDD://Spannung VDD einstellen
      gstVoltages.Values.gstVdd.stBytes.ucHighByte = pucData[0];
      gstVoltages.Values.gstVdd.stBytes.ucLowByte = pucData[1];
      guiFlags |= FLAG_SET_VDD_REQUIRED;
      break;
      
    case UART_VDD_SPARK://Lesen der Spannung, bei der das Plasma zündet. Diese Spannung wird bei der automatischen Zündung kurz nach Einschalten von Vgg kurz an Vdd angelegt
      eestVddSpark.stBytes.ucHighByte=pucData[0];
      eestVddSpark.stBytes.ucLowByte=pucData[1];
      break;

  //VGG (-6..-1V; -1..10mA)
    case UART_EN_VGG://Ein- bzw. Ausschalten des SwitchedCap-Spannungsreglers IC40 (VGG) (0-aus; 1-an)
//      if(pucData[0]==1) SetBit(EN_VGG_PORT->ODR,EN_VGG_PIN); else ClrBit(EN_VGG_PORT->ODR,EN_VGG_PIN);
      if(pucData[0]==1)
      {
        SetBit(EN_VGG_PORT->ODR,EN_VGG_PIN);
      }
      else
      {
        gucSwitchEvent = SWITCH_EVENT_SWITCH_OFF;
      }
      break;
    case UART_VGG://Spannung VGG einstellen
      gstVoltages.Values.gstVgg.stBytes.ucHighByte = pucData[0];
      gstVoltages.Values.gstVgg.stBytes.ucLowByte = pucData[1];
      guiFlags |= FLAG_SET_VGG_REQUIRED;
      break;    
    case UART_VGG_SPARK://Schreiben der Spannung, bei der das Plasma zündet. Diese Spannung wird bei der automatischen Zündung kurz nach Einschalten von Vgg kurz an Vdd angelegt
      eestVggSpark.stBytes.ucHighByte=pucData[0];
      eestVggSpark.stBytes.ucLowByte=pucData[1];
      break;
      
  //Steuerspannung für Kapazitäten zur Steuerung des Arbeitspunktes
    case UART_CTRL1://Spannung CTRL1 einstellen
      gstDAC[DAC_CTRL1].stBytes.ucHighByte = pucData[0];
      gstDAC[DAC_CTRL1].stBytes.ucLowByte = pucData[1];
      guiFlags |= FLAG_SET_CTRL1_REQUIRED;
      break;
    case UART_CTRL2://Spannung CTRL2 einstellen
      gstDAC[DAC_CTRL2].stBytes.ucHighByte = pucData[0];
      gstDAC[DAC_CTRL2].stBytes.ucLowByte = pucData[1];
      guiFlags |= FLAG_SET_CTRL2_REQUIRED;
      break;
    case UART_VCTRL1_SPARK://Schreiben der Steuerspannung CTRL1, welche während des Zündvorgangs angelegt wird
      gstDAC[DAC_CTRL1_SPARK].stBytes.ucHighByte=pucData[0];
      gstDAC[DAC_CTRL1_SPARK].stBytes.ucLowByte=pucData[1];
      break;
    case UART_VCTRL2_SPARK://Schreiben der Steuerspannung CTRL2, welche während des Zündvorgangs angelegt wird
      gstDAC[DAC_CTRL2_SPARK].stBytes.ucHighByte=pucData[0];
      gstDAC[DAC_CTRL2_SPARK].stBytes.ucLowByte=pucData[1];
      break;
   
    default:
      pucStringToSend[1]= 5;//1. Byte: Length of frame (5 bei einem Datenbyte; 6 bei 2 Datenbytes)  
      pucStringToSend[2]= UART_ERROR;
      pucStringToSend[3]= ERROR_WRONG_CMD;
      break;
  }
  UartTransmitString(pucStringToSend);
}