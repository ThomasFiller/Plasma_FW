//#ifndef __GlobalConsts_H_included
//#define __GlobalConsts_H_included

#include "stm8l15x.h"
#include "makros.h"
#include "stm8l15x_adc.h" 
#include "stm8l15x_usart.h"
#include "stm8l15x_dma.h"
#include "stm8l15x_gpio.h"

#define DEFAULT_UART_ADDRESS 0x2A

#define OVERTEMPERATURE         0x347B //entspricht etwa 80°C (NTC auf MW-Board)
#define OVERCURRENT_GATE        16000 //entspricht etwa 10mA
#define OVERCURRENT_DRAIN       16000 //16000 entspricht etwa 2,5A
#define VGG_SWITCH              0x0000 //Spannung, bei der VDD ein- oder ausgeschaltet wird; 

#define NO_OF_SETUPS 2

#define DAC_CTRL1 0
#define DAC_CTRL2 1
#define DAC_CTRL1_SPARK 2
#define DAC_CTRL2_SPARK 3

//******************************SwicthEvent states
#define SWITCH_EVENT_SWITCH_NOTHING     0
#define SWITCH_EVENT_SWITCH_OFF         1
#define SWITCH_EVENT_SWITCH_ON          2

//******************************Device states
#define STATE_OFF 0
#define STATE_BOOTING 1
#define STATE_READY 2
#define STATE_ON 3
#define STATE_OVERTEMP 4
#define STATE_IVDD_ERROR 5
#define STATE_IVGG_ERROR 6

//*******************************Dig. Poti-Commands
#define CMD_SET_POTI_CHANNEL_VGG        0x01
#define CMD_SET_POTI_CHANNEL_VDD        0x02
#define CMD_SAVE_CURRENT_VALUES         0x23

//*******************************Flag STM8 guiFlags
#define FLAG_CHANGE_SETUP_REQUIRED      0x0001
#define FLAG_SAVE_SETUP_REQUIRED        0x0002
#define FLAG_BLINK_LED_REQUIRED         0x0004
#define FLAG_READ_ADC_AND_SWITCH_REQUIRED       0x0008

#define FLAG_SAVE_DIGPOTI_REQUIRED      0x0010
#define FLAG_SET_VDD_REQUIRED           0x0020
#define FLAG_SET_VGG_REQUIRED           0x0040
#define FLAG_SWITCH_DEVICE              0x0080

#define FLAG_SET_CTRL1_REQUIRED         0x0100
#define FLAG_SET_CTRL2_REQUIRED         0x0200

//*******************************UART-Commands
//Allgemein; Setup
#define	UART_ERROR	        0x00	//	0	1	Errorcode (see below)
#define	UART_SETUP_NO	        0x03	//	1	1	liest oder schreibt eine der vier Setupnummern, die sich parrallel auch mit dem Schalter einstellen und über die LEDs ablesen lassen
#define	UART_SAVE_SETUP         0x04	//	0	/	veranlasst den uC, das aktuelle Setup und die aktuelle Setupnummer zu speichern
#define	UART_DEVICE_STATE	0x05	//	/	1	OFF=0; BOOTING=1; READY=2; ON=3; OVERTEMP=4
#define	UART_5V_GOOD	        0x06	//	/	1	Liest, ob 5-V-Versorgungsspannung ok ist (0-nicht ok; 1-ok)
#define	UART_TEMPERATURE	0x07	//	/	2	Liest Temperatur, die mit dem STM8-internen Sensor ermittelt wurde
#define	UART_STORAGE	        0x09	//	2	2	16 Reservedaten zum Abspeichern (z.Bsp. Tmax, Tmin…); 1. Byte: Adresse (0..15); zweites Byte: Datum
    #define STORAGE_1           0
    #define STORAGE_2           1
    #define STORAGE_3           2
#define	UART_EN_PLASMA	        0x0A	//	1	1	Einschalten oder Ausschalten des Plasmas in geordneter Sequenz
#define	UART_READ_ALL	        0x0B	//	/	16	Liest alle Daten, die für zyklische Anzeige benötigt werden
#define	UART_SPARK_DURATION	0x0C	//	2	2	Dauer, die Vgg,spark gehalten wird

//VDD (20..40V; 0..2,5A)			//			
#define	UART_EN_VDD	        0x10	//	1	1	Einschalten des Spannungsreglers IC20 (VDD)
#define	UART_VDD_GOOD	        0x11	//	/	1	Liest, ob VDD ok ist (0-nicht ok; 1-ok)
#define	UART_DIGPOTI_SAVE	0x12	//	0	/	Aktuelle Einstellung des Potis für VDD und VGG in das NonVolatile-Register schreiben; beim nächsten PotiStart wird dieser Wert übernommen
#define	UART_VDD	        0x13	//	2	2	Spannung VDD einstellen
#define	UART_I_VDD	        0x14	//	/	2	Strom durch VDD lesen
#define	UART_VDD_SPARK	        0x15	//	2	2	Spannung einstellen, bei der das Plasma zündet. Diese Spannung wird bei der automatischen Zündung kurz nach Einschalten von Vgg kurz an Vdd angelegt
#define	UART_READ_VDD	        0x16	//	/	2	lesen der gemessenen Spannung VDD
							
