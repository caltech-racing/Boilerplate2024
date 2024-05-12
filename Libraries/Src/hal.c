/*
 * hal.c
 *
 *  Created on: May 1, 2024
 *      Author: David Melisso
 *
 *  Handles overhead for Hardware Abstraction Layer.
 */

#include "buttons.h"

 void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
   Button_EXTI_Callback(GPIO_Pin);
 }
