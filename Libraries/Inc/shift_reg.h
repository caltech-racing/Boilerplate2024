/*
 * shift_reg.h
 *
 * 74HC595 shift register driver.
 *
 * IMPORTANT NOTES/TROUBLESHOOTING:
 *    1. SPI (if you are using SPI)
 *      - In your IOC file, activate a spi bus that is properly connected
 *        to the 74HC595
 *      - In your NVIC, ensure that 'SPIx global interrupt' is enabled
 *      - Under 'Hardware NSS Signal', select 'Disable'
 *      - Under 'Frame Format', select 'Motorola'
 *      - Under 'Data Size', select '8 Bits'
 *      - Under 'Clock Phase', select '1 Edge'
 *      - Under 'CRC Calculation', select 'Disabled'
 *    2. TIMERS (if you are using GPIO_IT):
 *      - In your IOC file, activate a timer to be used for the sole
 *        purpose of handling the shift register.
 *      - In your NVIC, ensure that 'TIMx global interrupt' is enabled
 *      - This timer should not be used for other purposes
 *    3. INIT:
 *      - Ensure you initialize  according to Usage
 *    4. SETTINGS
 *      - Ensure that USE_HAL_SPI_REGISTER_CALLBACKS is set to 1U in
 *        stm32f4xx_hal_conf.h; set using:
 *        STM32CubeMX Project Manager > Advanced Settings > Register Callback
 *        and on the right side, set the SPI callback to ENABLE
 *
 * Principle of Operation:
 *    We connect three input pins on the 74HC595:
 *      - the data input    (DATA, SER,   SER,       A)
 *      - the storage clock (STCP, RCLK,  RCLK,      LATCH CLOCK)
 *      - the shift   clock (SHCP, SRCLK, SCK,       SHIFT CLOCK)
 *
 *    These go by different names depending on the manufacturer. In order:
 *                          (NXP,  TI,    Fairchild, ONSEMI)
 *
 *    We will be using the NXP terminology.
 *
 *    The 74HC595 two (notable) registers: a shift register and a storage register.
 *
 *    On the rising edge of SHCP, all the bits in shift register are shifted up one bit.
 *    The least significant bit is set to the DATA input, and the most significant
 *    bit is output to the carry pin. The carry pin can be connected to the input
 *    of another 74HC595, allowing the shift registers to be cascaded together.
 *    This way, an arbitrary number of 74HC595 can be controlled on a single bus.
 *
 *    On the rising edge of STCP, all the bits of the shift register are stored in the
 *    storage register, which is output on the output pins of the 74HC595. These
 *    can be used to drive something, like LEDs.
 *
 *  Interfaces:
 *    The 74HC595 input is perfectly designed to work with a SPI output.
 *    Because the microcontroller has dedicated hardware for interfacing with SPI,
 *    it is desirable to use SPI instead of manually controlling the
 *    pins with code.
 *
 *    On the STM32, the SPI buses have a dedicated "hardware" slave select pin,
 *    which outputs high at the end of the operation. This would connect to
 *    the STCP, completing the operation, and should be used if possible.
 *    If not, STCP can be controlled by software ("software" slave select).
 *
 *    Currently, hardware slave select is not available.
 *
 *    Your SPIx bus should be connected to the 74HC595 as:
 *
 *    MOSI -> DATA
 *    SCK  -> SHCP
 *    NSS  -> STCP
 *
 *    If your chip is not connected to a SPI bus, you can control it manually
 *    via GPIO.
 *
 *    Your options for initializing are therefore:
 *    `Shift_Reg_GPIO_Init` -> GPIO/manual control
 *    `Shift_Reg_SPI_SW_NSS_Init` -> SPI control with software slave select
 *
 *  Usage:
 *
 *      #import "shift_reg.h"
 *
 *      // ...
 *
 *      Shift_Reg shift_reg = Shift_Reg_[MODE]_Init(...);
 *
 *      // ...
 *
 *      uint8_t num_bytes;
 *      uint8_t data[] = {}; // length = num_bytes
 *      Shift_Reg_Write(shift_reg, data, num_bytes);
 *
 *      // ...
 *
 *  Created on: Mar 19, 2024
 *      Authors: Gavin Hua
 *         David Melisso
 *         Sahil Azad
 */

#ifndef INC_SHIFT_REG_H_
#define INC_SHIFT_REG_H_

#include "string.h"
#include "main.h"

#ifdef HAL_SPI_MODULE_ENABLED
  #include "stm32f4xx_hal.h"
  #include "stm32f4xx_hal_spi.h"
#endif // End of HAL_SPI_MODULE_ENABLED check
#include <stdlib.h>


typedef enum {
  Shift_Reg_SPI_HW_NSS_Mode,    // SPI with Hardware Slave Select
  Shift_Reg_SPI_SW_NSS_Mode,    // SPI with Software Slave Select
  Shift_Reg_GPIO_Mode,          // Manual GPIO mode, blocking (no interrupts)
  Shift_Reg_GPIO_IT_Mode,       // Manual GPIO mode, non-blocking (with interrupts)
} Shift_Reg_Mode;

