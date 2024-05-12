# Boilerplate2024
Boilerplate for Caltech Racing code on the STM32F412VxT platform.
Contains several libraries for typical applications of the STM32F412VxT.

## Authors
[David Melisso](https://github.com/DJMcoder)
[Sahil Azad](https://github.com/Sakki2010)
[Gavin Hua](https://github.com/gavin-hyl)
[Garret Knuf](https://github.com/garrettknuf)

## Libraries
Libraries are contained within Libraries/Inc

### Buttons
`buttons.h`
Button debouncing library.

##### IMPORTANT NOTES/TROUBLESHOOTING:
1. TIMERS:
     - In your IOC file, activate a timer to be used for the sole
       purpose of debouncing.
     - In your NVIC, ensure that 'TIMx global interrupt' is enabled
	 - This timer should not be used for other purposes
2. INIT:
     - Ensure you initialize your buttons according to Usage
3. EXTI:
     - Set the buttons you want to handle as EXTI pins
     - ensure that the EXTI triggers on both the rising
       and falling edge
     - ensure the appropriate EXTI line interrupt is
       set in NVIC
     - ensure that `Button_EXTI_Callback` is called in `HAL_GPIO_EXTI_Callback`
4. SETTINGS
     - Ensure that `USE_HAL_TIM_REGISTER_CALLBACKS` is set to `1U` in
       `stm32f4xx_hal_conf.h`; set using:
       STM32CubeMX Project Manager > Advanced Settings > Register Callback
       and on the right side, set the TIM callback to ENABLE

##### Usage

```c
#import "buttons.h"

// ...

void Debug_Button_1_Handler(GPIO_PinState state) {
	// ...
}

//...

Init_Button_Begin();
Init_Button(
	DEBUG_BUTTON_1_GPIO_Port,
	DEBUG_BUTTON_1_Pin,
	GPIO_PIN_SET,
	Debug_Button_1_Handler);
Init_Button( /* ... */ );
Init_Button( /* ... */ );
Init_Button_Finish();

// ...
```

##### Functions
`HAL_StatusTypeDef Init_Button_Begin();`
`Button *Init_Button(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState init_state, button_callback_t cb);`
`HAL_StatusTypeDef Init_Button_Finish();`
`HAL_StatusTypeDef Button_EXTI_Callback(uint16_t GPIO_Pin);`

### Shift Register
`shift_reg.h`
74HC595 shift register driver.

##### IMPORTANT NOTES/TROUBLESHOOTING:
1. SPI (if you are using SPI)
   - In your IOC file, activate a spi bus that is properly connected
     to the 74HC595
   - In your NVIC, ensure that 'SPIx global interrupt' is enabled
2. TIMERS (if you are using GPIO_IT):
   - In your IOC file, activate a timer to be used for the sole
     purpose of handling the shift register.
   - In your NVIC, ensure that 'TIMx global interrupt' is enabled
   - This timer should not be used for other purposes
3. INIT:
   - Ensure you initialize  according to Usage
4. SETTINGS
   - Ensure that `USE_HAL_SPI_REGISTER_CALLBACKS` is set to `1U` in
     `stm32f4xx_hal_conf.h`; set using:
     STM32CubeMX Project Manager > Advanced Settings > Register Callback
     and on the right side, set the SPI callback to ENABLE

##### Principle of Operation
We connect three input pins on the 74HC595:
  - the data input    (`DATA`, `SER`,   `SER`,       `A`)
  - the storage clock (`STCP`, `RCLK`,  `RCLK`,      `LATCH CLOCK`)
  - the shift   clock (`SHCP`, `SRCLK`, `SCK`,       `SHIFT CLOCK`)

These go by different names depending on the manufacturer. In order:
                      (NXP,  TI,    Fairchild, ONSEMI)

We will be using the NXP terminology.

The 74HC595 two (notable) registers: a shift register and a storage register.

On the rising edge of `SHCP`, all the bits in shift register are shifted up one bit. 
The least significant bit is set to the `DATA` input, and the most significant
bit is output to the carry pin. The carry pin can be connected to the input
of another 74HC595, allowing the shift registers to be cascaded together.
This way, an arbitrary number of 74HC595 can be controlled on a single bus.

On the rising edge of `STCP`, all the bits of the shift register are stored in the
storage register, which is output on the output pins of the 74HC595. These
can be used to drive something, like LEDs.

##### Interfaces
The 74HC595 input is perfectly designed to work with a SPI output.
Because the microcontroller has dedicated hardware for interfacing with SPI,
it is desirable to use SPI instead of manually controlling the
pins with code.

On the STM32, the SPI buses have a dedicated "hardware" slave select pin,
which outputs high at the end of the operation. This would connect to
the STCP, completing the operation, and should be used if possible.
If not, STCP can be controlled by software ("software" slave select).

Currently, hardware slave select is not available.

Your SPIx bus should be connected to the 74HC595 as:

`MOSI` -> `DATA`
`SCK`  -> `SHCP`
`NSS`  -> `STCP`

If your chip is not connected to a SPI bus, you can control it manually
via GPIO.

Your options for initializing are therefore:
`Shift_Reg_GPIO_Init` -> GPIO/manual control
`Shift_Reg_SPI_SW_NSS_Init` -> SPI control with software slave select

##### Usage
```c
#import "shift_reg.h"

// ...

Shift_Reg shift_reg = Shift_Reg_[MODE]_Init(...);

// ...

uint8_t num_bytes;
uint8_t data[] = {}; // length = num_bytes
Shift_Reg_Write(shift_reg, data, num_bytes);

// ...
```

##### Functions
`Shift_Reg *Shift_Reg_GPIO_Init(GPIO_TypeDef *shcp_port, uint16_t shcp_pin,
          GPIO_TypeDef *stcp_port, uint16_t stcp_pin,
          GPIO_TypeDef *data_port, uint16_t data_pin);`
          
`Shift_Reg *Shift_Reg_SPI_SW_NSS_Init(SPI_HandleTypeDef *hspi,
                              GPIO_TypeDef *stcp_port, uint16_t stcp_pin);`

`HAL_StatusTypeDef Shift_Reg_Write(Shift_Reg *shift_reg, uint8_t* data, uint8_t data_length);`

### Seven Segment Display
`seven_seg.h`
Library for using the DC56-11EWA seven segment display with the 74HC595
shift register as is connected on uControllerTemplate2024.PrjPcb in Altium.

##### Usage
```c
#import "seven_seg.h"

// ...

Shift_Reg shift_reg = Shift_Reg_[MODE]_Init(...);
Seven_Seg seven_seg = Seven_Seg_Init(shift_reg);

// ...

// write individual LEDs
Seven_Seg_Write_Raw(seven_seg, SEVEN_SEG_LEFT_G | SEVEN_SEG_LEFT_D, 0xFF & ~SEVEN_SEG_RIGHT_DP);

// write "A9", and the right decimal place
Seven_Seg_Write_Hex(seven_seg, 0xA9, 0, 1);

// write "-8" to the display
Seven_Seg_Write_Integer(seven_seg, -8);

// write "3.5" to the display
Seven_Seg_Write_Decimal(seven_seg, 3.5);

// write "Hi." to the display
Seven_Seg_Write_Chars(seven_seg, "Hi", 0, 1);

// write "hi" to the display
Seven_Seg_Write_Text(seven_seg, "hi");

// ...
```