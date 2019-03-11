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

#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

/* Menu sections */
#define MENU_INIT          0
#define MENU_ROOT          1
#define MENU_SELECT_PARAM  2
#define MENU_CHANGE_PARAM  3
#define MENU_SET_TIMER     4
#define MENU_TIMER_RUNNING 5
#define MENU_TIMER_FINISHED 6

/* Menu events */
#define MENU_EVENT_PUSH_BUTTON1     1
#define MENU_EVENT_PUSH_BUTTON2     2
#define MENU_EVENT_PUSH_BUTTON3     3

#define MENU_EVENT_RELEASE_BUTTON1  4
#define MENU_EVENT_RELEASE_BUTTON2  5
#define MENU_EVENT_RELEASE_BUTTON3  6

#define MENU_EVENT_LONGPRESS_BUTTON1 7
#define MENU_EVENT_LONGPRESS_BUTTON2 8
#define MENU_EVENT_LONGPRESS_BUTTON3 9

#define MENU_EVENT_CHECK_TIMER      10

void initMenu();
void refreshMenu();
uint8_t getMenuDisplay();
void feedMenu (uint8_t event);

#endif
