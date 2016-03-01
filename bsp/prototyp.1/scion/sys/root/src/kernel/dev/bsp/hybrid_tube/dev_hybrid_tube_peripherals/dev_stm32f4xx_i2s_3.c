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
#include "kernel/core/malloc.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_spi.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfsdev.h"
#include "kernel/core/kernel_ring_buffer.h"

#include "kernel/dev/bsp/hybrid_tube/cubemx_hal/gpio.h"

//#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/dev_stm32f4xx_i2s_3.h"

/*============================================
| Global Declaration
==============================================*/

#include "stm32f4xx_hal.h"

#define SAMPLES_PER_BUFFER 48//((48)*2) //32

#pragma data_alignment=4 
static int32_t RxBuffer[SAMPLES_PER_BUFFER * 2]={0};//RX is receive of input audio from codec
#pragma data_alignment=4 
static int32_t TxBuffer[SAMPLES_PER_BUFFER * 2]={0};//TX is transmit of output audio to codec
//#pragma data_alignment=4 
//static int32_t WorkingBuffer[SAMPLES_PER_BUFFER * 2]={0};


typedef struct stm32f4xx_i2s_3_info_st{
  
 desc_t desc_r;
 desc_t desc_w; 
 I2S_HandleTypeDef hi2s3;
 DMA_HandleTypeDef hdma_i2s3_ext_rx;
 DMA_HandleTypeDef hdma_spi3_tx;
 
}stm32f4xx_i2s_3_info_t;

#pragma data_alignment=4 
static stm32f4xx_i2s_3_info_t g_stm32f4xx_i2s_3_info;

const char dev_stm32f4xx_i2s_3_name[]="i2s3\0";

int dev_stm32f4xx_i2s_3_load(void);
int dev_stm32f4xx_i2s_3_open(desc_t desc, int o_flag);
int dev_stm32f4xx_i2s_3_close(desc_t desc);
int dev_stm32f4xx_i2s_3_isset_read(desc_t desc);
int dev_stm32f4xx_i2s_3_isset_write(desc_t desc);
int dev_stm32f4xx_i2s_3_read(desc_t desc, char* buf,int size);
int dev_stm32f4xx_i2s_3_write(desc_t desc, const char* buf,int size);
int dev_stm32f4xx_i2s_3_seek(desc_t desc,int offset,int origin);
int dev_stm32f4xx_i2s_3_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_stm32f4xx_i2s_3_map={
   dev_stm32f4xx_i2s_3_name,
   S_IFCHR,
   dev_stm32f4xx_i2s_3_load,
   dev_stm32f4xx_i2s_3_open,
   dev_stm32f4xx_i2s_3_close,
   dev_stm32f4xx_i2s_3_isset_read,
   dev_stm32f4xx_i2s_3_isset_write,
   dev_stm32f4xx_i2s_3_read,
   dev_stm32f4xx_i2s_3_write,
   dev_stm32f4xx_i2s_3_seek,
   dev_stm32f4xx_i2s_3_ioctl //ioctl
};

//
#define I2S_AUDIO_CHANNEL_MAX           2

#define I2S_AUDIO_CHANNEL_ADC_INPUT     0
#define I2S_AUDIO_CHANNEL_ADC_INPUT_SZ  ((SAMPLES_PER_BUFFER*4)*sizeof(int32_t))

#define I2S_AUDIO_CHANNEL_DAC_OUTPUT    1
#define I2S_AUDIO_CHANNEL_DAC_OUTPUT_SZ ((SAMPLES_PER_BUFFER*4)*sizeof(int32_t))

#pragma data_alignment=4
static uint8_t i2s_audio_channel_adc_input_buffer[I2S_AUDIO_CHANNEL_ADC_INPUT_SZ]={0};

#pragma data_alignment=4
static uint8_t i2s_audio_channel_dac_output_buffer[I2S_AUDIO_CHANNEL_DAC_OUTPUT_SZ]={0};


static kernel_ring_buffer_t krb_i2s_audio_channel_adc_input;
static kernel_ring_buffer_t krb_i2s_audio_channel_dac_output;


