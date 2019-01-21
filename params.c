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
 * Control functions for EEPROM storage of persistent application parameters.
 *
 * The list of aplication parameters with default values:
 * Name |Def| Description
 * -----+---+---------------------------------------------
 * P0 - | C | Cooling/Heating
 *            (relay ON when temperature is over(C)/below(H) threshold value)
 * P1 - | 2 | 0.1 ... 15.0 - Hysteresis
 * P2 - | 50| 30 ... 70 - Maximum allowed temperature value
 * P3 - | 20| 10 ... 45 Minimum allowed temperature value
 * P4 - | 0 | 7.0 ... -7.0 Correction of temperature value
 * P5 - | 0 | 0 ... 10 Relay switching delay in minutes
 * P6 - |Off| On/Off Indication of overheating
 * P7 - | 44| 30.0 ... 55.0 Threshold value in degrees of Celsius
 * FT - | 8h| 1h ... 15h Fermentation time in hours
 */

#include <stdint.h>

#include "params.h"
#include "buttons.h"
#include "display.h"
#include "persist.h"

#define N_PARAMETERS   7 // Number of P0 .. Pn parameters

#define MAGIC          0x4E48
#define MagicId        (N_PARAMETERS + 1)

static uint8_t paramId;
static int paramCache[SZ_PARAMETER];

/* Parameter Formattings */
#define DISPLAY_NUM            0
#define DISPLAY_NUM_FRACT_1    0    /* 12.3 */
#define DISPLAY_NUM_FRACT_2    1    /* 1.23 */
#define DISPLAY_NUM_INT        6    /* 123  */
#define DISPLAY_STR_NC_NO    -(0+1) /* if false: "NC" else "NO" */
#define DISPLAY_STR_OFF_ON   -(7+1) /* if false: "OFF" else "ON" */
#define DISPLAY_STR_NONE     -(14+1)/* "---" */

static const char displayString[] =
    " NC| NO"  //  0
    "OFF| ON"  //  7
    "---|---"  //  7
;              // +7

static const int paramMin[] =     {0,   1,  300,  100, -70,  0, 0, 300,     0,  1};
static const int paramMax[] =     {1, 150,  700,  450,  70, 10, 1, 550,     0, 15};
static const int paramDefault[] = {0,  20,  500,  200,   0,  0, 0, 440, MAGIC,  8};
static const uint8_t paramInc[] = {1,   1,   10,   10,   1,  1, 1,   5,     0,  1};
static const int8_t paramDisplay[] = {
    [PARAM_RELAY_MODE]             = DISPLAY_STR_NC_NO,
    [PARAM_RELAY_HYSTERESIS]       = DISPLAY_NUM_FRACT_1,
    [PARAM_MAX_TEMPERATURE]        = DISPLAY_NUM_FRACT_1,
    [PARAM_MIN_TEMPERATURE]        = DISPLAY_NUM_FRACT_1,
    [PARAM_TEMPERATURE_CORRECTION] = DISPLAY_NUM_FRACT_1,
    [PARAM_RELAY_DELAY]            = DISPLAY_NUM_INT,
    [PARAM_OVERHEAT_INDICATION]    = DISPLAY_STR_OFF_ON,
    [PARAM_THRESHOLD]              = DISPLAY_NUM_FRACT_1,
    [MagicId]                      = DISPLAY_STR_NONE,
    [PARAM_FERMENTATION_TIME]      = DISPLAY_NUM_INT
};


/**
 * @brief Stores updated parameters in paramCache to EEPROM.
 */
void storeParams(void)
{
    ee_storeParams(paramCache);
}

/**
 * @brief Check values in the EEPROM to be correct then load them into
 * parameters' cache.
 */
void initParamsEEPROM()
{
    uint8_t i;

    ee_loadParams (paramCache);

    if (paramCache[MagicId] != MAGIC
        || getButton2() && getButton3() ) {

        // Restore parameters to default values
        for (i = 0; i < SZ_PARAMETER; i++) {
            paramCache[i] = paramDefault[i];
        }

        storeParams();
    }

    paramId = 0;
}

/**
 * @brief
 * @param id
 * @return
 */
int getParamById (uint8_t id)
{
    if (id < SZ_PARAMETER) {
        return paramCache[id];
    }

    return -1;
}

/**
 * @brief
 * @param id
 * @param val
 */
void setParamById (uint8_t id, int val)
{
    if (id < SZ_PARAMETER) {
        paramCache[id] = val;
    }
}

/**
 * @brief
 * @return
 */
int getParam()
{
    return paramCache[paramId];
}

/**
 * @brief
 * @param val
 */
void setParam (int val)
{
    paramCache[paramId] = val;
}

/**
 * @brief Incrementing the value of the currently selected parameter.
 */
void incParam()
{
    uint8_t i = paramId;
    int v = paramCache[i] + paramInc[i];

    /* Check if id is a switch style parameter */
    if (paramDisplay[i] < 0) {
        paramCache[i] ^= 0x0001;
    }
    else if (v <= paramMax[i]) {
        paramCache[i] = v;
    }
}

/**
 * @brief Decrementing the value of the currently selected parameter.
 */
void decParam()
{
    uint8_t i = paramId;
    int v = paramCache[i] - paramInc[i];

    /* Check if id is a switch style parameter */
    if (paramDisplay[i] < 0) {
        paramCache[i] ^= 0x0001;
    }
    else if (v >= paramMin[i]) {
        paramCache[i] = v;
    }
}

/**
 * @brief
 * @return
 */
uint8_t getParamId()
{
    return paramId;
}

/**
 * @brief
 * @param val
 */
void setParamId (uint8_t val)
{
    if (val < SZ_PARAMETER) {
        paramId = val;
    }
}

/**
 * @brief
 */
void incParamId()
{
    if (paramId < N_PARAMETERS) {
        paramId++;
    } else {
        paramId = 0;
    }
}

/**
 * @brief
 */
void decParamId()
{
    if (paramId > 0) {
        paramId--;
    } else {
        paramId = N_PARAMETERS;
    }
}

/**
 * @brief Converts the current value of the selected parameter to a string.
 * @param id
 *  The identifier of the parameter to be processed.
 * @param strBuff
 *  A pointer to a string buffer where the result should be placed.
 */
void paramToString (uint8_t id, char *strBuff)
{
    int value;
    int8_t  format;

    if (id >= SZ_PARAMETER)
        id = MagicId;

    value = paramCache[id];
    format = paramDisplay[id];

    if (format >= DISPLAY_NUM) {
        itofpa (value, strBuff, format);
    }
    else {
        format = (value & 1)*4 - format - 1;
        strBuff[0] = displayString[format];
        strBuff[1] = displayString[format+1];
        strBuff[2] = displayString[format+2];
        strBuff[3] = 0;
    }
}

