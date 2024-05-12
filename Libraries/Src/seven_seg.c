/*
 * seven_seg.c
 *
 *  Created on: May 2, 2024
 *     Authors: David Melisso
 *              Sahil Azad
 *              Gavin Hua
 */

#include "seven_seg.h"
#include <math.h>

#define ASCII_MAX  128
#define ASCII_SKIP 32

/* Private functions */
static char hex_to_char(uint8_t hex);
static HAL_StatusTypeDef Seven_Seg_Write_Integer_DP(Seven_Seg *seven_seg, int8_t val, uint8_t left_dp, uint8_t right_dp);
static HAL_StatusTypeDef Seven_Seg_Write_Hexes(Seven_Seg *seven_seg, uint8_t left_val, uint8_t right_val, uint8_t left_dp, uint8_t right_dp);

// table borrowed from
// https://github.com/dmadison/LED-Segment-ASCII/blob/master/7-Segment/7-Segment-ASCII_BIN.txt
static const uint8_t LEFT_ASCII[ASCII_MAX - ASCII_SKIP] = {
    0b00000000, /*   */
    0b00111000, /* ! */
    0b00001001, /* " */
    0b11101011, /* # */
    0b01100111, /* $ */
    0b10011010, /* % */
    0b00101010, /* & */
    0b00000001, /* ' */
    0b01000101, /* ( */
    0b01001100, /* ) */
    0b00000101, /* * */
    0b10000011, /* + */
    0b10000000, /* , */
    0b00000010, /* - */
    0b00010000, /* . */
    0b10001010, /* / */
    0b11101101, /* 0 */
    0b00101000, /* 1 */
    0b11001110, /* 2 */
    0b01101110, /* 3 */
    0b00101011, /* 4 */
    0b01100111, /* 5 */
    0b11100111, /* 6 */
    0b00101100, /* 7 */
    0b11101111, /* 8 */
    0b01101111, /* 9 */
    0b01000100, /* : */
    0b01100100, /* ; */
    0b00000111, /* < */
    0b01000010, /* = */
    0b00001110, /* > */
    0b10011110, /* ? */
    0b11101110, /* @ */
    0b10101111, /* A */
    0b11100011, /* B */
    0b11000101, /* C */
    0b11101010, /* D */
    0b11000111, /* E */
    0b10000111, /* F */
    0b11100101, /* G */
    0b10101011, /* H */
    0b10000001, /* I */
    0b11101000, /* J */
    0b10100111, /* K */
    0b11000001, /* L */
    0b10100100, /* M */
    0b10101101, /* N */
    0b11101101, /* O */
    0b10001111, /* P */
    0b01001111, /* Q */
    0b10001101, /* R */
    0b01100111, /* S */
    0b11000011, /* T */
    0b11101001, /* U */
    0b11101001, /* V */
    0b01001001, /* W */
    0b10101011, /* X */
    0b01101011, /* Y */
    0b11001110, /* Z */
    0b11000101, /* [ */
    0b00100011, /* \ */
    0b01101100, /* ] */
    0b00001101, /* ^ */
    0b01000000, /* _ */
    0b00001000, /* ` */
    0b11101110, /* a */
    0b11100011, /* b */
    0b11000010, /* c */
    0b11101010, /* d */
    0b11001111, /* e */
    0b10000111, /* f */
    0b01101111, /* g */
    0b10100011, /* h */
    0b10000000, /* i */
    0b01100000, /* j */
    0b10100111, /* k */
    0b10000001, /* l */
    0b10100000, /* m */
    0b10100010, /* n */
    0b11100010, /* o */
    0b10001111, /* p */
    0b00101111, /* q */
    0b10000010, /* r */
    0b01100111, /* s */
    0b11000011, /* t */
    0b11100000, /* u */
    0b11100000, /* v */
    0b10100000, /* w */
    0b10101011, /* x */
    0b01101011, /* y */
    0b11001110, /* z */
    0b00101010, /* { */
    0b10000001, /* | */
    0b10000011, /* } */
    0b00000100, /* ~ */
};

static const uint8_t RIGHT_ASCII[ASCII_MAX - ASCII_SKIP] = {
    0b00000000, /*   */
    0b10000011, /* ! */
    0b10100000, /* " */
    0b10111110, /* # */
    0b01101110, /* $ */
    0b10010101, /* % */
    0b10000110, /* & */
    0b00100000, /* ' */
    0b01101000, /* ( */
    0b11001000, /* ) */
    0b01100000, /* * */
    0b00110100, /* + */
    0b00010000, /* , */
    0b00000100, /* - */
    0b00000001, /* . */
    0b10010100, /* / */
    0b11111010, /* 0 */
    0b10000010, /* 1 */
    0b11011100, /* 2 */
    0b11001110, /* 3 */
    0b10100110, /* 4 */
    0b01101110, /* 5 */
    0b01111110, /* 6 */
    0b11000010, /* 7 */
    0b11111110, /* 8 */
    0b11101110, /* 9 */
    0b01001000, /* : */
    0b01001010, /* ; */
    0b01100100, /* < */
    0b00001100, /* = */
    0b11000100, /* > */
    0b11010101, /* ? */
    0b11011110, /* @ */
    0b11110110, /* A */
    0b00111110, /* B */
    0b01111000, /* C */
    0b10011110, /* D */
    0b01111100, /* E */
    0b01110100, /* F */
    0b01111010, /* G */
    0b10110110, /* H */
    0b00110000, /* I */
    0b10011010, /* J */
    0b01110110, /* K */
    0b00111000, /* L */
    0b01010010, /* M */
    0b11110010, /* N */
    0b11111010, /* O */
    0b11110100, /* P */
    0b11101100, /* Q */
    0b11110000, /* R */
    0b01101110, /* S */
    0b00111100, /* T */
    0b10111010, /* U */
    0b10111010, /* V */
    0b10101000, /* W */
    0b10110110, /* X */
    0b10101110, /* Y */
    0b11011100, /* Z */
    0b01111000, /* [ */
    0b00100110, /* \ */
    0b11001010, /* ] */
    0b11100000, /* ^ */
    0b00001000, /* _ */
    0b10000000, /* ` */
    0b11011110, /* a */
    0b00111110, /* b */
    0b00011100, /* c */
    0b10011110, /* d */
    0b11111100, /* e */
    0b01110100, /* f */
    0b11101110, /* g */
    0b00110110, /* h */
    0b00010000, /* i */
    0b00001010, /* j */
    0b01110110, /* k */
    0b00110000, /* l */
    0b00010010, /* m */
    0b00010110, /* n */
    0b00011110, /* o */
    0b11110100, /* p */
    0b11100110, /* q */
    0b00010100, /* r */
    0b01101110, /* s */
    0b00111100, /* t */
    0b00011010, /* u */
    0b00011010, /* v */
    0b00010010, /* w */
    0b10110110, /* x */
    0b10101110, /* y */
    0b11011100, /* z */
    0b10000110, /* { */
    0b00110000, /* | */
    0b00110100, /* } */
    0b01000000, /* ~ */
};

