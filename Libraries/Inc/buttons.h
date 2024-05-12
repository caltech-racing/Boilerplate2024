/*
 * buttons.h
 *
 *  Created on: Apr 17, 2024
 *     Authors: David Melisso
 *              Sahil Azad
 *
 * Button debouncing library.
 *
 * IMPORTANT NOTES/TROUBLESHOOTING:
 *    1. TIMERS:
 *      - In your IOC file, activate a timer to be used for the sole
 *        purpose of debouncing.
 *      - In your NVIC, ensure that 'TIMx global interrupt' is enabled
 *		  - This timer should not be used for other purposes
 *    2. INIT:
 *      - Ensure you initialize your buttons according to Usage
 *    3. EXTI:
 *      - Set the buttons you want to handle as EXTI pins
 *      - ensure that the EXTI triggers on both the rising
 *        and falling edge
 *      - ensure the appropriate EXTI line interrupt is
 *        set in NVIC
 *      - ensure that Button_EXTI_Callback is called in HAL_GPIO_EXTI_Callback
 *    4. SETTINGS
 *      - Ensure that USE_HAL_TIM_REGISTER_CALLBACKS is set to 1U in
 *        stm32f4xx_hal_conf.h; set using:
 *        STM32CubeMX Project Manager > Advanced Settings > Register Callback
 *        and on the right side, set the TIM callback to ENABLE
 *
 * Usage:
 *
 *    #import "buttons.h"
 *     // ...
 *     void Debug_Button_1_Handler(GPIO_PinState state) {
 *        // ...
 *      }
 *     //...
 *     Init_Button_Begin();
 *     Init_Button(
 *         DEBUG_BUTTON_1_GPIO_Port,
 *         DEBUG_BUTTON_1_Pin,
 *         GPIO_PIN_SET,
 *         Debug_Button_1_Handler);
 *     Init_Button( ... );
 *     Init_Button( ... );
 *    Init_Button_Finish();
 *    // ...
 *
 */
#include "stm32f4xx.h"

/* Definitions */
typedef void (*button_callback_t)(GPIO_PinState state);

typedef struct {
  GPIO_TypeDef *Port;
  uint16_t Pin;
  button_callback_t callback;
  GPIO_PinState last_state;
} Button;


/* Functions */

/**
 * Initializes button registration, allows for individual buttons to be initialized
 *
 * @param htim pointer to the timer handler used for debouncing
 * @param debounce_time wait time for button pattern to settle, in ms
 *
 * @error returns HAL_StatusTypeDef
 *
 * @retval the status of the operation
 */
HAL_StatusTypeDef Init_Button_Begin();

/**
 * Initializes an individual button
 *
 * @param port        the GPIO port the button is associated with
 * @param pin         the GPIO pin  the button is associated with
 * @param init_state  the initial state the button pin should be in
 * @param cb          the callback which should be called when the button state changes
 *
 * @error returns NULL
 *
 * @retval a reference to the button if there was no errors, NULL otherwise
 */
Button *Init_Button(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState init_state, button_callback_t cb);

/**
 * Finishes initialization of buttons.
 *
 * @error returns HAL_StatusTypeDef
 *
 * @retval the status of the opperation
 */
HAL_StatusTypeDef Init_Button_Finish();

/**
 * Given an EXTI pin trigger, if the pin is associated with a registered button,
 * reset the button timer.
 *
 * Intended to be called from HAL_GPIO_EXTI_Callback
 *
 * @param GPIO_Pin the EXTI pin that was triggered
 *
 * @error returns HAL_StatusTypeDef
 *
 * @retval returns the status of the operation
 */
HAL_StatusTypeDef Button_EXTI_Callback(uint16_t GPIO_Pin);
