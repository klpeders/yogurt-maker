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

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

void initTimer();
void startFTimer();
void stopFTimer();
void resetUptime();
bool isFTimer();
uint32_t getUptime();
uint16_t getUptimeTicks();
uint8_t getUptimeSeconds();
uint8_t getUptimeMinutes();
uint8_t getUptimeHours();
uint8_t getUptimeDays();
void uptimeToString (char*, const char*);
void TIM4_UPD_handler() __interrupt (23);

void enableBeep(uint8_t set);

#endif
