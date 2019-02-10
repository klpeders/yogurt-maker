/*
 * This file is part of the W1209 firmware replacement project
 * (https://github.com/mister-grumbler/w1209-firmware).
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

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

/* Definition for buttons */

// Port C control input from buttons.
#define BUTTONS_PORT   PC_IDR

#define BUTTON1_BIT    0x08 // PC.3
#define BUTTON2_BIT    0x10 // PC.4
#define BUTTON3_BIT    0x20 // PC.5


/* Prototypes */

void initButtons();

uint8_t getButton();

bool getButton1();
bool getButton2();
bool getButton3();

void buttonRetrigger(uint8_t keys, uint8_t timeout);

void buttonEnableLongPress(uint8_t keys);

void refreshButtons(void);

void EXTI2_handler() __interrupt (5);

#endif
