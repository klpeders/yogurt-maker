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

#include <stdint.h>
#include "stm8s003/prom.h"
#include "persist.h"


/* Definitions for EEPROM */
#define EEPROM_BASE_ADDR        0x4000
#define EEPROM_PARAMS_OFFSET    100

/**
 * @brief Write protect the EEPROM.
 */
static void ee_lock(void)
{
    //  Write protect the EEPROM.
    FLASH_IAPSR &= ~0x08;
}

/**
 * @brief Check if the EEPROM is write-protected.  If it is then unlock the EEPROM.
 */
static void ee_unlock(void)
{
    if ( (FLASH_IAPSR & 0x08) == 0) {
        FLASH_DUKR = 0xAE;
        FLASH_DUKR = 0x56;
    }
}


/**
 * @brief Stores updated parameters from array 'params' to EEPROM.
 */
void ee_storeParams(const ee_persist_t *params)
{
    uint8_t i;
    ee_persist_t *const persistent_params =
            (ee_persist_t *) (EEPROM_BASE_ADDR + EEPROM_PARAMS_OFFSET);

    ee_unlock();

    //  Write to the EEPROM parameters which value is changed.
    for (i = 0; i < SZ_PARAMETER; i++) {
        if (params[i] != persistent_params[i]) {
            persistent_params[i] = params[i];
        }
    }
    ee_lock();
}


/**
 * @brief Load parameters into array 'params' from EEPROM.
 */
void ee_loadParams(ee_persist_t *params)
{
    uint8_t i;
    const ee_persist_t *const persistent_params =
        (const ee_persist_t *) (EEPROM_BASE_ADDR + EEPROM_PARAMS_OFFSET);

    // Load parameters from EEPROM
    for (i = 0; i < SZ_PARAMETER; i++) {
        params[i] =  persistent_params[i];
    }
}

#if 0
/**
 * @brief Write single value to persistent storage
 */
static void ee_writeParam (ee_persist_t val, uint8_t index)
{
    ee_persist_t *const persistent_params =
            (ee_persist_t *) (EEPROM_BASE_ADDR + EEPROM_PARAMS_OFFSET);

    ee_unlock();

    //  Write the data to the EEPROM.
    persistent_params[index] = val;

    ee_lock();
}

#endif
