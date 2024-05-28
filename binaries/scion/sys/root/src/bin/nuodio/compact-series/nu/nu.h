/* Copyright (c) 2017 nuodio. All Rights Reserved.
*
* The information contained herein is property of nuodio.
*
*
*/


/*============================================
| Compiler Directive
==============================================*/
#ifndef __NU_H__
#define __NU_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/

//midi nrpn
#define NU_MIDI_NRPN_PARAMETER_MSB_INVALID ((uint8_t)(0xff))
#define NU_MIDI_NRPN_PARAMETER_LSB_INVALID ((uint8_t)(0xff))

//
#define NU_MIDI_NRPN_PARAMETER_MSB_FIRST ((uint8_t)(1))
#define NU_MIDI_NRPN_PARAMETER_LSB_FIRST ((uint8_t)(0))
#define NU_MIDI_NRPN_PARAMETER_MSB_LAST  ((uint8_t)(NU_EFFECT_PARAMETERS_MAX))
#define NU_MIDI_NRPN_PARAMETER_LSB_LAST  ((uint8_t)(0))



//shared memory
typedef struct shared_memory_midi_st{
   uint8_t status;
   uint8_t channel;
   uint8_t length;
   uint8_t data1;
   uint8_t data2;
}shared_memory_midi_t;

#define SHARED_MEM_ADDRESS_SYSTEM         ((int)(0x00000010))
#define SHARED_MEM_ADDRESS_MIDI           ((int)(0x00000100))
   

//
typedef struct nu_xgui_st {
   //
   effect_parameter_t * effect_parameters_list;
   //
   
   //
   int fd_keybfootswitch;
   int fd_rotary_1;
   int fd_rotary_2;
   int fd_rotary_3;
   int fd_rotary_4;
   //
   int fd_mem;
   //
   uint8_t rotary_1_is_pressed;
   uint8_t rotary_2_is_pressed;
   uint8_t rotary_3_is_pressed;
   uint8_t rotary_4_is_pressed;
   //
   uint8_t rotary_1_is_long_pressed;
   uint8_t rotary_2_is_long_pressed;
   uint8_t rotary_3_is_long_pressed;
   uint8_t rotary_4_is_long_pressed;
   
   //
   time_t rotary_1_start_time_pressed;
   time_t rotary_2_start_time_pressed;
   time_t rotary_3_start_time_pressed;
   time_t rotary_4_start_time_pressed;
   //
   uint8_t rotary_1_is_turned;
   uint8_t rotary_2_is_turned;
   uint8_t rotary_3_is_turned;
   uint8_t rotary_4_is_turned;
   
   //
   int midi_preset_no;
   //
   int current_midi_preset_is_modified;
   int current_midi_preset_no;
   
   //midi
   shared_memory_midi_t shared_memory_midi;
   uint8_t midi_nrpn_msb_parameter;
   uint8_t midi_nrpn_lsb_parameter;
   uint8_t midi_nrpn_msb_data;
   uint8_t midi_nrpn_lsb_data;
   
   //
   nu_preset_t  nu_preset;
   ///
   nu_preset_storage_t nu_preset_storage;
} nu_xgui_t, *P_nu_xgui_t;

//
int nu_save_preset_parameter_value(nu_xgui_t *p_nu_xgui,int preset_no);
int nu_load_preset_parameter_value(nu_xgui_t *p_nu_xgui,int preset_no);


#endif
