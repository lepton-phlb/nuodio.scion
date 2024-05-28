/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2011. All Rights Reserved.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/
#ifndef _MIDI_SYSEX_H_
#define _MIDI_SYSEX_H_

/*===========================================
Declaration
=============================================*/

//#define USE_MIDI_SYSEX_STANDALONE

#ifdef USE_MIDI_SYSEX_STANDALONE
#define __midi_sysex_parser_debug_printf(__debug__,...) if(__debug__)printf(__VA_ARGS__)
#else
#define __midi_sysex_parser_debug_printf(__debug__,...)
#endif

//
#define MIDI_SYSEX_NUODIO_ID_MSB ((uint8_t)(0x3F))
#define MIDI_SYSEX_NUODIO_ID_LSB ((uint8_t)(0x6F))

//
#define MIDI_SYSEX_NUODIO_MODE_DEBUG   ((uint8_t)(0))
#define MIDI_SYSEX_NUODIO_MODE_CENTRAL ((uint8_t)(1))
#define MIDI_SYSEX_NUODIO_MODE_DEVICE  ((uint8_t)(2))

//
#define MIDI_SYSEX_NUODIO_INVALID_DEV_ID  ((uint8_t)(0x7F))
#define MIDI_SYSEX_NUODIO_CENTRAL_DEV_ID  ((uint8_t)(0x00))

//
#define SYSEX_PROTOCOL_DECODER_STATE_NULL                   ((uint8_t)(0))
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0                ((uint8_t)(1)) //midi status command SYSEX START
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_ID00              ((uint8_t)(2)) 
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_IDMSB             ((uint8_t)(3)) 
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_IDLSB             ((uint8_t)(4))
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMETYPE         ((uint8_t)(5))
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMELENGTH_MSB   ((uint8_t)(6))
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMELENGTH_LSB   ((uint8_t)(7))

//
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_DSTDEVID          ((uint8_t)(8))
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_SRCDEVID          ((uint8_t)(9))
//
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_RQSTCOUNTER       ((uint8_t)(10))
//
#define SYSEX_PROTOCOL_DECODER_STATE_WAIT_EOX               ((uint8_t)(11))//midi status command SYSEX END

typedef struct sysex_parser_context_st {
   uint8_t flag_debug;
   uint8_t mode;
   //
   uint8_t dev_id;
   //
   uint8_t protocol_decoder_state;
   //
   uint8_t frame_type;
   uint8_t frame_length;
   uint8_t src_dev_id;
   uint8_t dst_dev_id;
   uint8_t request_counter;
   //
   uint8_t* p_data_buffer;
   uint8_t data_buffer_counter;
   uint8_t data_buffer_sz_max;
   //
   uint8_t frame_crc[3];
   uint8_t frame_crc_counter;
   crc16_t crc;
   uint8_t crc_check_ok;

   //
   uint8_t error_code;
}sysex_parser_context_t;

//frame type
#define SYSEX_PROTOCOL_DECODER_FRAMETYPE_INVALID   ((uint8_t)(0xFF))
#define SYSEX_PROTOCOL_DECODER_FRAMETYPE_DISCOVER  ((uint8_t)(0x0D))
#define SYSEX_PROTOCOL_DECODER_FRAMETYPE_REQUEST   ((uint8_t)(0x11))
#define SYSEX_PROTOCOL_DECODER_FRAMETYPE_RESPONSE  ((uint8_t)(0x12))

//string data offset
#define SYSEX_PROTOCOL_FRAME_STRING_DATA_OFFSET  ((uint8_t)(10))

//
int sysex_protocol_construct_frame(uint8_t frame_type, uint8_t dst_id, char* str_ascii_data, uint8_t* p_midi_sysex_buffer, int midi_sysex_buffer_sz);
int sysex_protocol_decoder(sysex_parser_context_t* p_sysex_parser_context, uint8_t midi_byte);
int sysex_protocol_context_reset(sysex_parser_context_t* p_sysex_parser_context);
int sysex_protocol_context_init(sysex_parser_context_t* p_sysex_parser_context, uint8_t*  data_buffer, int data_buffer_size_max);
//
int sysex_protocol_context_set_device_id(sysex_parser_context_t* p_sysex_parser_context, uint8_t dev_id);
//
int sysex_protocol_decoder_tunnel(sysex_parser_context_t* p_sysex_parser_context, uint8_t*  midi_byte_output, uint8_t  midi_byte_input);

#endif
