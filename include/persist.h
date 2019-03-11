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

#ifndef PERSIST_H
#define PERSIST_H

#include <stdint.h>

/* Define size of persistent storage */
#define SZ_PARAMETER 10  /* Size of parameter structure */

typedef int ee_persist_t ; /* Parameter type */

/**
 * @brief Stores updated parameters from array 'params' to EEPROM.
 */
void ee_storeParams(const ee_persist_t *params);


/**
 * @brief Load parameters into array 'params' from EEPROM.
 */
void ee_loadParams(ee_persist_t *params);


/**
 * @brief Write single value to persistent storage
 * @param val
 * @param index
 */
//void ee_writeParam (ee_persist_t val, uint8_t index);

#endif
