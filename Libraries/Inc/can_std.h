/*
 * can.h
 *
 *  Created on: May 20, 2024
 *      Author: Caltech Racing
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "stm32f4xx_hal.h"

typedef enum {

  CAN_ID_HIGH_PRIO      = 0x000,

  CAN_ID_AMS            = 0x010,    /* Accumulator Management System */
  CAN_ID_CHARGER        = 0x020,    /* Charger */
  CAN_ID_MOTOR          = 0x030,    /* Motor Controller */
  CAN_ID_DASH           = 0x040,    /* Dashboard */
  CAN_ID_PEDAL          = 0x050,    /* Pedal Board */

  CAN_ID_TACH           = 0x400,    /* Wheel Speed Sensor */
  CAN_ID_STEER          = 0x410,    /* Steering Angle Sensor */

  CAN_ID_LOW_PRIO       = 0x7FF     /* Testing/Debugging */

} CAN_ID;


#endif /* INC_CAN_H_ */
