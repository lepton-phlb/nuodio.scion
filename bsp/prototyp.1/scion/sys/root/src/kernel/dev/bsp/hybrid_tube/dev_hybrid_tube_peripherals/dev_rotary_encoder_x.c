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


/*============================================
| Includes
==============================================*/

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_spi.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfsdev.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_gpio_ex.h"

#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/ioctl_rotary_encoder.h"
#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/dev_rotary_encoder_x.h"


/*============================================
| Global Declaration
==============================================*/


/*============================================
| Implementation
==============================================*/

static void Encoder_Configration(void)
{
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   GPIO_InitTypeDef GPIO_InitStructure;
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
   GPIO_Init(GPIOG, &GPIO_InitStructure);
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
   GPIO_Init(GPIOG, &GPIO_InitStructure);

   GPIO_PinAFConfig(GPIOG, GPIO_PinSource4, GPIO_AF_TIM8);
   GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_TIM8);

   //TIM_SetAutoreload (TIM8, 0xffffffff);
   /* Configure the timer */
   /* Time base configuration */
   TIM_TimeBaseStructure.TIM_Period = 0xffff;
   TIM_TimeBaseStructure.TIM_Prescaler = 0;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
   TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

   TIM_EncoderInterfaceConfig(TIM8, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Falling);
   //
   //TIM_ITConfig(TIM8,TIM_IT_Update , ENABLE);
   /* TIM2 counter enable */
   TIM_Cmd(TIM8, ENABLE);
}

static void encoder_configuration_timer(rotary_encoder_info_t* rotary_encoder_info)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   
   //
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
   //
   //
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   //pin A
   GPIO_InitStructure.GPIO_Pin = rotary_encoder_info->pin_a.gpio_pin_no;
   GPIO_Init(rotary_encoder_info->pin_a.gpio_bank_no, &GPIO_InitStructure);
   //pin B
   GPIO_InitStructure.GPIO_Pin = rotary_encoder_info->pin_b.gpio_pin_no;
   GPIO_Init(rotary_encoder_info->pin_b.gpio_bank_no, &GPIO_InitStructure);
   //
   GPIO_PinAFConfig(rotary_encoder_info->pin_a.gpio_bank_no, GPIO_PinSource3, rotary_encoder_info->timer_gpio_af);
   GPIO_PinAFConfig(rotary_encoder_info->pin_b.gpio_bank_no, GPIO_PinSource2, rotary_encoder_info->timer_gpio_af);

  
   /* Configure the timer */
   /* Timer configuration in Encoder mode for left encoder*/

   TIM_TimeBaseStructure.TIM_Prescaler = 0x00;  // No prescaling
   TIM_TimeBaseStructure.TIM_Period = 0xffffffff; //max resolution
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//devide by clock by one
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;// count up  
   TIM_TimeBaseInit(rotary_encoder_info->timer_no, &TIM_TimeBaseStructure);
   //TIM_SetCounter (rotary_encoder_info->timer_no, (uint32_t) 0); 

   //
   TIM_SetAutoreload (rotary_encoder_info->timer_no, 0xffffffff);
   //
   TIM_EncoderInterfaceConfig(rotary_encoder_info->timer_no, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Falling);
   //
   TIM_ITConfig(TIM1,TIM_IT_Update , ENABLE);
   /* TIM2 counter enable */
   TIM_Cmd(rotary_encoder_info->timer_no, ENABLE);
}

