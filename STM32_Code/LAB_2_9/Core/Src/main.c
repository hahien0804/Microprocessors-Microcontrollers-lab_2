/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal_tim.h" // Added to resolve TIM_HandleTypeDef and HAL_TIM functions

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
// Đồng hồ 4 LED 7 đoạn
int hour = 15;
int minute = 8;
int second = 50;

const int MAX_LED = 4;
int index_led = 0;
int led_buffer[4] = {1, 5, 0, 8};

// LED matrix 8x8: quét theo cột
const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;
uint8_t matrix_buffer[8] = {0};

// Chữ A dạng 5 cột, mỗi bit biểu diễn một hàng.
// PB8 tương ứng bit 0, PB15 tương ứng bit 7.
const uint8_t letterA[5] = {
    0x7E, 0x11, 0x11, 0x11, 0x7E
};

// Vị trí cột bên trái của chữ A.
// Bắt đầu ngoài mép phải, sau đó dịch dần sang trái.
int animation_position = 8;
int animation_counter = 0;
const int ANIMATION_INTERVAL = 150; // 150 ms nếu TIM2 ngắt mỗi 1 ms
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Hiển thị một số trên LED 7 đoạn
void display7SEG(int num)
{
    const uint8_t segmentMap[10] = {
        0x40, 0x79, 0x24, 0x30, 0x19,
        0x12, 0x02, 0x78, 0x00, 0x10
    };

    if (num < 0 || num > 9) return;

    uint8_t val = segmentMap[num];

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (val >> 0) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (val >> 1) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, (val >> 2) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, (val >> 3) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (val >> 4) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (val >> 5) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (val >> 6) & 0x01);
}

// Quét 4 LED 7 đoạn
void update7SEG(int index)
{
    // Tắt tất cả chữ số trước khi đổi dữ liệu segment
    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
        GPIO_PIN_SET
    );

    switch (index)
    {
        case 0:
            display7SEG(led_buffer[0]);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
            break;
        case 1:
            display7SEG(led_buffer[1]);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
            break;
        case 2:
            display7SEG(led_buffer[2]);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
            break;
        case 3:
            display7SEG(led_buffer[3]);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

// Cập nhật dữ liệu thời gian cho LED 7 đoạn
void updateClockBuffer(void)
{
    led_buffer[0] = hour / 10;
    led_buffer[1] = hour % 10;
    led_buffer[2] = minute / 10;
    led_buffer[3] = minute % 10;
}

/*
 * Quét LED matrix theo cột.
 * PA1, PA2, PA10-PA15 chọn cột qua ULN2803.
 * PB8-PB15 xuất dữ liệu hàng.
 */
void updateLEDMatrix(int index)
{
    // Tắt tất cả các cột trước khi đổi dữ liệu hàng
    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_1 | GPIO_PIN_2 |
        GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
        GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
        GPIO_PIN_RESET
    );

    uint8_t column_data = matrix_buffer[index];

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8,  (column_data >> 0) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9,  (column_data >> 1) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (column_data >> 2) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, (column_data >> 3) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (column_data >> 4) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, (column_data >> 5) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (column_data >> 6) & 0x01);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (column_data >> 7) & 0x01);

    // Chọn cột hiện tại
    switch (index)
    {
        case 0: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,  GPIO_PIN_SET); break;
        case 1: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2,  GPIO_PIN_SET); break;
        case 2: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET); break;
        case 3: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET); break;
        case 4: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET); break;
        case 5: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET); break;
        case 6: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_SET); break;
        case 7: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET); break;
        default: break;
    }
}

// Tạo khung hình animation: chữ A dịch từ phải sang trái
void updateMatrixAnimation(void)
{
    // Xóa toàn bộ 8 cột
    for (int i = 0; i < MAX_LED_MATRIX; i++)
    {
        matrix_buffer[i] = 0x00;
    }

    // Vẽ 5 cột của chữ A ở vị trí hiện tại
    for (int i = 0; i < 5; i++)
    {
        int column = animation_position + i;

        if (column >= 0 && column < MAX_LED_MATRIX)
        {
            matrix_buffer[column] = letterA[i];
        }
    }

    // Dịch chữ A sang trái một cột mỗi lần cập nhật
    animation_position--;

    // Khi chữ A đã rời khỏi cạnh trái, bắt đầu lại từ bên phải
    if (animation_position < -5)
    {
        animation_position = 8;
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    // Hiển thị thời gian ban đầu
    updateClockBuffer();

    // Bắt đầu ngắt TIM2: quét LED 7 đoạn + matrix + animation
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    while (1)
    {
        HAL_Delay(1000);

        second++;

        if (second >= 60)
        {
            second = 0;
            minute++;
        }

        if (minute >= 60)
        {
            minute = 0;
            hour++;
        }

        if (hour >= 24)
        {
            hour = 0;
        }

        updateClockBuffer();

        // Chớp nháy DOT và D1 mỗi giây
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief TIM2 Initialization Function
  * @retval None
  */
static void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7999; // 8 MHz / (7999 + 1) = 1 kHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 9;         // 1 kHz update event = every 1 ms (fixed period from 0 to 9)
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Khởi tạo mức ban đầu: tắt cột matrix và DOT/D1
    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 |
        GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
        GPIO_PIN_14 | GPIO_PIN_15,
        GPIO_PIN_RESET
    );

    // Tắt các digit LED 7 đoạn (active-low)
    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
        GPIO_PIN_SET
    );

    // Khởi tạo các đường dữ liệu LED 7 đoạn và matrix
    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
        GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
        GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
        GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
        GPIO_PIN_RESET
    );

    // GPIOA outputs: matrix column select, DOT/D1, digit select
    GPIO_InitStruct.Pin =
        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 |
        GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
        GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
        GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // GPIOB outputs: seven segments and matrix row data
    GPIO_InitStruct.Pin =
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
        GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
        GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
        GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // Quét LED 7 đoạn mỗi 1 ms
        update7SEG(index_led);
        index_led++;

        if (index_led >= MAX_LED)
        {
            index_led = 0;
        }

        // Quét một cột matrix mỗi 1 ms
        updateLEDMatrix(index_led_matrix);
        index_led_matrix++;

        if (index_led_matrix >= MAX_LED_MATRIX)
        {
            index_led_matrix = 0;
        }

        // Cập nhật vị trí chữ A mỗi 150 ms, không dùng HAL_Delay trong interrupt
        animation_counter++;

        if (animation_counter >= ANIMATION_INTERVAL)
        {
            animation_counter = 0;
            updateMatrixAnimation();
        }
    }
}

/* USER CODE END 4 */

/**
  * @brief This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* Uses_FULL_ASSERT */
