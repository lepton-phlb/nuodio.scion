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
#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfsdev.h"
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
#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/dev_rotary_encoder_x.h"

#include "dev_hybrid_tube_board.h"

/*===========================================
Global Declaration
=============================================*/

const char dev_hybrid_tube_board_name[]="board\0";

int dev_hybrid_tube_board_load(void);
int dev_hybrid_tube_board_open(desc_t desc, int o_flag);
int dev_hybrid_tube_board_close(desc_t desc);
int dev_hybrid_tube_board_isset_read(desc_t desc);
int dev_hybrid_tube_board_isset_write(desc_t desc);
int dev_hybrid_tube_board_read(desc_t desc, char* buf,int size);
int dev_hybrid_tube_board_write(desc_t desc, const char* buf,int size);
int dev_hybrid_tube_board_seek(desc_t desc,int offset,int origin);
int dev_hybrid_tube_board_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_hybrid_tube_board_map={
   dev_hybrid_tube_board_name,
   S_IFBLK,
   dev_hybrid_tube_board_load,
   dev_hybrid_tube_board_open,
   dev_hybrid_tube_board_close,
   dev_hybrid_tube_board_isset_read,
   dev_hybrid_tube_board_isset_write,
   dev_hybrid_tube_board_read,
   dev_hybrid_tube_board_write,
   dev_hybrid_tube_board_seek,
   dev_hybrid_tube_board_ioctl //ioctl
};

// GPIO definition  see in dev_hybrid_board.h
const _Gpio_Descriptor Gpio_Descriptor[] = {
  {GPIO_TYPE_STD, GPIOD,  GPIO_Pin_8,   0,  GPIO_MODE_IN,       0},   // GPIO_TXD3
  {GPIO_TYPE_STD, GPIOD,  GPIO_Pin_9,   0,  GPIO_MODE_IN,       0},   // GPIO_RXD3

  {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_6,   0,  GPIO_MODE_IN,       0},   // GPIO_TXD6
  {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_7,   0,  GPIO_MODE_IN,       0},   // GPIO_RXD6
  
  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_7,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_FX_MOSI
  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_6,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_FX_MISO
  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_5,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_FX_SCLK
  
  {GPIO_TYPE_STD, GPIOE,  GPIO_Pin_6,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_CODEC_MOSI
  {GPIO_TYPE_STD, GPIOE,  GPIO_Pin_5,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_CODEC_MISO
  {GPIO_TYPE_STD, GPIOE,  GPIO_Pin_2,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_CODEC_SCLK
  
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_9,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_OLED_MOSI
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_7,   0,  GPIO_MODE_IN,       0},   // GPIO_ID_OLED_SCLK
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_6,   1,  GPIO_MODE_OUT,      1},   // GPIO_ID_OLED_DC
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_5,   1,  GPIO_MODE_OUT,      1},   // GPIO_ID_OLED_CS
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_4,   1,  GPIO_MODE_OUT,      1},   // GPIO_ID_OLED_RESET
 

  {GPIO_TYPE_STD, GPIOE,  GPIO_Pin_13,   1,  GPIO_MODE_OUT,      1},   // GPIO_LED_R
  {GPIO_TYPE_STD, GPIOE,  GPIO_Pin_14,   1,  GPIO_MODE_OUT,      1},   // GPIO_LED_V
  {GPIO_TYPE_STD, GPIOE,  GPIO_Pin_15,   1,  GPIO_MODE_OUT,      1},   // GPIO_LED_B
  
};



// peripheral descriptor definition. specific for each BSP.
// uart 3
board_stm32f4xx_uart_info_t stm32f4xx_uart_3=
{
   .uart_descriptor={USART3, RCC_APB1PeriphClockCmd, RCC_APB1Periph_USART3, USART3_IRQn, DMA1_Stream1, DMA_Channel_4, DMA1_Stream1_IRQn, GPIO_TXD3, GPIO_RXD3, GPIO_AF_USART3, &Uart_Ctrl[UART_ID_3]}   // UART_3
};

// uart 6
board_stm32f4xx_uart_info_t stm32f4xx_uart_6=
{
   .uart_descriptor={USART6, RCC_APB2PeriphClockCmd, RCC_APB2Periph_USART6, USART6_IRQn, DMA2_Stream1, DMA_Channel_5, DMA2_Stream1_IRQn, GPIO_TXD6, GPIO_RXD6, GPIO_AF_USART6, &Uart_Ctrl[UART_ID_6]}   // UART_3
};