//
void I2S2_RX_ProcessBuffer(uint32_t posbegin, uint32_t posend){
   //
   if(__kernel_ring_buffer_is_empty(krb_i2s_audio_channel_adc_input)){
      __fire_io_int(ofile_lst[g_stm32f4xx_i2s_3_info.desc_r].owner_pthread_ptr_read);
   }
   //write data from adc to ring buffer
   kernel_ring_buffer_write(&krb_i2s_audio_channel_adc_input,(uint8_t*)&RxBuffer[posbegin],(posend-posbegin)*sizeof(int32_t));
                            
   //read data from ring buffer to dac
   kernel_ring_buffer_read(&krb_i2s_audio_channel_dac_output,(uint8_t*)&TxBuffer[posbegin],(posend-posbegin)*sizeof(int32_t));
                            
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s){
   //refill buffer HALF - END
   if(hi2s->Instance == SPI3){
      I2S2_RX_ProcessBuffer(SAMPLES_PER_BUFFER,SAMPLES_PER_BUFFER*2);
   }
 
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
   //refill buffer BEGIN - HALF
   if(hi2s->Instance == SPI3){
      I2S2_RX_ProcessBuffer(0,SAMPLES_PER_BUFFER);
   }
 
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
  uint32_t status = hi2s->Instance->SR;
  if(status & 0x40) {
      __HAL_I2S_CLEAR_OVRFLAG(hi2s);
  }
  if(status & 0x08) {
      __HAL_I2S_CLEAR_UDRFLAG(hi2s);
  }
   if(status & 0x100){
     __HAL_I2S_DISABLE(hi2s);
    __HAL_I2S_ENABLE(hi2s);
  }
}

/**
* @brief This function handles DMA1 Stream0 global interrupt.
*/
void DMA1_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream0_IRQn 0 */
  __hw_enter_interrupt();
  /* USER CODE END DMA1_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&g_stm32f4xx_i2s_3_info.hdma_i2s3_ext_rx);
  /* USER CODE BEGIN DMA1_Stream0_IRQn 1 */
  __hw_leave_interrupt();
  /* USER CODE END DMA1_Stream0_IRQn 1 */
}

/**
* @brief This function handles DMA1 Stream5 global interrupt.
*/
void DMA1_Stream5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */
  __hw_enter_interrupt();
  /* USER CODE END DMA1_Stream5_IRQn 0 */
  HAL_DMA_IRQHandler(&g_stm32f4xx_i2s_3_info.hdma_spi3_tx);
  /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */
   __hw_leave_interrupt();
  /* USER CODE END DMA1_Stream5_IRQn 1 */
}

void SPI3_IRQHandler(void) {
//  uint16_t t = I2SxEXT(hi2s2.Instance)->DR;
//  uint16_t tmpreg1 = I2SxEXT(hi2s2.Instance)->SR;
//  if(hi2s2.Lock != HAL_LOCKED) {
    HAL_I2S_IRQHandler(&g_stm32f4xx_i2s_3_info.hi2s3);
 
//  } //else {
//      __HAL_I2S_DISABLE(&hi2s2);
//      __HAL_I2S_CLEAR_OVRFLAG(&hi2s2);
//      __HAL_I2S_CLEAR_UDRFLAG(&hi2s2);
//      while(HAL_GPIO_ReadPin(GPIOB,1<<12) != GPIO_PIN_RESET);
//      while(HAL_GPIO_ReadPin(GPIOB,1<<12) != GPIO_PIN_SET);
//      __HAL_I2S_ENABLE(&hi2s2);
//  }
}

/*============================================
| Implementation
==============================================*/

