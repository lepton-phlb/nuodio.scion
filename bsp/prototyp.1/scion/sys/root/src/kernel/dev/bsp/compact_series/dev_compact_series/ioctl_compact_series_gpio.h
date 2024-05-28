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
#ifndef __IOCTL_COMPACT_SERIES_GPIO_H__
#define __IOCTL_COMPACT_SERIES_GPIO_H__


/*============================================
| Includes
==============================================*/



/*============================================
| Declaration
==============================================*/
// true by pass
#define GPIOCMD_TRUEBYPASS             (uint32_t)(0x01)
// 20V analogic power
#define GPIOCMD_ANALOGICPOWER          (uint32_t)(0x02)
// MIDI MODE (out or thru)
#define GPIOCMD_MIDI_MODE              (uint32_t)(0x03)

//FXIO
#define GPIOCMD_FXIO_BASE              (0x10)
#define GPIOCMD_FXIO_0                 (uint32_t)(GPIOCMD_FXIO_BASE+0x00)
#define GPIOCMD_FXIO_1                 (uint32_t)(GPIOCMD_FXIO_BASE+0x01)
#define GPIOCMD_FXIO_2                 (uint32_t)(GPIOCMD_FXIO_BASE+0x02)
#define GPIOCMD_FXIO_3                 (uint32_t)(GPIOCMD_FXIO_BASE+0x03)
#define GPIOCMD_FXIO_4                 (uint32_t)(GPIOCMD_FXIO_BASE+0x04)
#define GPIOCMD_FXIO_5                 (uint32_t)(GPIOCMD_FXIO_BASE+0x05)
#define GPIOCMD_FXIO_6                 (uint32_t)(GPIOCMD_FXIO_BASE+0x06)
#define GPIOCMD_FXIO_7                 (uint32_t)(GPIOCMD_FXIO_BASE+0x07)


#define GPIOCMD_IN2FX2OUT              (uint32_t)(0x20)
#define GPIOCMD_IN2ADC_DAC2OUT         (uint32_t)(0x21)

#define  GPIOCMD_DETECT_JACK_INPUT     (uint32_t)(0x30)
#define  GPIOCMD_DETECT_JACK_OUTPUT    (uint32_t)(0x31)


#endif