typedef struct {
  /**
   * Mode, select from:
   * SPI with hardware slave select,
   * SPI with software slave select,
   * GPIO (manual mode)
   */
  Shift_Reg_Mode Mode;

  /**
   * GPIO Ports
   */
  GPIO_TypeDef *STCP_Port;    // Storage Register clock GPIO Port
  uint16_t     STCP_Pin;      // Storage Register clock GPIO Pin

  GPIO_TypeDef *SHCP_Port;    // Shift Register clock GPIO Port
  uint16_t     SHCP_Pin;      // Shift Register clock GPIO Pin

  GPIO_TypeDef *DATA_Port;    // Register data input GPIO Port
  uint16_t     DATA_Pin;      // Register data input GPIO Pin

  /**
   * Memory for holding data between cycles
   */

  volatile uint8_t  remaining_bytes;  // number of bytes left to send
  volatile uint8_t  remaining_bits;   // number of bits left to send in current byte
  volatile uint8_t *send_data;        // pointer to current byte in array
  volatile uint8_t  cur_byte;         // working byte (being modified)

  TIM_HandleTypeDef  *htim;           // timer for timing GPIO outputs

  #ifdef HAL_SPI_MODULE_ENABLED
  /**
   * SPI handler
   */
  SPI_HandleTypeDef *hspi;

  #endif // End of HAL_SPI_MODULE_ENABLED check
} Shift_Reg;

/**
 * Initializes shift register using GPIO. Does not use interrupts,
 * so this is blocking.
 *
 * @param stcp_port   the GPIO port corresponding with the STCP pin of the
 *                    74HC595
 * @param stcp_pin    the GPIO pin corresponding with the STCP pin of the
 *                    74HC595
 * @param shcp_port   the GPIO port corresponding with the SHCP pin of the
 *                    74HC595
 * @param shcp_pin    the GPIO pin corresponding with the SHCP pin of the
 *                    74HC595
 * @param data_port   the GPIO port corresponding with the data pin of the
 *                    74HC595
 * @param data_pin    the GPIO pin corresponding with the data pin of the
 *                    74HC595
 *
 * @retval the Shift_Reg handler configured to the given pins
 */
Shift_Reg *Shift_Reg_GPIO_Init(GPIO_TypeDef *shcp_port, uint16_t shcp_pin,
          GPIO_TypeDef *stcp_port, uint16_t stcp_pin,
          GPIO_TypeDef *data_port, uint16_t data_pin);

#ifdef HAL_SPI_MODULE_ENABLED

#if (USE_HAL_SPI_REGISTER_CALLBACKS == 1)
/**
 * Initializes shift register using SPI, with software slave select pin.
 *
 * Use this if your 74HC595 shift clock (SHCP) and data pins are connected
 * to the SPI bus correctly, but your storage clock (STCP) is not connected
 * to SPIx_NSS.
 *
 * @param hspi        the spi handler corresponding to the SPI bus connected
 *                    to the 74HC595
 * @param stcp_port   the GPIO port corresponding with the STCP pin of the
 *                    74HC595
 * @param stcp_pin    the GPIO pin corresponding with the STCP pin of the
 *                    74HC595
 *
 * @retval the Shift_Reg handler configured to the given SPI bus and NSS pin
 */
Shift_Reg *Shift_Reg_SPI_SW_NSS_Init(SPI_HandleTypeDef *hspi,
                              GPIO_TypeDef *stcp_port, uint16_t stcp_pin);

#endif // #if (USE_HAL_SPI_REGISTER_CALLBACKS == 1)


/**
 * Initializes shift register using SPI, with hardware slave select pin.
 *
 * This is the most preferred method of interfacing with the 74HC595 chip.
 * Use this method if your SPI bus is connected as:
 * MOSI -> DATA
 * SCK  -> SHCP
 * NSS  -> STCP
 *
 * @param hspi        the spi handler corresponding to the SPI bus connected
 *                    to the 74HC595
 *
 * @retval the Shift_Reg handler configured to the given SPI bus
 */
// Shift_Reg *Shift_Reg_SPI_HW_NSS_Init(SPI_HandleTypeDef *hspi);



#endif // End of HAL_SPI_MODULE_ENABLED check

/**
 * Writes a value to a cascade of 74HC595 chips.
 *
 * @param shift_reg   a reference to the shift register instance
 * @param data        array of bytes to store in the shift registers
 * @param data_length the number of bytes in data
 *
 * @error   return HAL_StatusTypeDef error
 *
 * @retval  the HAL_StatusTypeDef status of the operation
 */
HAL_StatusTypeDef Shift_Reg_Write(Shift_Reg *shift_reg, uint8_t* data, uint8_t data_length);


#endif /* INC_SHIFT_REG_H_ */