/*-------------------------------------------
| Name:WaitForWCLK 
| Description:WS (stm32f4 I2S salve mode issue. Workaround).
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
// WS waiting function:
static uint8_t WaitForWCLK(uint8_t edge)
{
   uint32_t TOTmr1ms = 1;  // TOTmr1ms is decremented in SysTick interrupt
   if(edge>0){ // wait for WCLK=1
        while (0==(GPIOA->IDR & GPIO_PIN_15)){
            if(TOTmr1ms==0)
                return 0;
        }
        return 1;
   }else{    // wait for WCLK=0
        while (0!=(GPIOA->IDR & GPIO_PIN_15)){
            if(TOTmr1ms==0)
                return 0;
        }
        return 1;
 
   }
}

/**
  * @brief Full-Duplex Transmit/Receive data in non-blocking mode using DMA  
  * @param  hi2s: pointer to a I2S_HandleTypeDef structure that contains
  *         the configuration information for I2S module
  * @param pTxData: a 16-bit pointer to the Transmit data buffer.
  * @param pRxData: a 16-bit pointer to the Receive data buffer.
  * @param Size: number of data sample to be sent:
  * @note When a 16-bit data frame or a 16-bit data frame extended is selected during the I2S
  *       configuration phase, the Size parameter means the number of 16-bit data length 
  *       in the transaction and when a 24-bit data frame or a 32-bit data frame is selected 
  *       the Size parameter means the number of 16-bit data length. 
  * @note The I2S is kept enabled at the end of transaction to avoid the clock de-synchronization 
  *       between Master and Slave(example: audio streaming).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_I2SEx_TransmitReceive_DMA_Workaround(I2S_HandleTypeDef *hi2s, uint16_t *pTxData, uint16_t *pRxData, uint16_t Size)
{
  uint32_t *tmp;
  uint32_t tmp1 = 0, tmp2 = 0;
    
  if((pTxData == NULL ) || (pRxData == NULL ) || (Size == 0)) 
  {
    return  HAL_ERROR;
  }

  if(hi2s->State == HAL_I2S_STATE_READY)
  {
    hi2s->pTxBuffPtr = pTxData;
    hi2s->pRxBuffPtr = pRxData;

    tmp1 = hi2s->Instance->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);
    tmp2 = hi2s->Instance->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);
    /* Check the Data format: When a 16-bit data frame or a 16-bit data frame extended 
       is selected during the I2S configuration phase, the Size parameter means the number
       of 16-bit data length in the transaction and when a 24-bit data frame or a 32-bit data 
       frame is selected the Size parameter means the number of 16-bit data length. */
    if((tmp1 == I2S_DATAFORMAT_24B)||\
       (tmp2 == I2S_DATAFORMAT_32B))
    {
      hi2s->TxXferSize = Size*2;
      hi2s->TxXferCount = Size*2;
      hi2s->RxXferSize = Size*2;
      hi2s->RxXferCount = Size*2;
    }
    else
    {
      hi2s->TxXferSize = Size;
      hi2s->TxXferCount = Size;
      hi2s->RxXferSize = Size;
      hi2s->RxXferCount = Size;
    }

    /* Process Locked */
    __HAL_LOCK(hi2s);

    hi2s->State = HAL_I2S_STATE_BUSY_TX_RX;
    hi2s->ErrorCode = HAL_I2S_ERROR_NONE;

    /* Set the I2S Rx DMA Half transfer complete callback */
    hi2s->hdmarx->XferHalfCpltCallback = I2S_DMARxHalfCplt;

    /* Set the I2S Rx DMA transfer complete callback */
    hi2s->hdmarx->XferCpltCallback = I2S_DMARxCplt;

    /* Set the I2S Rx DMA error callback */
    hi2s->hdmarx->XferErrorCallback = I2S_DMAError;

    /* Set the I2S Tx DMA Half transfer complete callback */
    hi2s->hdmatx->XferHalfCpltCallback = I2S_DMATxHalfCplt;

    /* Set the I2S Tx DMA transfer complete callback */
    hi2s->hdmatx->XferCpltCallback = I2S_DMATxCplt;

    /* Set the I2S Tx DMA error callback */
    hi2s->hdmatx->XferErrorCallback = I2S_DMAError;

    tmp1 = hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SCFG;
    tmp2 = hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SCFG;
    /* Check if the I2S_MODE_MASTER_TX or I2S_MODE_SLAVE_TX Mode is selected */
    if((tmp1 == I2S_MODE_MASTER_TX) || (tmp2 == I2S_MODE_SLAVE_TX))
    {
      /* Enable the Rx DMA Stream */
      tmp = (uint32_t*)&pRxData;
      HAL_DMA_Start_IT(hi2s->hdmarx, (uint32_t)&I2SxEXT(hi2s->Instance)->DR, *(uint32_t*)tmp, hi2s->RxXferSize);

      /* Enable Rx DMA Request */  
      I2SxEXT(hi2s->Instance)->CR2 |= SPI_CR2_RXDMAEN;

      /* Enable the Tx DMA Stream */
      tmp = (uint32_t*)&pTxData;
      HAL_DMA_Start_IT(hi2s->hdmatx, *(uint32_t*)tmp, (uint32_t)&hi2s->Instance->DR, hi2s->TxXferSize);

      /* Enable Tx DMA Request */  
      hi2s->Instance->CR2 |= SPI_CR2_TXDMAEN;

      /* Check if the I2S is already enabled */ 
      if((hi2s->Instance->I2SCFGR &SPI_I2SCFGR_I2SE) != SPI_I2SCFGR_I2SE)
      {
          //
           //WS issue workaround: to do move in HAL_I2SEx_TransmitReceive_DMA() 
           __disable_interrupt_section_in();
           //WaitForWCLK(0);
           while (0!=(GPIOA->IDR & GPIO_PIN_15));
           //WaitForWCLK(1);
           while (0==(GPIOA->IDR & GPIO_PIN_15));
          /* Enable I2Sext(receiver) before enabling I2Sx peripheral */
          I2SxEXT(hi2s->Instance)->I2SCFGR |= SPI_I2SCFGR_I2SE;
          /* Enable I2S peripheral after the I2Sext */
         __HAL_I2S_ENABLE(hi2s);
         //
         __disable_interrupt_section_out();
      }
    }
    else
    {    
      /* Enable the Tx DMA Stream */
      tmp = (uint32_t*)&pTxData;
      HAL_DMA_Start_IT(hi2s->hdmatx, *(uint32_t*)tmp, (uint32_t)&I2SxEXT(hi2s->Instance)->DR, hi2s->TxXferSize);

      /* Enable Tx DMA Request */  
      I2SxEXT(hi2s->Instance)->CR2 |= SPI_CR2_TXDMAEN;

      /* Enable the Rx DMA Stream */
      tmp = (uint32_t*)&pRxData;
      HAL_DMA_Start_IT(hi2s->hdmarx, (uint32_t)&hi2s->Instance->DR, *(uint32_t*)tmp, hi2s->RxXferSize);

      /* Enable Rx DMA Request */  
      hi2s->Instance->CR2 |= SPI_CR2_RXDMAEN;

      /* Check if the I2S is already enabled */ 
      if((hi2s->Instance->I2SCFGR &SPI_I2SCFGR_I2SE) != SPI_I2SCFGR_I2SE)
      {
          //WS issue workaround: to do move in HAL_I2SEx_TransmitReceive_DMA() 
          __disable_interrupt_section_in();
          WaitForWCLK(0);
          WaitForWCLK(1);
         /* Enable I2S peripheral after the I2Sext */
         __HAL_I2S_ENABLE(hi2s);
         //
         __disable_interrupt_section_out();

        /* Enable I2Sext(transmitter) after enabling I2Sx peripheral */
        I2SxEXT(hi2s->Instance)->I2SCFGR |= SPI_I2SCFGR_I2SE;
      }
      else
      {
        /* Check if Master Receiver mode is selected */
        if((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SCFG) == I2S_MODE_MASTER_RX)
        {
          /* Clear the Overrun Flag by a read operation on the SPI_DR register followed by a read
          access to the SPI_SR register. */ 
          __HAL_I2S_CLEAR_OVRFLAG(hi2s);
        }
      }
    }

    /* Process Unlocked */
    __HAL_UNLOCK(hi2s);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/*-------------------------------------------
| Name:MX_I2S3_Init
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static void MX_I2S3_Init(void)
{ 
  //
  g_stm32f4xx_i2s_3_info.hi2s3.Instance = SPI3;
  g_stm32f4xx_i2s_3_info.hi2s3.Init.Mode = I2S_MODE_SLAVE_TX;
  g_stm32f4xx_i2s_3_info.hi2s3.Init.Standard = I2S_STANDARD_PHILLIPS;
  g_stm32f4xx_i2s_3_info.hi2s3.Init.DataFormat = I2S_DATAFORMAT_24B;//I2S_DATAFORMAT_32B;
  g_stm32f4xx_i2s_3_info.hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  g_stm32f4xx_i2s_3_info.hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  g_stm32f4xx_i2s_3_info.hi2s3.Init.CPOL = I2S_CPOL_LOW;
  g_stm32f4xx_i2s_3_info.hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  g_stm32f4xx_i2s_3_info.hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_ENABLE;
  HAL_I2S_Init(&g_stm32f4xx_i2s_3_info.hi2s3);

}

/*-------------------------------------------
| Name:HAL_I2S3_MspInit
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static void HAL_I2S3_MspInit(stm32f4xx_i2s_3_info_t* stm32f4xx_i2s_3_info)
{  
   GPIO_InitTypeDef GPIO_InitStruct;

   /* USER CODE BEGIN SPI3_MspInit 0 */

   /* USER CODE END SPI3_MspInit 0 */
   /* Peripheral clock enable */
   __SPI3_CLK_ENABLE();

   /**I2S3 GPIO Configuration    
   PC7     ------> I2S3_MCK
   PA15     ------> I2S3_WS
   PD6     ------> I2S3_SD
   PB3     ------> I2S3_CK
   PB4     ------> I2S3_ext_SD 
   */
   /*
   GPIO_InitStruct.Pin = GPIO_PIN_7;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
   */
   
   GPIO_InitStruct.Pin = GPIO_PIN_15;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = GPIO_PIN_6;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
   HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = GPIO_PIN_3;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = GPIO_PIN_4;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF7_I2S3ext;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   /* Peripheral DMA init*/

   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Instance = DMA1_Stream0;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.Channel = DMA_CHANNEL_3;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.PeriphInc = DMA_PINC_DISABLE;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.MemInc = DMA_MINC_ENABLE;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.Mode = DMA_CIRCULAR;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.MemBurst = DMA_MBURST_SINGLE;//DMA_MBURST_INC4;
   stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;//DMA_PBURST_INC4;
   HAL_DMA_Init(&stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx);

   __HAL_LINKDMA(&stm32f4xx_i2s_3_info->hi2s3,hdmarx,stm32f4xx_i2s_3_info->hdma_i2s3_ext_rx);

   stm32f4xx_i2s_3_info->hdma_spi3_tx.Instance = DMA1_Stream5;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.Channel = DMA_CHANNEL_0;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.Mode = DMA_CIRCULAR;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.MemBurst = DMA_MBURST_SINGLE;//DMA_MBURST_INC4;
   stm32f4xx_i2s_3_info->hdma_spi3_tx.Init.PeriphBurst =  DMA_PBURST_SINGLE;//DMA_PBURST_INC4;
   HAL_DMA_Init(&stm32f4xx_i2s_3_info->hdma_spi3_tx);

   __HAL_LINKDMA(&stm32f4xx_i2s_3_info->hi2s3,hdmatx,stm32f4xx_i2s_3_info->hdma_spi3_tx);
}

