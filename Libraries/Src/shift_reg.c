/*
 * shift_reg.c
 *
 *  Created on: Mar 19, 2024
 *      Authors: Gavin Hua
 *         David Melisso
 *         Sahil Azad
 */

#include "shift_reg.h"
#ifdef HAL_SPI_MODULE_ENABLED
  #include "stm32f4xx_hal.h"
  #include "stm32f4xx_hal_spi.h"
#endif // End of HAL_SPI_MODULE_ENABLED check

#define MAX_SHIFT_REGS 3

#ifdef HAL_SPI_MODULE_ENABLED
#if (USE_HAL_SPI_REGISTER_CALLBACKS == 1)
static void Shift_Reg_SPI_Reset_NSS(SPI_HandleTypeDef *hspi);

static SPI_HandleTypeDef* SPI_Handlers[MAX_SHIFT_REGS];
static Shift_Reg* Shift_Regs[MAX_SHIFT_REGS];
static uint8_t num_shift_regs = 0;
#endif // #if (USE_HAL_SPI_REGISTER_CALLBACKS == 1)
#endif // End of HAL_SPI_MODULE_ENABLED check

Shift_Reg *Shift_Reg_GPIO_Init(GPIO_TypeDef *shcp_port, uint16_t shcp_pin,
            GPIO_TypeDef *stcp_port, uint16_t stcp_pin,
            GPIO_TypeDef *data_port, uint16_t data_pin) {
  Shift_Reg *shift_reg = malloc(sizeof(Shift_Reg));
  shift_reg->Mode      = Shift_Reg_GPIO_Mode;
  shift_reg->SHCP_Port = shcp_port;
  shift_reg->SHCP_Pin  = shcp_pin;
  shift_reg->STCP_Port = stcp_port;
  shift_reg->STCP_Pin  = stcp_pin;
  shift_reg->DATA_Port = data_port;
  shift_reg->DATA_Pin  = data_pin;

  shift_reg->remaining_bits = 0;
  shift_reg->remaining_bytes = 0;
  shift_reg->cur_byte = 0;
  shift_reg->send_data = NULL;

  return shift_reg;
}

#ifdef HAL_TIM_MODULE_ENABLED
// TODO: make GPIO non-blocking mode
#endif // #ifdef HAL_TIM_MODULE_ENABLED

#ifdef HAL_SPI_MODULE_ENABLED

// Hardware NSS disabled because it does not function properly
/*
Shift_Reg *Shift_Reg_SPI_HW_NSS_Init(SPI_HandleTypeDef *hspi) {
  Shift_Reg *shift_reg = malloc(sizeof(Shift_Reg));
  shift_reg->Mode      = Shift_Reg_SPI_HW_NSS_Mode;
  shift_reg->hspi      = hspi;

  return shift_reg;
}
*/

#if (USE_HAL_SPI_REGISTER_CALLBACKS == 1)
Shift_Reg *Shift_Reg_SPI_SW_NSS_Init(SPI_HandleTypeDef *hspi,
                              GPIO_TypeDef *stcp_port, uint16_t stcp_pin) {

  if (num_shift_regs >= MAX_SHIFT_REGS) {
      return NULL;
  }

  // ensure that SPI settings are correct
  uint16_t should_be_unset = SPI_CR1_RXONLY | SPI_CR1_DFF | SPI_CR1_CPHA | SPI_CR1_LSBFIRST | SPI_CR1_CRCEN;
  uint16_t should_be_set   = SPI_CR1_MSTR | SPI_CR1_SSM;
  if ( ( hspi->Instance->CR1 & should_be_set   ) != should_be_set ||
       ( hspi->Instance->CR1 & should_be_unset ) != 0 ) {
      return NULL;
  }

  Shift_Reg *shift_reg = malloc(sizeof(Shift_Reg));
  shift_reg->Mode      = Shift_Reg_SPI_SW_NSS_Mode;
  shift_reg->hspi      = hspi;
  shift_reg->STCP_Port = stcp_port;
  shift_reg->STCP_Pin  = stcp_pin;

  SPI_Handlers[num_shift_regs] = hspi;
  Shift_Regs[num_shift_regs] = shift_reg;
  ++num_shift_regs;

  HAL_SPI_RegisterCallback(hspi, HAL_SPI_TX_COMPLETE_CB_ID, Shift_Reg_SPI_Reset_NSS);

  return shift_reg;
}

