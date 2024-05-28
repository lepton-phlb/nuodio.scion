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
| Compiler Directive
==============================================*/
#ifndef __DEV_ROTARY_ENCODER_X_H__
#define __DEV_ROTARY_ENCODER_X_H__


/*============================================
| Includes
==============================================*/



/*============================================
| Declaration
==============================================*/

typedef struct rotary_encoder_gpio_st{
  //stm32f4  gpio
  GPIO_TypeDef*   gpio_bank_no;
  uint32_t        gpio_pin_no;
}rotary_encoder_gpio_t;

typedef struct exti_src_gpio_st{
  //stm32f4 src gpio
  uint8_t  src_gpio_bank_no;
  uint8_t  src_gpio_pin_no;
}exti_src_gpio_t;


typedef struct rotary_encoder_info_st{
   //filled by user
   rotary_encoder_gpio_t pin_a; //signal a
   rotary_encoder_gpio_t pin_b; //signal b

   rotary_encoder_gpio_t pin_s;   //swicth s

   //timer encoder mode
   TIM_TypeDef* timer_no;  //timer TIM2
   u8 timer_gpio_af; //GPIO_AF_TIM2

   //external interrupt mode
   exti_src_gpio_t exti_pin; //interrupt pin

   //reserved: filled by system
   desc_t desc_r;
   uint32_t EXTI_Line;
   uint8_t input_r;
   uint8_t input_w;
   //
   int16_t counter;
   int16_t counter_limit_min;
   int16_t counter_limit_max;
   int16_t counter_step;

}rotary_encoder_info_t;

extern void dev_rotary_encoder_x_interrupt(rotary_encoder_info_t* rotary_encoder_info);

extern int dev_rotary_encoder_x_load(rotary_encoder_info_t* rotary_encoder_info);
extern int dev_rotary_encoder_x_open(desc_t desc, int o_flag, rotary_encoder_info_t* rotary_encoder_info);
extern int dev_rotary_encoder_x_close(desc_t desc);
extern int dev_rotary_encoder_x_isset_read(desc_t desc);
extern int dev_rotary_encoder_x_isset_write(desc_t desc);
extern int dev_rotary_encoder_x_read(desc_t desc, char* buf,int size);
extern int dev_rotary_encoder_x_write(desc_t desc, const char* buf,int size);
extern int dev_rotary_encoder_x_seek(desc_t desc,int offset,int origin);
extern int dev_rotary_encoder_x_ioctl(desc_t desc,int request,va_list ap);

#endif