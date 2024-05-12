/*
 * buttons.c
 *
 *  Created on: Apr 17, 2024
 *      Authors: David Melisso
 *      	       Sahil Azad
 *
 * See buttons.h for usage and troubleshooting.
 *
 * Functionality:
 *  All buttons are expected to be registered in the GPIO_EXTIx state, and should trigger
 * 	on both the rising and falling edge. All EXTI calls will reset the button timer's count,
 * 	and start the timer. When the button timer period elapses, the button pattern is
 * 	expected to have 'settled', so each of the registered button callbacks are called, with
 * 	the current state of the pin.
 */

/* INCLUDES */
#include "stm32f4xx_hal.h"
#include "buttons.h"

#define MAX_BUTTONS 16

// ensure that timer register callbacks are enabled
#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
#ifdef HAL_TIM_MODULE_ENABLED

/* GLOBAL VARS */
TIM_HandleTypeDef *htim_debounce; // timer used for debounce handling
uint8_t NUM_BUTTONS = 0;          // current number of registered btns
Button buttons[MAX_BUTTONS];      // array of registered buttons
uint16_t button_mask;             // pin register mask of used EXTI pins

/* PRIVATE FUNCTIONS */
static HAL_StatusTypeDef Debounce_Timer_Init(TIM_HandleTypeDef* htim, uint32_t debounce_time);
static void Debounce_Button_Pattern(TIM_HandleTypeDef *htim);

/* FUNCTION IMPLEMENTATIONS */


HAL_StatusTypeDef Init_Button_Begin(TIM_HandleTypeDef* htim, uint32_t debounce_time) {
  static uint8_t initialized = 0;

  // Only run once
  if (initialized != 0) {
      return HAL_OK;
  }
  initialized = 1;

  button_mask = 0;

  // setup button debounce timer
  return Debounce_Timer_Init(htim, debounce_time);
}

Button *Init_Button(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState init_state, button_callback_t cb) {

  // ensure we have not initialized too many buttons
  if (NUM_BUTTONS == MAX_BUTTONS){
      return NULL;
  }

  // get number associated with pin (i.e. PA9 -> 9)
  uint8_t pin_number = 0;
  uint16_t pin_register = pin;
  while (pin_register != 0) {
      pin_register >>= 1;
      pin_number++;
  }
  pin_number--;

  // verify that button is an exti pin and port
  // reverse-engineered from stm32f4xx_hal_gpio.c
  uint32_t port_index = ( SYSCFG->EXTICR[pin_number >> 2U] >> (4U * (pin_number & 0x03U)) ) & 0x0F;
  if ( ( ( EXTI->IMR & pin ) == 0 ) || ( port_index != GPIO_GET_INDEX(port)) ) {
      return NULL;
  }

  // add button to array
  button_mask |= pin;
  buttons[NUM_BUTTONS] = (Button){ Port: port, Pin: pin, last_state: init_state, callback: cb};
  return &buttons[NUM_BUTTONS++];
}


HAL_StatusTypeDef Init_Button_Finish() {
  return HAL_OK;
}

/**
 * Calls button callbacks if the button state has changed.
 * Should be called when the button timer elapses.
 *
 * @param htim the timer handler whose period elapsed (should be the button timer)
 */
static void Debounce_Button_Pattern(TIM_HandleTypeDef *htim) {
  HAL_TIM_Base_Stop_IT(htim);
  for (uint8_t button = 0; button < NUM_BUTTONS; button++) {
      GPIO_PinState new_state = HAL_GPIO_ReadPin(buttons[button].Port, buttons[button].Pin);
      if(new_state != buttons[button].last_state){
          buttons[button].callback(new_state);
          buttons[button].last_state = new_state;
      }
  }
  return;
}

HAL_StatusTypeDef Button_EXTI_Callback(uint16_t GPIO_Pin)
{
  HAL_StatusTypeDef status;
  // if pin is associated with a button
  if ( ( GPIO_Pin & button_mask ) != 0) {
      // reset and start the timer
      status = HAL_TIM_Base_Stop_IT(htim_debounce);
      if (status != HAL_OK) {
          return status;
      }
      __HAL_TIM_SET_COUNTER(htim_debounce, 0);
      status = HAL_TIM_Base_Start_IT(htim_debounce);
      if (status != HAL_OK) {
          return status;
      }
  }
  return HAL_OK;
}


/**
 * Initializes the button debounce timer.
 *
 * @param htim pointer to the timer handler used for debouncing
 * @param debounce_time wait time for button pattern to settle, in ms
 */
static HAL_StatusTypeDef Debounce_Timer_Init(TIM_HandleTypeDef* htim, uint32_t debounce_time)
{
  HAL_StatusTypeDef status;

  htim->Init.Prescaler = HAL_RCC_GetPCLK1Freq()/10000 - 1;
  htim->Init.Period = 10 * debounce_time - 1;

  htim->Init.CounterMode = TIM_COUNTERMODE_UP;
  htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  status = HAL_TIM_Base_Init(htim);
  if (status != HAL_OK) {
      return status;
  }

  HAL_TIM_Base_Stop_IT(htim);

  pTIM_CallbackTypeDef callback = Debounce_Button_Pattern;

  status = HAL_TIM_RegisterCallback(htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, callback);
  if (status != HAL_OK) {
      return status;
  }

  htim_debounce = htim;

  return HAL_OK;
}

#endif // #ifdef HAL_TIM_MODULE_ENABLED
#endif // #if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)