static void Shift_Reg_SPI_Reset_NSS(SPI_HandleTypeDef *hspi) {
  for (uint8_t index = 0; index < num_shift_regs; index++) {
      if (SPI_Handlers[index] == hspi) {
          Shift_Reg *shift_reg = Shift_Regs[index];
          HAL_GPIO_WritePin(shift_reg->STCP_Port, shift_reg->STCP_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(shift_reg->STCP_Port, shift_reg->STCP_Pin, GPIO_PIN_SET);

          return;
      }
  }
}
#endif // #if (USE_HAL_SPI_REGISTER_CALLBACKS == 1)
#endif // End of HAL_SPI_MODULE_ENABLED check


static HAL_StatusTypeDef Shift_Reg_Write_Data_GPIO(Shift_Reg *shift_reg, uint8_t* data, uint8_t data_length) {
  GPIO_PinState msb;
  uint8_t cur_byte;

  // for each byte
  while (data_length > 0) {
      // get the current byte
      cur_byte = *data;

      // for each bit
      for (int i = 0; i < 8; i++) {
          // get the current bit
          if ((cur_byte & 0b10000000) > 0) {
              msb = GPIO_PIN_SET;
          }
          else {
              msb = GPIO_PIN_RESET;
          }

          // write that bit to the register
          HAL_GPIO_WritePin(shift_reg->SHCP_Port, shift_reg->SHCP_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(shift_reg->DATA_Port, shift_reg->DATA_Pin, msb);
          HAL_GPIO_WritePin(shift_reg->SHCP_Port, shift_reg->SHCP_Pin, GPIO_PIN_SET);

          // move to the next bit
          cur_byte <<= 1;
      }

      // move to the next byte
      --data_length;
      ++data;
  }
  return HAL_OK;
}

static HAL_StatusTypeDef Shift_Reg_Write_Data_SPI(Shift_Reg *shift_reg, uint8_t* data, uint8_t num_digits) {
#ifdef HAL_SPI_MODULE_ENABLED
  return HAL_SPI_Transmit_IT(shift_reg->hspi, data, num_digits);
#else
  return HAL_ERROR;
#endif
}

HAL_StatusTypeDef Shift_Reg_Write(Shift_Reg *shift_reg, uint8_t* data, uint8_t num_digits) {
  if (shift_reg == NULL) {
      return HAL_ERROR;
  }

  // manually turn off storage clock pin
  if (shift_reg->Mode == Shift_Reg_GPIO_Mode ||
      shift_reg->Mode == Shift_Reg_GPIO_IT_Mode ||
      shift_reg->Mode == Shift_Reg_SPI_SW_NSS_Mode) {
      HAL_GPIO_WritePin(shift_reg->STCP_Port, shift_reg->STCP_Pin, GPIO_PIN_RESET);
  }

  // send the data
  HAL_StatusTypeDef status;
  if (shift_reg->Mode == Shift_Reg_GPIO_Mode) {
      status = Shift_Reg_Write_Data_GPIO(shift_reg, data, num_digits);
  }
  else {
      status = Shift_Reg_Write_Data_SPI(shift_reg, data, num_digits);
  }
  if (status != HAL_OK) {
      return status;
  }

  // manually turn on the storage clock pin
  if (shift_reg->Mode == Shift_Reg_GPIO_Mode) {
      HAL_GPIO_WritePin(shift_reg->STCP_Port, shift_reg->STCP_Pin, GPIO_PIN_SET);
   }

  return HAL_OK;
}
