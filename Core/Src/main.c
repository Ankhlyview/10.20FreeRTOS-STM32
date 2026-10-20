/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "MPU6050.h"
#include "math.h"
#include "OLED.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/**
 * @brief 声明二进制信号量用于串口通信，队列用于MPU和OLED传递参数，数组用于接受传串口的信息
 *
 */
SemaphoreHandle_t xSemaphorUart;
QueueHandle_t xQueueMPU;
uint8_t receiveData[20];
static uint16_t size;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
void vTaskUart(void*pvParameters);
void vTaskOLED(void*pvParameters);
void vTaskMPU(void*pvParameters);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief 用接收端为空中断来给串口任务执行指令(二进制信号量)
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	size = Size;
	BaseType_t xWoke = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphorUart,&xWoke);
	portYIELD_FROM_ISR(xWoke);
//	HAL_UART_Transmit_DMA(&huart2,receiveData,Size);
//	HAL_UARTEx_ReceiveToIdle_DMA(&huart2,receiveData,sizeof(receiveData));
}
/**
 * @brief 串口任务
 */
void vTaskUart(void*pvParameters)
{
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2,receiveData,sizeof(receiveData));
	while(1)
	{
		xSemaphoreTake(xSemaphorUart,portMAX_DELAY);
		HAL_UART_Transmit_DMA(&huart2,receiveData,size);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2,receiveData,sizeof(receiveData));
	}
}
/**
 * @brief MPU读取和姿态结算任务，姿态解算用的是融合滤波，只给队列传入最新的结算结果
 */
void vTaskMPU(void*pvParameters)
{
	uint8_t buf[14];
	float Ax,Ay,Az,Gx,Gy,Gz;
	Deal_data_t data;
	data.pitch = 0.0;
	data.roll = 0.0;
	data.yaw = 0.0;
	xQueueOverwrite(xQueueMPU, &data);
	while(1){
		MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H, buf, 14);
		
		Ax = (int16_t)(buf[0] << 8 | buf[1])/8192.0f;
		Ay = (int16_t)(buf[2] << 8 | buf[3])/8192.0f;
		Az = (int16_t)(buf[4] << 8 | buf[5])/8192.0f;
		
		Gx = (int16_t)(buf[8]  << 8 | buf[9])/65.5f;
		Gy = (int16_t)(buf[10]  << 8 | buf[11])/65.5f;
		Gz = (int16_t)(buf[12]  << 8 | buf[13])/65.5f;
		
		vTaskDelay(pdMS_TO_TICKS(10));
		data.yaw = data.yaw + Gz*0.010;
		data.pitch = 0.95238*(data.pitch + Gx*0.010)+(1-0.95238)*atan2(Ay,Az)/3.1415927*180.0f;
		data.roll = 0.95238*(data.roll + Gy*0.010)+(1-0.95238)*atan2(Ax,Az)/3.1415927*180.0f;
		xQueueOverwrite(xQueueMPU, &data); 
	}
	
}

/**
 * @brief OLED显示任务，直接从队列里面读数
 */
void vTaskOLED(void *pvParameters)
{
    Deal_data_t data; 
    OLED_ShowString(2, 1, "roll:");
    OLED_ShowString(3, 1, "yaw:");
    OLED_ShowString(4, 1, "pitch:");
    for (;;)
    {
        if (xQueueReceive(xQueueMPU, &data, portMAX_DELAY) == pdTRUE)
        {
					OLED_ShowFloatNum(2, 8, data.roll,  2);
					OLED_ShowFloatNum(3, 8, data.yaw,   2);
					OLED_ShowFloatNum(4, 8, data.pitch, 2);
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	xSemaphorUart = xSemaphoreCreateBinary();
	xQueueMPU = xQueueCreate(1,sizeof(Deal_data_t));
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
	
	/// 初始化OLED和MPU6050，并在OLED上显示MPu的初始化情况
	OLED_Init();
	OLED_Clear();
	if(MPU6050_Init() != 0)
	{
		OLED_ShowString(1,1,"MPU6050 Failed");
		Error_Handler();
	}
	OLED_ShowString(1,1,"MPU6050 Success");
	
	/// 创建任务，并且分配合适的优先级
	xTaskCreate(vTaskUart,"UART",128,NULL,3,NULL);
	xTaskCreate(vTaskMPU,"MPU",256,NULL,3,NULL);
	xTaskCreate(vTaskOLED,"OLED",128,NULL,2,NULL);
	
	/// 启动任务调度器
	vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