//VGG (-6..-1V; -1..10mA)			//			
#define	UART_EN_VGG	        0x20	//	1	1	Einschalten des SwitchedCap-Spannungsreglers IC40 (VGG)
#define	UART_VGG	        0x21	//	2	2	Spannung VGG einstellen
#define	UART_I_VGG	        0x22	//	/	2	Strom durch VGG lesen
#define	UART_VGG_SPARK	        0x23	//	2	2	Spannung einstellen, bei der das Plasma zündet. Diese Spannung wird bei der automatischen Zündung kurz nach Einschalten von Vdd kurz an Vgg angelegt
#define	UART_READ_VGG	        0x24	//	/	2	lesen der gemessenen Spannung VGG 
							
//MW // Mikrowellenparameter			//			
#define	UART_MW_PWR_TO	        0x30	//	/	2	Leistung, die in die Mikrowelle geht
#define	UART_MW_PWR_BACK	0x31	//	/	2	Leistung, die von der Mikrowelle kommt

//Steuerspannung für Kapazitäten zur Steuerung des Arbeitspunktes						
#define	UART_CTRL1	        0x40	//	2	2	Steuerspannung CTRL1
#define	UART_CTRL2	        0x41	//	2	2	Steuerspannung CTRL2
#define	UART_VCTRL1_SPARK	0x42	//	2	2	Steuerspannung CTRL1, welche während des Zündvorgangs angelegt wird
#define	UART_VCTRL2_SPARK	0x43	//	2	2	Steuerspannung CTRL2, welche während des Zündvorgangs angelegt wird

//*******************************ERROR-Codes
#define	NO_ERROR			0
#define	ERROR_TIMEOUT_BEFORE_1st_BYTE	1
#define	ERROR_TIMEOUT_BEFORE_2nd_BYTE	2
#define	ERROR_NOT_ENOUGH_DATABYTES	3
#define	ERROR_WRONG_CMD			4
#define	ERROR_VGG_NOT_RIGHT     	5
#define	ERROR_CHECKSUM			7

////////////////////////////////////////////////////////////////////////////////PORTS//////////////////////////////////////////////////
//*******************************SPI-Port
#define SPI_nCS_POT_PORT GPIOC//chip select des elektronischen Potis für Einstellung von VGG und VDD
#define SPI_nCS_POT_PIN 1
#define SPI_nCS_ADC1_PORT GPIOA
#define SPI_nCS_ADC1_PIN 3
#define SPI_nCS_ADC2_PORT GPIOC
#define SPI_nCS_ADC2_PIN 4
#define SPI_nCS_DAC_PORT GPIOC
#define SPI_nCS_DAC_PIN 0


#define SPI_SCK_PORT GPIOC //Clock für SPI
#define SPI_SCK_PIN     5

#define SPI_MOSI_PORT GPIOC
#define SPI_MOSI_PIN    6

#define SPI_MISO_PORT GPIOA
#define SPI_MISO_PIN    2

//******************************UART-Port
#define UART_PORT GPIOC
#define UART_RX_PIN     2
#define UART_TX_PIN     3

//******************************Spannungsversorgung VDD
#define EN_VDD_PORT GPIOA
#define EN_VDD_PIN      6

#define VDD_GOOD_PORT GPIOA
#define VDD_GOOD_PIN    5

//******************************Spannungsversorgung VGG
#define EN_VGG_PORT GPIOA
#define EN_VGG_PIN      4

//******************************Spannungsversorgung 5V für uC (über IC60->3V3) und BIAS IC20
#define V5V_GOOD_PORT GPIOD
#define V5V_GOOD_PIN    0

//******************************Display-LED
#define LED_PORT GPIOD
#define LED_RED_PIN     5
#define LED_GREEN_PIN   6
#define LED_BLUE_PIN    7

//******************************Taster
#define SWITCH_PORT GPIOB
#define SWITCH_PIN      4

/*-------------------------------------------------------------------------
 *      Interrupt vector numbers
 *-----------------------------------------------------------------------*/
#define TRAP_vector                          0x01
//RESERVED
#define FLASH_vector                         0x03
#define DMA1_CH0_CH1_vector                  0x04
#define DMA1_CH2_CH3_vector                  0x05
#define RTC_vector                           0x06
#define EXTIE_PVD_vector                     0x07
#define EXTIB_vector                         0x08
#define EXTID_vector                         0x09
#define EXTI0_vector                         0x0A
#define EXTI1_vector                         0x0B
#define EXTI2_vector                         0x0C
#define EXTI3_vector                         0x0D
#define EXTI4_vector                         0x0E
#define EXTI5_vector                         0x0F
#define EXTI6_vector                         0x10
#define EXTI7_vector                         0x11
//LCD
#define CLK_CSS_vector                       0x13
#define CLK_SWITCH_vector                    0x13
#define TIM1_BIF_vector                      0x13
#define COMP_EF1_EF2_vector                  0x14
#define TIM2_OVR_UIF_vector                  0x15
#define TIM2_CAPCOM_TIF_vector               0x16
#define TIM3_OVR_UIF_vector                  0x17
#define TIM3_CAPCOM_TIF_vector               0x18
#define TIM1_OVR_UIF_vector                  0x19
#define TIM1_CAPCOM_vector                   0x1A
#define TIM4_TIF_UIF_vector                  0x1B
#define SPI_vector                           0x1C
#define USART_T_vector                       0x1D
#define USART_R_vector                       0x1E
#define I2C_vector                           0x1F

//#endif