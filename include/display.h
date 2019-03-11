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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

void initDisplay();
void refreshDisplay();
void setDisplayInt (int);
void setDisplayOff (bool val);
void setDisplayStr (const char *str);
void setDisplayTestMode (bool, const char *str);
char *xitoa(int16_t val, char *str, uint8_t len);
void itofpa (int val, char* str, uint8_t pointPosition);

void displayBeep();

#endif
