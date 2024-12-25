/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   app_fatfs.c
  * @brief  Code for fatfs applications
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
#include "app_fatfs.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_INIT,
  APPLICATION_RUNNING,
  APPLICATION_SD_UNPLUGGED,
}FS_FileOperationsTypeDef;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BLOCK_SIZE		(512U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
FATFS USERFatFs;    /* File system object for USER logical drive */
FIL USERFile;       /* File  object for USER */
char USERPath[4];   /* USER logical drive path */
/* USER CODE BEGIN PV */
FS_FileOperationsTypeDef Appli_state = APPLICATION_IDLE;

static FATFS FatFS;    /* File system object for USER logical drive */
static FIL file;       /* File object for USER */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void SD_Card_Mount(void);
static void SD_Card_Unmount(void);
/* USER CODE END PFP */

/**
  * @brief  FatFs initialization
  * @param  None
  * @retval Initialization result
  */
int32_t MX_FATFS_Init(void)
{
  /*## FatFS: Link the disk I/O driver(s)  ###########################*/

if (FATFS_LinkDriver(&USER_Driver, USERPath) != 0)
  /* USER CODE BEGIN FATFS_Init */
  {
    return APP_ERROR;
  }
  else
  {
    Appli_state = APPLICATION_INIT;
    return APP_OK;
  }
  /* USER CODE END FATFS_Init */
}

/**
  * @brief  FatFs application main process
  * @param  None
  * @retval Process result
  */
int32_t MX_FATFS_Process(void)
{
  /* USER CODE BEGIN FATFS_Process */
  int32_t process_res = APP_OK;

  return process_res;
  /* USER CODE END FATFS_Process */
}

/**
  * @brief  Gets Time from RTC (generated when FS_NORTC==0; see ff.c)
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN Application */
static void SD_Card_Mount(void)
{
	FRESULT fres;

	fres = f_mount(&FatFS, "", 1);    //1=mount now
	if (fres != FR_OK)
	{
		DEBUG_PRINTF("No SD Card found : (%i)\r\n", fres);
	}
	else
	{
		DEBUG_MESSAGE("SD Card Mounted Successfully!!!\r\n");
	}
}

static void SD_Card_Unmount(void)
{
	FRESULT fres;

	fres = f_mount(NULL, "", 0);
	if (fres != FR_OK)
	{
		DEBUG_PRINTF("SD Card Unmounted Failed: (%i) \r\n", fres);
	}
	else
	{
		DEBUG_MESSAGE("SD Card Unmounted Successfully!!!\r\n");
	}
}

void SD_Card_Open(void)
{
    FRESULT fres;

    SD_Card_Mount();

    fres = f_open(&file, "datalogger.bin", FA_WRITE |FA_OPEN_APPEND|FA_CREATE_ALWAYS);
    if (fres != FR_OK)
    {
        DEBUG_PRINTF("File creation/open Error : (%i)\r\n", fres);
    }
}

void SD_Card_Close(void)
{
	FRESULT fres;

	fres = f_close(&file);
	if(fres != FR_OK)
	{
		DEBUG_PRINTF("File close Error : (%i)\r\n", fres);
	}
	SD_Card_Unmount();
}

void SD_Card_Write(const uint8_t* data, uint32_t length)
{
    FRESULT fres;
    UINT bytes_written;
    uint32_t offset = 0;


    while (length > 0)
    {
        uint32_t bytes_to_write = (length >= BLOCK_SIZE) ? BLOCK_SIZE : length;

        fres = f_write(&file, data + offset, bytes_to_write, &bytes_written);

        if((fres != FR_OK) || (bytes_written != bytes_to_write))
        {
           // DEBUG_PRINTF("File write Error : (%i)\r\n", fres);
            break;
        }
        else
        {
        	Update_Read_Index(bytes_to_write);
           // DEBUG_PRINTF("Written %i bytes \r\n", bytes_written);
        }
        length -= bytes_to_write;
        offset += bytes_to_write;
    }
    f_sync(&file);
}

FIL* Get_File(void)
{
	return &file;
}
/* USER CODE END Application */
