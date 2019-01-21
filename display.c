/*
 * This file is part of the firmware for yogurt maker project
 * (https://github.com/mister-grumbler/yogurt-maker).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Control functions for the seven-segment display (SSD).
 */

#include <stdint.h>

#include "display.h"
#include "stm8s003/gpio.h"

/* Definitions for display */
// Port A controls segments: B, F
// 0000 0110
#define SSD_SEG_BF_PORT     PA_ODR
#define SSD_BF_PORT_MASK    0b00000110
// Port C controls segments: C, G
// 1100 0000
#define SSD_SEG_CG_PORT     PC_ODR
#define SSD_CG_PORT_MASK    0b11000000
// Port D controls segments: A, E, D, P
// 0010 1110
#define SSD_SEG_AEDP_PORT   PD_ODR
#define SSD_AEDP_PORT_MASK  0b00101110

// PD.5
#define SSD_SEG_A_BIT       0x20
// PA.2
#define SSD_SEG_B_BIT       0x04
// PC.7
#define SSD_SEG_C_BIT       0x80
// PD.3
#define SSD_SEG_D_BIT       0x08
// PD.1
#define SSD_SEG_E_BIT       0x02
// PA.1
#define SSD_SEG_F_BIT       0x02
// PC.6
#define SSD_SEG_G_BIT       0x40
// PD.2
#define SSD_SEG_P_BIT       0x04

// Port B controls digits: 1, 2
#define SSD_DIGIT_12_PORT   PB_ODR
// Port D controls digit: 3
#define SSD_DIGIT_3_PORT    PD_ODR

// PB.4
#define SSD_DIGIT_1_BIT     0x10
// PB.5
#define SSD_DIGIT_2_BIT     0x20
// PD.4
#define SSD_DIGIT_3_BIT     0x10



#define bit(n) (1 << (n))


// Global variables

static uint8_t activeSegId;
static uint8_t display[8];

static bool displayOff;
static bool testMode;

/**
 * @brief FontROM. The last element of the table must be '0'.
 * The list of segments as they located on display:
 *  _3_       _2_       _1_
 *  <A>       <A>       <A>
 * F   B     F   B     F   B
 *  <G>       <G>       <G>
 * E   C     E   C     E   C
 *  <D> (P)   <D> (P)   <D> (P)
 *
 */
static const uint8_t font[] = {
    ' ', 0,
    '-', bit(SEG_G),
    '0', bit(SEG_B) | bit(SEG_F) | bit(SEG_C) | bit(SEG_A) | bit(SEG_D) | bit(SEG_E),
    '1', bit(SEG_B) | bit(SEG_C),
    '2', bit(SEG_B) | bit(SEG_G) | bit(SEG_A) | bit(SEG_D) | bit(SEG_E),
    '3', bit(SEG_B) | bit(SEG_C) | bit(SEG_G) | bit(SEG_A) | bit(SEG_D),
    '4', bit(SEG_B) | bit(SEG_C) | bit(SEG_F) | bit(SEG_G),
    '5', bit(SEG_C) | bit(SEG_F) | bit(SEG_G) | bit(SEG_A) | bit(SEG_D),
    '6', bit(SEG_C) | bit(SEG_F) | bit(SEG_G) | bit(SEG_A) | bit(SEG_D) | bit(SEG_E),
    '7', bit(SEG_B) | bit(SEG_C) | bit(SEG_A),
    '8', bit(SEG_B) | bit(SEG_C) | bit(SEG_F) | bit(SEG_G) | bit(SEG_A) | bit(SEG_D) | bit(SEG_E),
    '9', bit(SEG_B) | bit(SEG_C) | bit(SEG_F) | bit(SEG_G) | bit(SEG_A) | bit(SEG_D),
    'A', bit(SEG_B) | bit(SEG_C) | bit(SEG_F) | bit(SEG_G) | bit(SEG_A) | bit(SEG_E),
    'B', bit(SEG_C) | bit(SEG_F) | bit(SEG_G) | bit(SEG_D) | bit(SEG_E),
    'C', bit(SEG_F) | bit(SEG_A) | bit(SEG_D) | bit(SEG_E),
    'D', bit(SEG_B) | bit(SEG_C) | bit(SEG_G) | bit(SEG_D) | bit(SEG_E),
    'E', bit(SEG_F) | bit(SEG_G) | bit(SEG_A) | bit(SEG_D) | bit(SEG_E),
    'F', bit(SEG_F) | bit(SEG_G) | bit(SEG_A) | bit(SEG_E),
    'H', bit(SEG_B) | bit(SEG_C) | bit(SEG_F) | bit(SEG_G) | bit(SEG_E),
    'L', bit(SEG_F) | bit(SEG_D) | bit(SEG_E),
    'N', bit(SEG_B) | bit(SEG_F) | bit(SEG_C) | bit(SEG_A) | bit(SEG_E),
    'O', bit(SEG_B) | bit(SEG_F) | bit(SEG_C) | bit(SEG_A) | bit(SEG_D) | bit(SEG_E),
    'P', bit(SEG_B) | bit(SEG_F) | bit(SEG_G) | bit(SEG_A) | bit(SEG_E),
    'R', bit(SEG_G) | bit(SEG_E),
    'T', bit(SEG_F) | bit(SEG_G) | bit(SEG_D) | bit(SEG_E),
    'c', bit(SEG_G) | bit(SEG_E) | bit(SEG_D),
    'h', bit(SEG_F) | bit(SEG_E) | bit(SEG_G) | bit(SEG_C),
    'o', bit(SEG_G) | bit(SEG_C) | bit(SEG_D) | bit(SEG_E),
    'n', bit(SEG_C) | bit(SEG_G) | bit(SEG_E),
    'r', bit(SEG_G) | bit(SEG_E),
    't', bit(SEG_F) | bit(SEG_G) | bit(SEG_D) | bit(SEG_E),
    0,   0
};

