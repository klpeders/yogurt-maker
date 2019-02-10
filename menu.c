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
 * Implementation of application menu.
 */

#include "menu.h"
#include "buttons.h"
#include "display.h"
#include "params.h"
#include "timer.h"
#include "relay.h"

#define MENU_1_SEC_PASSED   32
#define MENU_3_SEC_PASSED   MENU_1_SEC_PASSED * 3
#define MENU_5_SEC_PASSED   MENU_1_SEC_PASSED * 5
#define MENU_AUTOINC_DELAY  MENU_1_SEC_PASSED / 8

static uint8_t menuDisplay;
static uint8_t menuState;
/* Timer counter of menu. Being incremented on every call of refreshMenu() function.
 * Used to handle menu timeouts and handling of actions on holding a button. */
static unsigned int timer;

/**
 * @brief Initialization of local variables.
 */
void initMenu()
{
    timer = 0;
    menuState = menuDisplay = MENU_ROOT;
}

/**
 * @brief Gets menu state for displaying appropriate value on the SSD.
 * @return
 */
uint8_t getMenuDisplay()
{
    return menuDisplay;
}

/**
 * @brief Updating state of application's menu and displaying info when new
 *  event is received. Possible states of menu and displaying are:
 *  MENU_ROOT
 *  MENU_SELECT_PARAM
 *  MENU_CHANGE_PARAM
 *  MENU_SET_TIMER
 *
 * @param event is one of:
 *  MENU_EVENT_PUSH_BUTTON1
 *  MENU_EVENT_PUSH_BUTTON2
 *  MENU_EVENT_PUSH_BUTTON3
 *  MENU_EVENT_RELEASE_BUTTON1
 *  MENU_EVENT_RELEASE_BUTTON2
 *  MENU_EVENT_RELEASE_BUTTON3
 *  MENU_EVENT_LONGPRESS_BUTTON1
 *  MENU_EVENT_LONGPRESS_BUTTON2
 *  MENU_EVENT_LONGPRESS_BUTTON3
 *  MENU_EVENT_CHECK_TIMER
 */
void feedMenu (uint8_t event)
{
    bool blink;

    if (menuState == MENU_ROOT) {
        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            timer = 0;
            menuDisplay = MENU_SET_TIMER;
            break;

        case MENU_EVENT_RELEASE_BUTTON1:
            if (timer < MENU_5_SEC_PASSED) {
                menuState = MENU_SET_TIMER;
            }

            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            if (timer > MENU_3_SEC_PASSED) {
                timer = 0;

                if (getButton1() ) {
                    setParamId (0);
                    menuState = menuDisplay = MENU_SELECT_PARAM;
                } else {
                    if (getButton2() ) {    // Enable/Disable thermostat
                        if (isRelayEnabled() && !isFTimer() ) {
                            enableRelay (false);
                        } else {
                            enableRelay (true);
                        }
                    } else if (getButton3() ) { // Start/Stop fermentation timer
                        if (isFTimer() ) {
                            stopFTimer();
                            enableRelay (false);
                        } else {
                            startFTimer();
                            enableRelay (true);
                        }
                    }
                }

            }

            break;

        default:
            if (timer > MENU_5_SEC_PASSED) {
                timer = 0;
                menuState = menuDisplay = MENU_ROOT;
            }

            break;
        }
    } else if (menuState == MENU_SELECT_PARAM) {
        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            menuState = menuDisplay = MENU_CHANGE_PARAM;

        case MENU_EVENT_RELEASE_BUTTON1:
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON2:
            incParamId();

        case MENU_EVENT_RELEASE_BUTTON2:
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON3:
            decParamId();

        case MENU_EVENT_RELEASE_BUTTON3:
            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            if (timer > MENU_1_SEC_PASSED + MENU_AUTOINC_DELAY) {
                if (getButton2() ) {
                    incParamId();
                    timer = MENU_1_SEC_PASSED;
                } else if (getButton3() ) {
                    decParamId();
                    timer = MENU_1_SEC_PASSED;
                }
            }

            if (timer > MENU_5_SEC_PASSED) {
                timer = 0;
                setParamId (0);
                storeParams();
                menuState = menuDisplay = MENU_ROOT;
            }

            break;

        default:
            break;
        }
    } else if (menuState == MENU_CHANGE_PARAM) {
        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            menuState = menuDisplay = MENU_SELECT_PARAM;

        case MENU_EVENT_RELEASE_BUTTON1:
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON2:
            incParam();

        case MENU_EVENT_RELEASE_BUTTON2:
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON3:
            decParam();

        case MENU_EVENT_RELEASE_BUTTON3:
            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            if (timer > MENU_1_SEC_PASSED + MENU_AUTOINC_DELAY) {
                if (getButton2() ) {
                    incParam();
                    timer = MENU_1_SEC_PASSED;
                } else if (getButton3() ) {
                    decParam();
                    timer = MENU_1_SEC_PASSED;
                }
            }

            if (timer > MENU_5_SEC_PASSED) {
                timer = 0;
                storeParams();
                menuState = menuDisplay = MENU_ROOT;
            }

            break;

        default:
            break;
        }
    } else if (menuState == MENU_SET_TIMER) {
        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            timer = 0;
            menuDisplay = MENU_ROOT;
            setDisplayOff (false);
            break;

        case MENU_EVENT_RELEASE_BUTTON1:
            if (timer < MENU_5_SEC_PASSED) {
                storeParams();
                menuState = MENU_ROOT;
                setDisplayOff (false);
            }

            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON2:
            setParamId (PARAM_FERMENTATION_TIME);
            incParam();

        case MENU_EVENT_RELEASE_BUTTON2:
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON3:
            setParamId (PARAM_FERMENTATION_TIME);
            decParam();

        case MENU_EVENT_RELEASE_BUTTON3:
            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            if (getButton2() || getButton3() ) {
                blink = false;
            } else {
                blink = (bool) ( (uint8_t) getUptimeTicks() & 0x80);
            }

            if (timer > MENU_1_SEC_PASSED + MENU_AUTOINC_DELAY) {
                setParamId (PARAM_FERMENTATION_TIME);

                if (getButton2() ) {
                    incParam();
                    timer = MENU_1_SEC_PASSED;
                } else if (getButton3() ) {
                    decParam();
                    timer = MENU_1_SEC_PASSED;
                }
            }

            setDisplayOff (blink);

            if (timer > MENU_5_SEC_PASSED) {
                timer = 0;

                if (getButton1() ) {
                    menuState = menuDisplay = MENU_SELECT_PARAM;
                    setDisplayOff (false);
                    break;
                }

                storeParams();
                menuState = menuDisplay = MENU_ROOT;
                setDisplayOff (false);
            }

            break;

        default:
            break;
        }
    }
}

/**
 * @brief This function is being called during timer's interrupt
 *  request so keep it extremely small and fast.
 *  During this call all time-related functionality of application
 *  menu is handled. For example: fast value change while holding
 *  a button, return to root menu when no action is received from
 *  user within a given time.
 */
void refreshMenu()
{
    timer++;
    feedMenu (MENU_EVENT_CHECK_TIMER);
}
