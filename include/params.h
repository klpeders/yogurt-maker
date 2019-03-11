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

#ifndef PARAMS_H
#define PARAMS_H

#include <stdint.h>
#include <stdbool.h>


#define PARAMETERS(PARAM)                                                           \
    /*     PARAMETER NAME                MIN  MAX  DEFA STEP  FORMAT CODE        */ \
    PARAM(PARAM_RELAY_MODE,                0,   1,    0,   1, DISPLAY_STR_NC_NO  ), \
    PARAM(PARAM_RELAY_HYSTERESIS,          1, 150,   20,   1, DISPLAY_NUM_FRACT_1), \
    PARAM(PARAM_MAX_TEMPERATURE,         300, 700,  500,  10, DISPLAY_NUM_FRACT_1), \
    PARAM(PARAM_MIN_TEMPERATURE,         100, 450,  200,  10, DISPLAY_NUM_FRACT_1), \
    PARAM(PARAM_TEMPERATURE_CORRECTION,  -70,  70,    0,   1, DISPLAY_NUM_FRACT_1), \
    PARAM(PARAM_RELAY_DELAY,               0,  10,    0,   1, DISPLAY_NUM_INT    ), \
    PARAM(PARAM_OVERHEAT_INDICATION,       0,   1,    0,   1, DISPLAY_STR_OFF_ON ), \
    PARAM(PARAM_THRESHOLD,               300, 550,  440,   5, DISPLAY_NUM_FRACT_1), \
    /* Parameters from magic_id and up is not available in parameter selection:  */ \
    PARAM(PARAM_MAGIC_ID,                  0,   0, PARAM_MAGIC_VERSION,  0, DISPLAY_STR_NONE   ), \
    PARAM(PARAM_FERMENTATION_TIME,         1,  15,    8,   1, DISPLAY_NUM_INT    ), \


/* enumerate the parameters */
enum {
#define PARAM_COUNT(name, ARGS...) name
    PARAMETERS(PARAM_COUNT)
    N_PARAMETERS
};

/* Parameter version magic number, increment when making changes */
#define PARAM_MAGIC_VERSION          (0x4E48 + N_PARAMETERS)

void initParamsEEPROM(bool restore);
void storeParams();

void setParamId (uint8_t);
uint8_t getParamId();
void incParamId();
void decParamId();

int getParamById (uint8_t);
void setParamById (uint8_t, int);
void incParam();
void decParam();

void paramToString (uint8_t, char*);

#endif