Seven_Seg *Seven_Seg_Init(Shift_Reg *shift_reg) {
  Seven_Seg *seven_seg = malloc(sizeof(Seven_Seg));
  seven_seg->shift_reg = shift_reg;
  return seven_seg;
}

HAL_StatusTypeDef Seven_Seg_Write_Raw(Seven_Seg *seven_seg, uint8_t left_raw, uint8_t right_raw) {
  uint8_t data[2] = { right_raw, left_raw };
  return Shift_Reg_Write(seven_seg->shift_reg, data, 2);
}

static char hex_to_char(uint8_t hex) {
  hex &= 0x0F;
  if (hex < 10) {
      return hex + '0';
  }

  switch (hex) {
    case 0x0A:
      return 'A';
    case 0x0B:
      return 'b';
    case 0x0C:
      return 'C';
    case 0x0D:
      return 'd';
    case 0x0E:
      return 'E';
    case 0x0F:
      return 'F';
    default:
      return ' ';
  }
}

static HAL_StatusTypeDef Seven_Seg_Write_Hexes(Seven_Seg *seven_seg, uint8_t left_val, uint8_t right_val, uint8_t left_dp, uint8_t right_dp) {
  char left = hex_to_char(left_val);
  char right = hex_to_char(right_val);

  char text[2] = { left, right };
  return Seven_Seg_Write_Chars(seven_seg, text, left_dp, right_dp);
}

HAL_StatusTypeDef Seven_Seg_Write_Hex(Seven_Seg *seven_seg, uint8_t hex, uint8_t left_dp, uint8_t right_dp) {
  return Seven_Seg_Write_Hexes(seven_seg, hex >> 4, hex, left_dp, right_dp);
}

static HAL_StatusTypeDef Seven_Seg_Write_Integer_DP(Seven_Seg *seven_seg, int8_t val, uint8_t left_dp, uint8_t right_dp) {
  // if positive value, write the last two digits
  if (val >= 0) {
      return Seven_Seg_Write_Hexes(seven_seg, (val / 10) % 10, val % 10, left_dp, right_dp);
  }

  // if negative value, write minus and the last digit
  else {
      char text[2] = { '-', '0' + abs(val) % 10 };
      return Seven_Seg_Write_Chars(seven_seg, text, left_dp, right_dp);
  }
}

HAL_StatusTypeDef Seven_Seg_Write_Integer(Seven_Seg *seven_seg, int8_t val) {
  return Seven_Seg_Write_Integer_DP(seven_seg, val, 0, 0);
}

HAL_StatusTypeDef Seven_Seg_Write_Chars(Seven_Seg *seven_seg, char text[2], uint8_t left_dp, uint8_t right_dp) {
  uint8_t left_raw, right_raw;

  // ensure char is within bounds of normal ASCII
  if (text[0] < ASCII_SKIP || text[0] >= ASCII_MAX) {
      left_raw = ' ';
  }
  else {
      left_raw = LEFT_ASCII[(uint8_t)text[0] - ASCII_SKIP];
  }
  if (text[1] < ASCII_SKIP || text[1] >= ASCII_MAX) {
      right_raw = ' ';
  }
  else {
      right_raw = RIGHT_ASCII[(uint8_t)text[1] - ASCII_SKIP];
  }

  // add decimal point if necessary
  if (left_dp) {
      left_raw |= SEVEN_SEG_LEFT_DP;
  }
  if (right_dp) {
      right_raw |= SEVEN_SEG_RIGHT_DP;
  }

  // write to display
  return Seven_Seg_Write_Raw(seven_seg, left_raw, right_raw);
}

HAL_StatusTypeDef Seven_Seg_Write_Text(Seven_Seg *seven_seg, char text[2]) {
  return Seven_Seg_Write_Chars(seven_seg, text, 0, 0);
}

HAL_StatusTypeDef Seven_Seg_Write_Decimal(Seven_Seg *seven_seg, float val) {
  // display to ones place
  if ( ( 10.0 <= val && val < 100.0 ) || ( -10.0 < val && val <= 0 ) ) {
      return Seven_Seg_Write_Integer_DP(seven_seg, (int8_t)val, 0, 1);
  }

  // display to tenths place
  else if ( 0.0 <= val && val < 10.0 ) {
      return Seven_Seg_Write_Integer_DP(seven_seg, (int8_t)(val * 10), 1, 0);
  }

  else {
      return Seven_Seg_Write_Text(seven_seg, "--");
  }
}