static const uint8_t displaySegment[] = {
    // Port A: id 0, 1
    SSD_SEG_B_BIT, SSD_SEG_F_BIT,
    // Port C: id 2, 3
    SSD_SEG_C_BIT, SSD_SEG_G_BIT,
    // Port D: id 3, 4, ...
    SSD_SEG_A_BIT, SSD_SEG_D_BIT, SSD_SEG_E_BIT, SSD_SEG_P_BIT
};

enum {
    SEG_B, SEG_F,
    SEG_C, SEG_G,
    SEG_A, SEG_D, SEG_E, SEG_P
};

enum {
    DIGIT_1 = 0x10,
    DIGIT_2 = 0x20,
    DIGIT_3 = 0x01 // Placeholder for 0x10, replaced in refresh
};


/**
 * @brief Configure appropriate bits for GPIO ports, initialize static
 *  variables and set test mode for display.
 */
void initDisplay()
{
    PA_DDR |= SSD_SEG_B_BIT | SSD_SEG_F_BIT;
    PA_CR1 |= SSD_SEG_B_BIT | SSD_SEG_F_BIT;
    PB_DDR |= SSD_DIGIT_1_BIT | SSD_DIGIT_2_BIT;
    PB_CR1 |= SSD_DIGIT_1_BIT | SSD_DIGIT_2_BIT;
    PC_DDR |= SSD_SEG_C_BIT | SSD_SEG_G_BIT;
    PC_CR1 |= SSD_SEG_C_BIT | SSD_SEG_G_BIT;
    PD_DDR |= SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT | SSD_SEG_P_BIT | SSD_DIGIT_3_BIT;
    PD_CR1 |= SSD_SEG_A_BIT | SSD_SEG_D_BIT | SSD_SEG_E_BIT | SSD_SEG_P_BIT | SSD_DIGIT_3_BIT;
    displayOff = false;
    activeSegId = 0;
    setDisplayTestMode (true, "");
}

/**
 * @brief
 * Enable the segment with given ID on SSD and remaining of segments are unchanged.
 *
 * @param id
 * The ID = 0 corresponds to the SEG_A to SEG_P enum
 * Accepted values are: SEG_A to SEG_P
 * @param set
 * Enable the segment if true, else disable
 */
static void enableSegment (uint8_t id, bool set)
{
    uint8_t seg = displaySegment[id];
    volatile uint8_t *rdport;

    if (id <= SEG_F)
        rdport = &SSD_SEG_BF_PORT;
    else if (id <= SEG_G)
        rdport = &SSD_SEG_CG_PORT;
    else
        rdport = &SSD_SEG_AEDP_PORT;

    if (set)
        *rdport |= seg;
    else
        *rdport &= ~seg;
}

/**
 * @brief This function is being called during timer's interrupt
 *  request so keep it extremely small and fast. During this call
 *  the data from display's buffer being used to drive appropriate
 *  GPIO pins of microcontroller.
 */
void refreshDisplay()
{
    uint8_t rdport, digits;

    enableSegment (activeSegId, false);

    if (displayOff) {
        return;
    }

    activeSegId = (activeSegId + 1) & 0x7;

    digits = display[activeSegId];

    rdport = SSD_DIGIT_12_PORT & ~(SSD_DIGIT_1_BIT | SSD_DIGIT_2_BIT);
    SSD_DIGIT_12_PORT = rdport | (digits & (SSD_DIGIT_1_BIT | SSD_DIGIT_2_BIT));

    rdport = SSD_DIGIT_3_PORT & ~SSD_DIGIT_3_BIT;
    if (digits & DIGIT_3)
        rdport |= SSD_DIGIT_3_BIT;
    SSD_DIGIT_3_PORT = rdport;

    enableSegment (activeSegId, true);
}

/**
 * @brief Enables/disables a test mode of SSDisplay. While in this mode
 *  the test message will be displayed and any attempts to update
 *  display's buffer will be ignored.
 * @param val
 *  value to be set: true - enable test mode, false - disable test mode.
 */
void setDisplayTestMode (bool val, char* str)
{
    if (!testMode && val) {
        if (*str == 0) {
            setDisplayStr ("888");
        } else {
            setDisplayStr (str);
        }
    }

    testMode = val;
}

/**
 * @brief Enable/disable display.
 * @param val
 *  value to be set: true - display off, false - display on.
 */
