/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2011 <lepton.phlb@gmail.com>.
All Rights Reserved.

Contributor(s): Jean-Jacques Pitrolle <lepton.jjp@gmail.com>.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/


/*============================================
| Compiler Directive
==============================================*/
#ifndef __DEV_HYBRID_TUBE_BOARD_H__
#define __DEV_HYBRID_TUBE_BOARD_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/
#define _GPIO_DEFAULT_SPEED   GPIO_Speed_50MHz

typedef enum
{
  GPIO_ID_TXD3, 
  GPIO_ID_RXD3,

  GPIO_ID_TXD6,
  GPIO_ID_RXD6,

  GPIO_ID_FX_MOSI,
  GPIO_ID_FX_MISO,
  GPIO_ID_FX_SCLK,
  
  GPIO_ID_CODEC_MOSI,
  GPIO_ID_CODEC_MISO,
  GPIO_ID_CODEC_SCLK,
  
  GPIO_ID_OLED_MOSI,
  GPIO_ID_OLED_SCLK,
  GPIO_ID_OLED_DC,
  GPIO_ID_OLED_CS,
  GPIO_ID_OLED_RESET,

  GPIO_ID_LED_R,
  GPIO_ID_LED_V,
  GPIO_ID_LED_B,

  GPIO_NB
} _GPIO_LIST;

#define GPIO_TXD3  (&Gpio_Descriptor[GPIO_ID_TXD3])
#define GPIO_RXD3  (&Gpio_Descriptor[GPIO_ID_RXD3])

#define GPIO_TXD6     (&Gpio_Descriptor[GPIO_ID_TXD6])
#define GPIO_RXD6     (&Gpio_Descriptor[GPIO_ID_RXD6])


#define GPIO_FX_MOSI     (&Gpio_Descriptor[GPIO_ID_FX_MOSI])
#define GPIO_FX_MISO     (&Gpio_Descriptor[GPIO_ID_FX_MISO])
#define GPIO_FX_SCLK     (&Gpio_Descriptor[GPIO_ID_FX_SCLK])

#define GPIO_CODEC_MOSI     (&Gpio_Descriptor[GPIO_ID_CODEC_MOSI])
#define GPIO_CODEC_MISO     (&Gpio_Descriptor[GPIO_ID_CODEC_MISO])
#define GPIO_CODEC_SCLK     (&Gpio_Descriptor[GPIO_ID_CODEC_SCLK])

#define GPIO_OLED_MOSI     (&Gpio_Descriptor[GPIO_ID_OLED_MOSI])
#define GPIO_OLED_SCLK     (&Gpio_Descriptor[GPIO_ID_OLED_SCLK])
#define GPIO_OLED_DC       (&Gpio_Descriptor[GPIO_ID_OLED_DC])
#define GPIO_OLED_CS       (&Gpio_Descriptor[GPIO_ID_OLED_CS])
#define GPIO_OLED_RESET    (&Gpio_Descriptor[GPIO_ID_OLED_RESET])



#define GPIO_LED_R  (&Gpio_Descriptor[GPIO_ID_LED_R])
#define GPIO_LED_V  (&Gpio_Descriptor[GPIO_ID_LED_V])
#define GPIO_LED_B  (&Gpio_Descriptor[GPIO_ID_LED_B])


#define GPIO_BYPASS_COMMAND_BANK_NO
#define GPIO_BYPASS_COMMAND_BANK_NO


/*******************************************************************************
* UART
*******************************************************************************/
//#define _UART_OS_SUPPORT

typedef enum
{
  UART_ID_3,
  UART_ID_6,
  UART_NB
} _UART_LIST;

//#define UART_3  (&Uart_Descriptor[UART_ID_3])
//#define UART_6  (&Uart_Descriptor[UART_ID_6])


/*******************************************************************************
* SPI
*******************************************************************************/
typedef enum
{
  SPI_ID_FX,
  SPI_ID_OLED,
  SPI_NB
} _SPI_LIST;

//#define SPI_OLED   (&Spi_Descriptor[SPI_ID_OLED])

/*******************************************************************************
* SPIFLASH
*******************************************************************************/
#define SPIFLASH_AT_FIRM_FROM_SECTOR   57
#define SPIFLASH_AT_FIRM_SECTOR_CNT    7
#define SPIFLASH_ST_FIRM_FROM_SECTOR   25
#define SPIFLASH_ST_FIRM_SECTOR_CNT    7

#endif //__DEV_HYBRID_TUBE_BOARD_H__