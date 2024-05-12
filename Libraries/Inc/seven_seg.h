/*
 * seven_seg.h
 *
 * Library for using the DC56-11EWA seven segment display with the 74HC595
 * shift register as is connected on uControllerTemplate2024.PrjPcb in Altium.
 *
 * See additional documentation in shift_reg.h .
 *
 *  Created on: May 2, 2024
 *     Authors: David Melisso
 *              Sahil Azad
 *              Gavin Hua
 *
 * Usage:
 *
 *
 *      #import "seven_seg.h"
 *
 *      // ...
 *
 *      Shift_Reg shift_reg = Shift_Reg_[MODE]_Init(...);
 *      Seven_Seg seven_seg = Seven_Seg_Init(shift_reg);
 *
 *      // ...
 *
 *      // write individual LEDs
 *      Seven_Seg_Write_Raw(seven_seg, SEVEN_SEG_LEFT_G | SEVEN_SEG_LEFT_D, 0xFF & ~SEVEN_SEG_RIGHT_DP);
 *
 *      // write "A9", and the right decimal place
 *      Seven_Seg_Write_Hex(seven_seg, 0xA9, 0, 1);
 *
 *      // write "-8" to the display
 *      Seven_Seg_Write_Integer(seven_seg, -8);
 *
 *      // write "3.5" to the display
 *      Seven_Seg_Write_Decimal(seven_seg, 3.5);
 *
 *      // write "Hi." to the display
 *      Seven_Seg_Write_Chars(seven_seg, "Hi", 0, 1);
 *
 *      // write "hi" to the display
 *      Seven_Seg_Write_Text(seven_seg, "hi");
 *
 *      // ...
 */

#ifndef INC_SEVEN_SEG_H_
#define INC_SEVEN_SEG_H_

// Individual LEDs on seven segment

#define SEVEN_SEG_LEFT_DP  0b00010000 /* Left digit decimal place LED */
#define SEVEN_SEG_LEFT_G   0b00000010 /* Left digit middle LED        */
#define SEVEN_SEG_LEFT_F   0b00000001 /* Left digit left-top LED      */
#define SEVEN_SEG_LEFT_E   0b10000000 /* Left digit left-bot LED      */
#define SEVEN_SEG_LEFT_D   0b01000000 /* Left digit bottom LED        */
#define SEVEN_SEG_LEFT_C   0b00100000 /* Left digit right-bot LED     */
#define SEVEN_SEG_LEFT_B   0b00001000 /* Left digit right-top LED     */
#define SEVEN_SEG_LEFT_A   0b00000100 /* Left digit top LED           */

#define SEVEN_SEG_RIGHT_DP 0b00000001 /* Right digit decimal place LED */
#define SEVEN_SEG_RIGHT_G  0b00000100 /* Right digit middle LED        */
#define SEVEN_SEG_RIGHT_F  0b00100000 /* Right digit left-top LED      */
#define SEVEN_SEG_RIGHT_E  0b00010000 /* Right digit left-bot LED      */
#define SEVEN_SEG_RIGHT_D  0b00001000 /* Right digit bottom LED        */
#define SEVEN_SEG_RIGHT_C  0b00000010 /* Right digit right-bot LED     */
#define SEVEN_SEG_RIGHT_B  0b10000000 /* Right digit right-top LED     */
#define SEVEN_SEG_RIGHT_A  0b01000000 /* Right digit top LED           */

#include "shift_reg.h"

typedef struct {
  Shift_Reg *shift_reg;
} Seven_Seg;

/**
 * Initializes a Seven Segment handler
 *
 * @param shift_reg   reference to the shift register handler associated
 *                    with the seven segment
 *
 * @retval a reference to the seven segment handler
 */
Seven_Seg *Seven_Seg_Init(Shift_Reg *shift_reg);

/**
 * Write raw binary into the seven segment display
 *
 * @param seven_seg  the seven seg handler
 * @param left_raw   the raw binary to write to the left digit
 * @param right_raw  the raw binary to write to the right digit
 *
 * @error   return HAL_StatusTypeDef error
 *
 * @retval  the HAL_StatusTypeDef status of the operation
 */