// spi 1
board_stm32f4xx_spi_info_t hybrid_tube_spi_1=
{
   .spi_descriptor={SPI1,  RCC_APB2PeriphClockCmd, RCC_APB2ENR_SPI1EN,  GPIO_FX_MISO,  GPIO_FX_MOSI,  GPIO_FX_SCLK, GPIO_AF5_SPI1, 0, 20000000} // SPI1 Digital potentiometer
};

// spi 4
board_stm32f4xx_spi_info_t hybrid_tube_spi_4=
{
   .spi_descriptor={SPI4,  RCC_APB2PeriphClockCmd, RCC_APB2ENR_SPI4EN,  GPIO_CODEC_MISO,  GPIO_CODEC_MOSI,  GPIO_CODEC_SCLK, GPIO_AF5_SPI4, 0, 1000000} // SPI4 CODEC 1MHz  
};


// spi 5
board_stm32f4xx_spi_info_t hybrid_tube_spi_5=
{
   .spi_descriptor={SPI5,  RCC_APB2PeriphClockCmd, RCC_APB2ENR_SPI5EN,  (void*)0,  GPIO_OLED_MOSI,  GPIO_OLED_SCLK, GPIO_AF5_SPI5, 0, 20000000} // SPI5 SSD1322 LCD OLED 
};




rotary_encoder_info_t rotary_encoder_info_1 ={
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


rotary_encoder_info_t rotary_encoder_info_2 ={
  .pin_a.gpio_bank_no=GPIOG,
  .pin_a.gpio_pin_no=GPIO_Pin_6,
  .pin_b.gpio_bank_no=GPIOG,
  .pin_b.gpio_pin_no=GPIO_Pin_5,
  .pin_s.gpio_bank_no=GPIOG,
  .pin_s.gpio_pin_no=GPIO_Pin_7,
  .exti_pin.src_gpio_bank_no=EXTI_PortSourceGPIOG,
  .exti_pin.src_gpio_pin_no=EXTI_PinSource5
};

rotary_encoder_info_t rotary_encoder_info_3 ={
  .pin_a.gpio_bank_no=GPIOH,
  .pin_a.gpio_pin_no=GPIO_Pin_15,
  .pin_b.gpio_bank_no=GPIOH,
  .pin_b.gpio_pin_no=GPIO_Pin_14,
  .pin_s.gpio_bank_no=GPIOH,
  .pin_s.gpio_pin_no=GPIO_Pin_13,
  .exti_pin.src_gpio_bank_no=EXTI_PortSourceGPIOH,
  .exti_pin.src_gpio_pin_no=EXTI_PinSource14
};

rotary_encoder_info_t rotary_encoder_info_4 ={
  .pin_a.gpio_bank_no=GPIOD,
  .pin_a.gpio_pin_no=GPIO_Pin_5,
  .pin_b.gpio_bank_no=GPIOD,
  .pin_b.gpio_pin_no=GPIO_Pin_4,
  .pin_s.gpio_bank_no=GPIOD,
  .pin_s.gpio_pin_no=GPIO_Pin_3,
  .exti_pin.src_gpio_bank_no=EXTI_PortSourceGPIOD,
  .exti_pin.src_gpio_pin_no=EXTI_PinSource4
};



/*===========================================
Implementation
=============================================*/

void EXTI2_IRQHandler(void){
  dev_rotary_encoder_x_interrupt(&rotary_encoder_info_1);
}

void EXTI4_IRQHandler(void){
  dev_rotary_encoder_x_interrupt(&rotary_encoder_info_4);
}

void EXTI9_5_IRQHandler(void){
  dev_rotary_encoder_x_interrupt(&rotary_encoder_info_2);
}

void EXTI15_10_IRQHandler(void){
  dev_rotary_encoder_x_interrupt(&rotary_encoder_info_3);
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
| Name:dev_hybrid_tube_board_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_board_load(void){
   //
   gpio_startup_init();
   dma_startup_init();
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_board_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_board_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_board_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_board_close(desc_t desc){
   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_board_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_board_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_board_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_board_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_hybrid_tube_board_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_board_read(desc_t desc, char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_board_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_board_write(desc_t desc, const char* buf,int size){
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
int dev_hybrid_tube_board_seek(desc_t desc,int offset,int origin){

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
| Name:dev_hybrid_tube_board_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_board_ioctl(desc_t desc,int request,va_list ap){

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
| End of Source  : dev_hybrid_tube_board.c
==============================================*/