void setDisplayOff (bool val)
{
    displayOff = val;
}

/**
 * @brief Sets bits within display's buffer appropriate to given value.
 */
static void paintChar(uint8_t mask, uint8_t id)
{
    int8_t i;

    for (i = 0; i < 8; i++, mask >>= 1)
        if (mask & 1)
            display[i] &= ~id;
        else
            display[i] |= id;
}


/**
 * @brief Sets bits within display's buffer appropriate to given value.
 *  So this symbol will be shown on display during refreshDisplay() call.
 *  When test mode is enabled the display's buffer will not be updated.
 *
 * @param id
 *  Identifier of character's position on display.
 *  Accepted values are: 0, 1, 2.
 * @param val
 *  Character to be represented on SSD at position being designated by id.
 *  Due to limited capabilities of SSD some characters are shown in a very
 *  schematic manner.
 *  Accepted values are: ANY.
 *  But only actual characters are defined. For the rest of values the
 *  '_' symbol is shown.
 * @param dot
 *  Enable dot (decimal point) for the character.
 *  Accepted values true/false.
 *
 */
static void setDigit (uint8_t id, uint8_t val, bool dot)
{
#ifdef RIGHT_ALIGN_TEXT
    static const uint8_t digitVec[] = {DIGIT_1, DIGIT_2, DIGIT_3};
#else
    static const uint8_t digitVec[] = {DIGIT_3, DIGIT_2, DIGIT_1};
#endif
    uint8_t mask;
    const uint8_t *p;

    if (id > 2) return;

    if (testMode) return;

    for (p = font; p[0]; p += 2)
        if (p[0] == val) break;
    mask = p[1];

    if (dot)
        mask |= bit(SEG_P);

    paintChar(mask, digitVec[id]);
}


/**
 * @brief Sets symbols of given null-terminated string into display's buffer.
 * @param val
 *  pointer to the null-terminated string.
 */
void setDisplayStr (const char *val)
{
#ifdef RIGHT_ALIGN_TEXT
    uint8_t i, d;

    // disable the digit if it is not needed.
    for (i = 0; i < 3; i++) {
        setDigit (i, ' ', false);
    }

    // get number of display digit(s) required to show given string.
    for (i = 0, d = 0; val[i]; i++, d++) {
        if (val[i] == '.' && i > 0 && val[i-1] != '.') d--;
    }

    // at this point d = required digits
    // but SSD have 3 digits only. So rest is doesn't matters.
    if (d > 3) {
        d = 3;
    }

    // set values for digits.
    for (i = 0; d != 0 && val[i]; i++, d--) {
        uint8_t c = val[i];
        bool dot = false;

        if (val[i+1] == '.') {
            dot = true;
            i++;
        }
        setDigit (d - 1, c, dot);
    }
#else
    uint8_t i;

    for (i = 0; i < 3; i++) {
        char c = val[0];
        bool d = false;

        if (c == '\0')
            c = ' ';
        else {
            if (val[1] == '.') {
                d = true;
                val++;
            }
            val++;
         }
        setDigit (i, c, d);
    }
#endif
}

/**
 * @brief Construction of a string representation of the given value.
 *  To emulate a floating-point value, a decimal point can be inserted
 *  before a certain digit.
 *  When the decimal point is not needed, set pointPosition to 6 or more.
 * @param val
 *  the value to be processed.
 * @param str
 *  pointer to buffer for constructed string.
 * @param pointPosition
 *  put the decimal point in front of specified digit.
 */
char *xitoa(int16_t val, char *str, uint8_t len)
{
    str += len;
    *str = 0;

    // Converting the value to the substring
    for (; len > 0; len--) {
        *--str = '0' + (val % 10);
        val /= 10;
    }
    return str;
}

/**
 * @brief Construction of a string representation of the given value.
 *  To emulate a floating-point value, a decimal point can be inserted
 *  before a certain digit.
 *  When the decimal point is not needed, set pointPosition to 6 or more.
 * @param val
 *  the value to be processed.
 * @param str
 *  pointer to buffer for constructed string.
 * @param pointPosition
 *  put the decimal point in front of specified digit.
 */
void itofpa (int val, char *str, uint8_t pointPosition)
{
    uint8_t i, l;
    char buffer[6];
    bool minus = false;

    // No calculation is required for zero value
    if (val == 0) {
        str[0] = '0';
        str[1] = 0;
        return;
    }

    // Correction for processing of negative value
    if (val < 0) {
        minus = true;
        val = -val;
    }

    // Forming the reverse string
    for (i = 0; val != 0; i++) {
        buffer[i] = '0' + (val % 10);

        if (i == pointPosition) {
            i++;
            buffer[i] = '.';
        }

        val /= 10;
    }

    // Add leading '0' in case of ".x" result
    if (buffer[i - 1] == '.') {
        buffer[i] = '0';
        i++;
    }

    // Add '-' sign for negative values
    if (minus) {
        buffer[i] = '-';
        i++;
    }

    // Reversing to get the result string
    for (l = i; i > 0; i--) {
        str[l - i] = buffer[i - 1];
    }

    // Put null at the end of string
    str[l] = 0;
}