HAL_StatusTypeDef Seven_Seg_Write_Raw(Seven_Seg *seven_seg, uint8_t left_raw, uint8_t right_raw);

/**
 * Writes a hexidecimal value to the seven segment display.
 *
 * E.g. 0xA9 will write "A9" to the display.
 *
 * @param seven_seg  the seven seg handler
 * @param hex        the hex value to display
 * @param left_dp    0 if the left  decimal point should be on, 1 otherwise
 * @param right_dp   0 if the right decimal point should be on, 1 otherwise
 *
 * @error   return HAL_StatusTypeDef error
 *
 * @retval  the HAL_StatusTypeDef status of the operation
 */
HAL_StatusTypeDef Seven_Seg_Write_Hex(Seven_Seg *seven_seg, uint8_t hex, uint8_t left_dp, uint8_t right_dp);

/**
 * Writes an integer value to the display
 *
 * E.g. 95 will write "95" to the display.
 * E.g. -9 will write "-9" to the display.
 *
 * Assumes -10 < val < 100
 *
 * @param seven_seg  the seven seg handler
 * @param val        the integer value to display
 *
 * @error   return HAL_StatusTypeDef error
 *
 * @retval  the HAL_StatusTypeDef status of the operation
 */
HAL_StatusTypeDef Seven_Seg_Write_Integer(Seven_Seg *seven_seg, int8_t val);

/**
 * Writes a float/decimal value to the display
 *
 * E.g. 3.5 will write "3.5" to the display.
 * E.g. 59  will write "59." to the display.
 * E.g. -5  will write "-5." to the display.
 *
 * Only two sig figs are possible for positive numbers,
 * and only one sig fig is possible for negative numbers.
 * Remaining sig figs are truncated (not rounded).
 *
 * Assumes -10.0 < val < 100.0
 *
 * @param seven_seg  the seven seg handler
 * @param val        the decimal value to display
 *
 * @error   return HAL_StatusTypeDef error
 *
 * @retval  the HAL_StatusTypeDef status of the operation
 */
HAL_StatusTypeDef Seven_Seg_Write_Decimal(Seven_Seg *seven_seg, float val);

/**
 * Writes two characters to the display, with decimal points.
 *
 * E.g. Ab will write "Ab" to the display.
 *
 * Reference this README file for how the text will appear.
 * https://github.com/dmadison/LED-Segment-ASCII/tree/master?tab=readme-ov-file#7-segment
 * As noted: "It's more difficult to display certain characters with the limited segments -
 * some require context and a bit of imagination."
 *
 * @param seven_seg  the seven seg handler
 * @param text       the two characters to display
 * @param left_dp    0 if the left  decimal point should be on, 1 otherwise
 * @param right_dp   0 if the right decimal point should be on, 1 otherwise
 *
 * @error   return HAL_StatusTypeDef error
 *
 * @retval  the HAL_StatusTypeDef status of the operation
 */
HAL_StatusTypeDef Seven_Seg_Write_Chars(Seven_Seg *seven_seg, char text[2], uint8_t left_dp, uint8_t right_dp);

/**
 * Writes two characters to the display, without decimal points.
 *
 * E.g. Ab will write "Ab" to the display.
 *
 * Reference this README file for how the text will appear.
 * https://github.com/dmadison/LED-Segment-ASCII/tree/master?tab=readme-ov-file#7-segment
 * As noted: "It's more difficult to display certain characters with the limited segments -
 * some require context and a bit of imagination."
 *
 * @param seven_seg  the seven seg handler
 * @param text       the two characters to display
 *
 * @error   return HAL_StatusTypeDef error
 *
 * @retval  the HAL_StatusTypeDef status of the operation
 */
HAL_StatusTypeDef Seven_Seg_Write_Text(Seven_Seg *seven_seg, char text[2]);

#endif /* INC_SEVEN_SEG_H_ */