static void encoder_configuration_interrupt(rotary_encoder_info_t* rotary_encoder_info)
{
   EXTI_InitTypeDef EXTI_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   //
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   //pin A
   GPIO_InitStructure.GPIO_Pin = rotary_encoder_info->pin_a.gpio_pin_no;
   GPIO_Init(rotary_encoder_info->pin_a.gpio_bank_no, &GPIO_InitStructure);
   //pin B
   GPIO_InitStructure.GPIO_Pin = rotary_encoder_info->pin_b.gpio_pin_no;
   GPIO_Init(rotary_encoder_info->pin_b.gpio_bank_no, &GPIO_InitStructure);
   //pin S
   GPIO_InitStructure.GPIO_Pin = rotary_encoder_info->pin_s.gpio_pin_no;
   GPIO_Init(rotary_encoder_info->pin_s.gpio_bank_no, &GPIO_InitStructure);
   
 
   /* Configure EXTI LineX */
   switch(rotary_encoder_info->exti_pin.src_gpio_pin_no){
      case EXTI_PinSource0:
          EXTI_InitStructure.EXTI_Line = EXTI_Line0;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
      break;
      case EXTI_PinSource1:
          EXTI_InitStructure.EXTI_Line = EXTI_Line1;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
      break;
      case EXTI_PinSource2:
          EXTI_InitStructure.EXTI_Line = EXTI_Line2;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
      break;
       case EXTI_PinSource3:
          EXTI_InitStructure.EXTI_Line = EXTI_Line3;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
      break;
      case EXTI_PinSource4:
          EXTI_InitStructure.EXTI_Line = EXTI_Line4;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
      break;
       case EXTI_PinSource5:
          EXTI_InitStructure.EXTI_Line = EXTI_Line5;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
      break;
      case EXTI_PinSource6:
          EXTI_InitStructure.EXTI_Line = EXTI_Line6;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
      break;
       case EXTI_PinSource7:
          EXTI_InitStructure.EXTI_Line = EXTI_Line7;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
      break;
      case EXTI_PinSource8:
          EXTI_InitStructure.EXTI_Line = EXTI_Line8;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
      break;
       case EXTI_PinSource9:
          EXTI_InitStructure.EXTI_Line = EXTI_Line9;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
      break;
      case EXTI_PinSource10:
          EXTI_InitStructure.EXTI_Line = EXTI_Line10;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
      break;
       case EXTI_PinSource11:
          EXTI_InitStructure.EXTI_Line = EXTI_Line11;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
      break;
      case EXTI_PinSource12:
          EXTI_InitStructure.EXTI_Line = EXTI_Line12;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
      break;
      case EXTI_PinSource13:
          EXTI_InitStructure.EXTI_Line = EXTI_Line13;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
      break;
      case EXTI_PinSource14:
          EXTI_InitStructure.EXTI_Line = EXTI_Line14;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
      break;
      case EXTI_PinSource15:
          EXTI_InitStructure.EXTI_Line = EXTI_Line15;
          NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
      break;
      //
      default:
        return;
   }
   
   //
   rotary_encoder_info->EXTI_Line =  EXTI_InitStructure.EXTI_Line;
   
   /* Enable SYSCFG clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
   //
   /* Connect EXTI LineX to PA0 pin */
   SYSCFG_EXTILineConfig(rotary_encoder_info->exti_pin.src_gpio_bank_no,rotary_encoder_info->exti_pin.src_gpio_pin_no);
   
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //EXTI_Trigger_Rising_Falling
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI LineX Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
 
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_interrupt
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
void dev_rotary_encoder_x_interrupt(rotary_encoder_info_t* rotary_encoder_info){
   //
   __hw_enter_interrupt();
   //
   if(EXTI_GetITStatus(rotary_encoder_info->EXTI_Line) != RESET){
      //
      uchar8_t pin_a = GPIO_ReadInputDataBit(rotary_encoder_info->pin_a.gpio_bank_no,rotary_encoder_info->pin_a.gpio_pin_no);
      uchar8_t pin_b = GPIO_ReadInputDataBit(rotary_encoder_info->pin_b.gpio_bank_no,rotary_encoder_info->pin_b.gpio_pin_no);
      
       if (pin_a == pin_b) {
         rotary_encoder_info->counter++;
      } else {
         rotary_encoder_info->counter--;
      }
      //
      if(rotary_encoder_info->input_w==rotary_encoder_info->input_r){
         if(rotary_encoder_info->desc_r!=-1)
            __fire_io_int(ofile_lst[rotary_encoder_info->desc_r].owner_pthread_ptr_read);
      }
      //
      rotary_encoder_info->input_w++;
        
      /* Clear the EXTI line 0 pending bit */
      EXTI_ClearITPendingBit(rotary_encoder_info->EXTI_Line);
   }
   //
  __hw_leave_interrupt();
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_rotary_encoder_x_load(rotary_encoder_info_t* rotary_encoder_info){
     rotary_encoder_info->desc_r=INVALID_DESC;
   return 0;
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_rotary_encoder_x_open(desc_t desc, int o_flag, rotary_encoder_info_t* rotary_encoder_info){

   //
   if(o_flag & O_RDONLY) {
      if(rotary_encoder_info->desc_r<0) {
         rotary_encoder_info->desc_r = desc;
         //
         rotary_encoder_info->input_r=0;
         rotary_encoder_info->input_w=0;
         rotary_encoder_info->counter = 255;
         //
         //Encoder_Configration();
         //encoder_configuration(rotary_encoder_info);
         encoder_configuration_interrupt(rotary_encoder_info);
        
   }   
      else
         return -1;                //already open
   }

   if(o_flag & O_WRONLY) {
         return -1;                //only open in read mode
   }

   if(!ofile_lst[desc].p)
      ofile_lst[desc].p=rotary_encoder_info;

   
   if(rotary_encoder_info->desc_r>=0) {
     //unmask IRQ
   }
   return 0;

}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_rotary_encoder_x_close(desc_t desc){

   return 0;
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_rotary_encoder_x_isset_read(desc_t desc){
  rotary_encoder_info_t* rotary_encoder_info = ofile_lst[desc].p;
  //
  if(rotary_encoder_info==(rotary_encoder_info_t*)0)
        return -1;
  //
  if(rotary_encoder_info->input_r!=rotary_encoder_info->input_w)
    return 0;
  //
  return -1;
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_rotary_encoder_x_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_rotary_encoder_x_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_rotary_encoder_x_read(desc_t desc, char* buf,int size){
  
   //
   if(ofile_lst[desc].oflag & O_RDONLY) {
      rotary_encoder_info_t* rotary_encoder_info = ofile_lst[desc].p;
      int32_t counter=0;
      if(rotary_encoder_info==(rotary_encoder_info_t*)0)
        return -1;
      //
      if(size<sizeof(counter))
        return -1;
      
      //counter = TIM_GetCounter(rotary_encoder_info->timer_no);
      counter=rotary_encoder_info->counter;
      //
      rotary_encoder_info->input_r=rotary_encoder_info->input_w;
      //
      memcpy(buf,&counter,sizeof(counter));
      //
      return sizeof(counter);
   }
   //
   return -1;
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_rotary_encoder_x_write(desc_t desc, const char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_rotary_encoder_x_seek(desc_t desc,int offset,int origin){
   return -1;
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/

int dev_rotary_encoder_x_ioctl(desc_t desc,int request,va_list ap){
   rotary_encoder_info_t* rotary_encoder_info = ofile_lst[desc].p;
   //
   if(rotary_encoder_info==(rotary_encoder_info_t*)0)
        return -1;
   //
   switch(request) {
      case ROTRYSWTCH:{
         uchar8_t* p_vu8= va_arg( ap, uchar8_t*);
         uchar8_t pin_s = GPIO_ReadInputDataBit(rotary_encoder_info->pin_s.gpio_bank_no,rotary_encoder_info->pin_s.gpio_pin_no);
         *p_vu8 = pin_s;
      }
      break;
      //
      case ROTRYSETCOUNT:{
         int32_t* p_counter= va_arg( ap, int32_t*);
         rotary_encoder_info->counter= *p_counter;
      }
      break;
      //
      case ROTRYGETCOUNT:{
         int32_t* p_counter= va_arg( ap, int32_t*);
         *p_counter = rotary_encoder_info->counter;
      }
      break;
      //
      default:
         return -1;
   }

   return 0;
}


/*============================================
| End of Source  : dev_rotary_encoder_x.c
==============================================*/
