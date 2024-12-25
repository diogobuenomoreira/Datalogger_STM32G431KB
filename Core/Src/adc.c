/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */
#include "tim.h"
#include "app_fatfs.h"
#include <string.h>
#include "usart.h"

#define BUFFER_SIZE			(21504UL)
#define SAMPLE_SIZE			(2U)
#define BYTES_TO_WRITTEN	(8192U)
#define BLOCK_SIZE			(512U)

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
volatile uint16_t adc_value=0;
static volatile int adc_stop = 0;
static uint8_t fifo_buffer[BUFFER_SIZE];
static volatile uint32_t write_index = 0;
static volatile uint32_t read_index = 0;
static volatile uint32_t available_data = 0;

void __attribute__((optimize("O0")))HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	uint8_t  elapsed_time = 0;
	uint8_t sample[SAMPLE_SIZE];
	static uint32_t last_time = 0;

	if (hadc->Instance == ADC1)
	{
		uint32_t current_time = __HAL_TIM_GET_COUNTER(&htim2);

		// Calcular o tempo transcorrido desde a última interrupção (em µs)
		if (current_time >= last_time)
		{
			elapsed_time = current_time - last_time;  // Sem overflow
		}
		else
		{
			// Caso haja overflow do contador do timer
			elapsed_time = (0xFFFFFFFF - last_time) + current_time + 1;
		}
		last_time = current_time;
		sample[0] = (uint8_t)(adc_value & 0xFF);
		sample[1] = (uint8_t)(((adc_value >> 8) & 0x0F) | (elapsed_time << 4));
		fifo_buffer[write_index++] = sample[0];
		fifo_buffer[write_index++] = sample[1];
		write_index %= BUFFER_SIZE;
		available_data += SAMPLE_SIZE;

		if (available_data > BUFFER_SIZE)
		{
			available_data = BUFFER_SIZE;
			read_index = write_index;
		}

		if(Get_ADC_Stop_Flag() == 0)
		{
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_value, 1);
		}
    }
}
/* USER CODE END 0 */


/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_92CYCLES_5;//ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC1 clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN1
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void Set_ADC_Stop_Flag(const int status)
{
	adc_stop = status;
}

int Get_ADC_Stop_Flag(void)
{
	return adc_stop;
}

uint32_t Get_Available_Data(void)
{
	return available_data;
}

void Update_Read_Index(const uint32_t bytes_written)
{
	read_index += bytes_written;
	read_index %= BUFFER_SIZE;
}

void SD_Write_Task(void)
{
    UINT bytes_written;
    FIL* file = Get_File();
    uint8_t temp_buffer[BYTES_TO_WRITTEN];

    while (available_data >= BYTES_TO_WRITTEN)
    {
        if (read_index + BYTES_TO_WRITTEN <= BUFFER_SIZE)
        {
            memcpy(temp_buffer, &fifo_buffer[read_index], BYTES_TO_WRITTEN);
            read_index = (read_index + BYTES_TO_WRITTEN) % BUFFER_SIZE;
        }
        else
        {
            uint32_t first_chunk_size = BUFFER_SIZE - read_index;
            memcpy(temp_buffer, &fifo_buffer[read_index], first_chunk_size);
            memcpy(temp_buffer + first_chunk_size, fifo_buffer, BYTES_TO_WRITTEN - first_chunk_size);
            read_index = (read_index + BYTES_TO_WRITTEN) % BUFFER_SIZE;
        }
        uint8_t* buffer_pointer = temp_buffer;
        uint32_t bytes_to_write = BYTES_TO_WRITTEN;

        while (bytes_to_write > 0)
        {
            FRESULT res = f_write(file, buffer_pointer, BLOCK_SIZE, &bytes_written);
            if (res != FR_OK || bytes_written != BLOCK_SIZE)            {

                return;
            }
            buffer_pointer += BLOCK_SIZE;
            bytes_to_write -= BLOCK_SIZE;
        }
        available_data -= BYTES_TO_WRITTEN;
    }
}

/* USER CODE END 1 */
