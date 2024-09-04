/*------------------------------------------------------------------------------
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates).
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   1.Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   2.Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   3.Neither the name of Arm nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *------------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------*/

#include "cmsis_os2.h"                  // ARM::CMSIS:RTOS:Keil RTX5
#include "Board_LED.h"                  // Board Support:LED
#include "Board_Buttons.h"              // Board Support:Buttons
#include "stm32f7xx_hal.h"              // Device:STM32Cube HAL:Common

static osThreadId_t tid_thrLED;         // Thread id of thread: LED
static osThreadId_t tid_thrBUT;         // Thread id of thread: BUT

TIM_HandleTypeDef htim1;
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void Error_Handler(void);
/*------------------------------------------------------------------------------
  thrLED: blink LED
 *----------------------------------------------------------------------------*/
__NO_RETURN void thrLED (void *argument) {
  uint32_t active_flag = 1U;

  for (;;) {
//    if (osThreadFlagsWait (1U, osFlagsWaitAny, 0U) == 1U) {
//      active_flag ^=1U; 
//    }
//    if (active_flag == 1U){
      LED_On (0U);                                // Switch LED on
      osDelay (500U);                             // Delay 500 ms
      LED_Off (0U);                               // Switch off
      osDelay (500U);                             // Delay 500 ms
//    }
//    else {
//      osDelay (500U);                             // Delay 500 ms
//    }
  }
}

/*------------------------------------------------------------------------------
  thrBUT: check button state
 *----------------------------------------------------------------------------*/
uint32_t wave_top = 0;
uint32_t wave_flag = 0;
uint32_t pwm_value = 0;
__NO_RETURN static void thrBUT(void *argument) {
  uint32_t last;
  uint32_t state;
	
	MX_GPIO_Init();
	MX_TIM1_Init();
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  for (;;) {
    state = (Buttons_GetState () & 1U);           // Get pressed button state
   // if (state != last){
     // if (state == 1){
				osThreadFlagsSet (tid_thrLED, 1U);        // Set flag to thrLED
				
			  for (; pwm_value <= 100; pwm_value++){
					__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value);
					osDelay(20);
					}
				for (; pwm_value >= 1; pwm_value--){
					__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value);
					osDelay(20);
					}
     // }
   // }
    //last = state;
    osDelay (100U);
  }
//		for(;;){
//		if(wave_flag==0)
//		{
//			wave_top++;
//			if(wave_top==50)
//				wave_flag=1;
//		}			
//		else if(wave_flag)
//		{
//			wave_top--;
//			if(wave_top==0)
//				wave_flag=0;
//		}
//		osDelay (10U);
//	}
}

/*------------------------------------------------------------------------------
 * Application main thread
 *----------------------------------------------------------------------------*/
void app_main (void *argument) {

  LED_Initialize ();                    // Initialize LEDs
  Buttons_Initialize ();                // Initialize Buttons

  tid_thrLED = osThreadNew (thrLED, NULL, NULL);  // Create LED thread
  if (tid_thrLED == NULL) { /* add error handling */ }

  tid_thrBUT = osThreadNew (thrBUT, NULL, NULL);  // Create BUT thread
  if (tid_thrBUT == NULL) { /* add error handling */ }

  osThreadExit();
}
static void MX_TIM1_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
	__HAL_RCC_TIM2_CLK_ENABLE(); 
    htim1.Instance = TIM2;
    htim1.Init.Prescaler = 2160-1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 100-1;  // PWM??
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM2;
    sConfigOC.Pulse = 0;  // ?????
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
}
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}