/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_i2s_3_load(){
   //
   g_stm32f4xx_i2s_3_info.desc_r = INVALID_DESC;
   g_stm32f4xx_i2s_3_info.desc_w = INVALID_DESC;
   //
   //MX_I2S3_Init();
   //
   kernel_ring_buffer_init(&krb_i2s_audio_channel_adc_input,i2s_audio_channel_adc_input_buffer,I2S_AUDIO_CHANNEL_ADC_INPUT_SZ);
   kernel_ring_buffer_init(&krb_i2s_audio_channel_dac_output,i2s_audio_channel_dac_output_buffer,I2S_AUDIO_CHANNEL_DAC_OUTPUT_SZ);
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_i2s_3_open(desc_t desc, int o_flag){

   //
   if(g_stm32f4xx_i2s_3_info.desc_r<0 && g_stm32f4xx_i2s_3_info.desc_w<0) {
      //
      HAL_StatusTypeDef status;
      /* DMA controller clock enable */
      __DMA1_CLK_ENABLE();
      
      //
      //
      HAL_I2S3_MspInit(&g_stm32f4xx_i2s_3_info);
      MX_I2S3_Init();
      //
      __HAL_I2S_DISABLE(&g_stm32f4xx_i2s_3_info.hi2s3);
      
      g_stm32f4xx_i2s_3_info.hdma_spi3_tx.Init.Channel = DMA_CHANNEL_0;
      HAL_DMA_Init(&g_stm32f4xx_i2s_3_info.hdma_spi3_tx);
     __HAL_LINKDMA(&g_stm32f4xx_i2s_3_info.hi2s3,hdmatx,g_stm32f4xx_i2s_3_info.hdma_spi3_tx);
      
      /* DMA interrupt init */
      //HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
      NVIC_SetPriority(DMA1_Stream0_IRQn, (1 << __NVIC_PRIO_BITS) -3);
      HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
      //
      //HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
      NVIC_SetPriority(DMA1_Stream5_IRQn, (1 << __NVIC_PRIO_BITS) -3);
      HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
      //
      __HAL_I2S_ENABLE_IT(&g_stm32f4xx_i2s_3_info.hi2s3, I2S_IT_TXE);
      __HAL_I2S_ENABLE_IT(&g_stm32f4xx_i2s_3_info.hi2s3, I2S_IT_RXNE);
      __HAL_I2S_ENABLE_IT(&g_stm32f4xx_i2s_3_info.hi2s3, I2S_IT_ERR);
      //
      status = HAL_I2SEx_TransmitReceive_DMA_Workaround(&g_stm32f4xx_i2s_3_info.hi2s3, (uint16_t*)&TxBuffer[0], (uint16_t*)&RxBuffer[0], SAMPLES_PER_BUFFER * 2);
      //
   }
  //
   if(o_flag & O_RDONLY) {
      if(g_stm32f4xx_i2s_3_info.desc_r<0) {
         g_stm32f4xx_i2s_3_info.desc_r = desc;
      }
      else
         return -1;                //already open
   }

   if(o_flag & O_WRONLY) {
      if(g_stm32f4xx_i2s_3_info.desc_w<0) {
         g_stm32f4xx_i2s_3_info.desc_w = desc;
      }
      else
         return -1;                //already open
   }
   //
   if(!ofile_lst[desc].p)
      ofile_lst[desc].p=&g_stm32f4xx_i2s_3_info;
   //
   return 0;

}

/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_i2s_3_close(desc_t desc){

   return 0;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_i2s_3_isset_read(desc_t desc){
   if(__kernel_ring_buffer_is_not_empty(krb_i2s_audio_channel_adc_input)){
      return 0;
   }
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_i2s_3_isset_write(desc_t desc){
   //if(audio_ring_buffer_channel[I2S_AUDIO_CHANNEL_DAC_OUTPUT].r==audio_ring_buffer_channel[I2S_AUDIO_CHANNEL_DAC_OUTPUT].w){
   //return 0;
   //}
   //return -1;
   return 0;
}
/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_i2s_3_read(desc_t desc, char* buf,int size){
  return kernel_ring_buffer_read(&krb_i2s_audio_channel_adc_input,(uint8_t*)buf,size);  
}

/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_i2s_3_write(desc_t desc, const char* buf,int size){
   int cb = kernel_ring_buffer_write(&krb_i2s_audio_channel_dac_output,(uint8_t*)buf,size);
   if(cb>0){
      __fire_io_int(ofile_lst[g_stm32f4xx_i2s_3_info.desc_w].owner_pthread_ptr_write);
   }
   return cb;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_i2s_3_seek(desc_t desc,int offset,int origin){
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_i2s_3_ioctl
| Description: ex: mount /dev/stm32f4xx_i2s_3 /dev/spi0 /dev/g_in 0 e.15 f.2 f.5 (a0, cs, shdn, pr)
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/

int dev_stm32f4xx_i2s_3_ioctl(desc_t desc,int request,va_list ap){
   switch(request) {
     
      case I_LINK: {
      }
      break;

      //
      case I_UNLINK: {
      }
      break;
      
      //
      default:
         return -1;

   }
   //
   return 0;
}


/*============================================
| End of Source  : dev_stm32f4xx_i2s_3.c
==============================================*/
