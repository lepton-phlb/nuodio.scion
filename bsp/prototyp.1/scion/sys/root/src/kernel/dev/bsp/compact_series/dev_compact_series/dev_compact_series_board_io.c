/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2015 <lepton.phlb@gmail.com>.
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


/*===========================================
Includes
=============================================*/
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/cpu.h"
#include "kernel/core/dirent.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/core/ioctl_board.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/uart.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/spi.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_gpio_ex.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_uart_x.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_spi_x.h"

#include "kernel/dev/bsp/compact_series/dev_compact_series/dev_rotary_encoder_x.h"
#include "kernel/dev/bsp/compact_series/dev_compact_series/dev_compact_series_board_io.h"

/*===========================================
Global Declaration
=============================================*/

const char dev_compact_series_board_io_name[]="brdio\0";

int dev_compact_series_board_io_load(void);
int dev_compact_series_board_io_open(desc_t desc, int o_flag);
int dev_compact_series_board_io_close(desc_t desc);
int dev_compact_series_board_io_isset_read(desc_t desc);
int dev_compact_series_board_io_isset_write(desc_t desc);
int dev_compact_series_board_io_read(desc_t desc, char* buf,int size);
int dev_compact_series_board_io_write(desc_t desc, const char* buf,int size);
int dev_compact_series_board_io_seek(desc_t desc,int offset,int origin);
int dev_compact_series_board_io_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_compact_series_board_io_map={
   dev_compact_series_board_io_name,
   S_IFBLK,
   dev_compact_series_board_io_load,
   dev_compact_series_board_io_open,
   dev_compact_series_board_io_close,
   dev_compact_series_board_io_isset_read,
   dev_compact_series_board_io_isset_write,
   dev_compact_series_board_io_read,
   dev_compact_series_board_io_write,
   dev_compact_series_board_io_seek,
   dev_compact_series_board_io_ioctl //ioctl
};

// GPIO definition  see in dev_hybrid_board.h
const _Gpio_Descriptor Gpio_Descriptor[] = {
  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_9,   0,  GPIO_MODE_IN,       0},   // GPIO_TXD1
  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_10,  0,  GPIO_MODE_IN,       0},   // GPIO_RXD1
  
  {GPIO_TYPE_STD, GPIOD,  GPIO_Pin_8,   0,  GPIO_MODE_IN,       0},   // GPIO_TXD3
  {GPIO_TYPE_STD, GPIOD,  GPIO_Pin_9,   0,  GPIO_MODE_IN,       0},   // GPIO_RXD3

  {GPIO_TYPE_STD, GPIOG,  GPIO_Pin_14,  0,  GPIO_MODE_IN,       0},   // GPIO_TXD6
  {GPIO_TYPE_STD, GPIOG,  GPIO_Pin_9,   0,  GPIO_MODE_IN,       0},   // GPIO_RXD6
  
  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_7,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_FX_MOSI
  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_6,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_FX_MISO
  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_5,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_FX_SCLK
  
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_9,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_OLED_MOSI
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_7,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_OLED_SCLK
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_10,  1,  GPIO_MODE_OUT,      1},   // GPIO_ID_OLED_DC
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_6,   1,  GPIO_MODE_OUT,      1},   // GPIO_ID_OLED_CS
  {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_0,   1,  GPIO_MODE_OUT,      1},   // GPIO_ID_OLED_RESET
};



// peripheral descriptor definition. specific for each BSP.

// uart 1 MIDI
board_stm32f4xx_uart_info_t stm32f4xx_uart_1=
{
   .uart_descriptor={USART1, RCC_APB2PeriphClockCmd, RCC_APB2Periph_USART1, USART1_IRQn, DMA2_Stream5, DMA_Channel_4, DMA2_Stream5_IRQn, GPIO_TXD1, GPIO_RXD1, GPIO_AF_USART1, &Uart_Ctrl[UART_ID_1]}   // UART_3
};

// uart 3 debug console
board_stm32f4xx_uart_info_t stm32f4xx_uart_3=
{
   .uart_descriptor={USART3, RCC_APB1PeriphClockCmd, RCC_APB1Periph_USART3, USART3_IRQn, DMA1_Stream1, DMA_Channel_4, DMA1_Stream1_IRQn, GPIO_TXD3, GPIO_RXD3, GPIO_AF_USART3, &Uart_Ctrl[UART_ID_3]}   // UART_3
};

// uart 6 BLE Nordic nrf52
board_stm32f4xx_uart_info_t stm32f4xx_uart_6=
{
   .uart_descriptor={USART6, RCC_APB2PeriphClockCmd, RCC_APB2Periph_USART6, USART6_IRQn, DMA2_Stream1, DMA_Channel_5, DMA2_Stream1_IRQn, GPIO_TXD6, GPIO_RXD6, GPIO_AF_USART6, &Uart_Ctrl[UART_ID_6]}   // UART_3
};


// spi 1 CPHA=1 CPOL=0
board_stm32f4xx_spi_info_t compact_series_spi_1=
{
   .spi_descriptor={SPI1,  RCC_APB2PeriphClockCmd, RCC_APB2ENR_SPI1EN,  GPIO_FX_MISO,  GPIO_FX_MOSI,  GPIO_FX_SCLK, GPIO_AF5_SPI1, 1, 200000} // SPI1 Digital potentiometer
};


// spi 5
board_stm32f4xx_spi_info_t compact_series_spi_5=
{
   .spi_descriptor={SPI5,  RCC_APB2PeriphClockCmd, RCC_APB2ENR_SPI5EN,  (void*)0,  GPIO_OLED_MOSI,  GPIO_OLED_SCLK, GPIO_AF5_SPI5, 0, 20000000} // SPI5 SSD1322 LCD OLED 
};


rotary_encoder_info_t compact_series_rotary_encoder_info_1 ={
  .pin_a.gpio_bank_no=GPIOH,
  .pin_a.gpio_pin_no=GPIO_Pin_15,
  .pin_b.gpio_bank_no=GPIOH,
  .pin_b.gpio_pin_no=GPIO_Pin_14,
  .pin_s.gpio_bank_no=GPIOH,
  .pin_s.gpio_pin_no=GPIO_Pin_13,
  .exti_pin.src_gpio_bank_no=EXTI_PortSourceGPIOH,
  .exti_pin.src_gpio_pin_no=EXTI_PinSource14
};


rotary_encoder_info_t compact_series_rotary_encoder_info_2 ={
  .pin_a.gpio_bank_no=GPIOG,
  .pin_a.gpio_pin_no=GPIO_Pin_6,
  .pin_b.gpio_bank_no=GPIOG,
  .pin_b.gpio_pin_no=GPIO_Pin_5,
  .pin_s.gpio_bank_no=GPIOG,
  .pin_s.gpio_pin_no=GPIO_Pin_7,
  .exti_pin.src_gpio_bank_no=EXTI_PortSourceGPIOG,
  .exti_pin.src_gpio_pin_no=EXTI_PinSource5
};

rotary_encoder_info_t compact_series_rotary_encoder_info_3 ={
  .pin_a.gpio_bank_no=GPIOG,
  .pin_a.gpio_pin_no=GPIO_Pin_3,
  .pin_b.gpio_bank_no=GPIOG,
  .pin_b.gpio_pin_no=GPIO_Pin_2,
  .pin_s.gpio_bank_no=GPIOG,
  .pin_s.gpio_pin_no=GPIO_Pin_4,
  .timer_no=TIM4,
  .timer_gpio_af= GPIO_AF_TIM4,
  .exti_pin.src_gpio_bank_no=EXTI_PortSourceGPIOG,
  .exti_pin.src_gpio_pin_no= EXTI_PinSource2
};



/*===========================================
Implementation
=============================================*/

void EXTI2_IRQHandler(void){
  dev_rotary_encoder_x_interrupt(&compact_series_rotary_encoder_info_3);
}

void EXTI9_5_IRQHandler(void){
  dev_rotary_encoder_x_interrupt(&compact_series_rotary_encoder_info_2);
}

void EXTI15_10_IRQHandler(void){
  dev_rotary_encoder_x_interrupt(&compact_series_rotary_encoder_info_1);
}



/*******************************************************************************
* Function Name  : gpio_startup_init
* Description    : Initialize all defined GPIOs
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void gpio_startup_init(void)
{
  u8 i;

  /* Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | \
                          RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI, ENABLE);

  /* Initialize GPIO */
  for (i = 0 ; i < GPIO_NB ; i++) if (Gpio_Descriptor[i].Init) gpio_init((&Gpio_Descriptor[i]));
        
}

/*-------------------------------------------
| Name:dev_compact_series_board_io_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_load(void){
   //
   gpio_startup_init();
   dma_startup_init();
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_board_io_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_board_io_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_close(desc_t desc){
   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_board_io_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_compact_series_board_io_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_compact_series_board_io_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_read(desc_t desc, char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_compact_series_board_io_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_write(desc_t desc, const char* buf,int size){
   //read only mode
   return -1;
}

/*-------------------------------------------
| Name:dev_a0350_board_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_seek(desc_t desc,int offset,int origin){

   switch(origin) {

   case SEEK_SET:
      ofile_lst[desc].offset=offset;
      break;

   case SEEK_CUR:
      ofile_lst[desc].offset+=offset;
      break;

   case SEEK_END:
      //to do: warning in SEEK_END (+ or -)????
      ofile_lst[desc].offset-=offset;
      break;
   }

   return ofile_lst[desc].offset;
}

/*-------------------------------------------
| Name:dev_compact_series_board_io_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_board_io_ioctl(desc_t desc,int request,va_list ap){

   switch(request) {
   case BRDPWRDOWN:   //power down
      break;
   //
   case BRDRESET:   //reset
      break;

   case BRDWATCHDOG:   //start watchdog
      break;
   //
   case BRDCFGPORT: {
   }
   break;

   case BRDBEEP: {
   }
   break;

   //
   default:
      return -1;

   }

   return 0;
}

/*============================================
| End of Source  : dev_compact_series_board_io.c
==============================================*/