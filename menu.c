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
#define MENU_AUTOINC_DELAY  MENU_1_SEC_PASSED / 4

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
 * @brief check for timeout in the state machine.
 */
static void checkTimeout()
{
    if (timer > MENU_5_SEC_PASSED) {
        setParamId (0);
        if (menuState != MENU_ROOT)
            storeParams();
        menuState = menuDisplay = MENU_ROOT;
        setDisplayOff (false);
        timer = 0;
    }
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

        buttonEnableLongPress(BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT);

        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            setParamId (PARAM_FERMENTATION_TIME);
            menuState = menuDisplay = MENU_SET_TIMER;
            timer = 0;
            break;

        case MENU_EVENT_LONGPRESS_BUTTON1:
            setParamId (0);
            menuState = menuDisplay = MENU_SELECT_PARAM;
            timer = 0;
            break;

        case MENU_EVENT_LONGPRESS_BUTTON2:    // Enable/Disable thermostat
            if ( !isFTimer() ) {
                if ( isRelayEnabled() ) {
                    enableRelay (false);
                }
                else {
                    enableRelay (true);
                }
            }
            timer = 0;
            break;

        case MENU_EVENT_LONGPRESS_BUTTON3: // Start/Stop fermentation timer
            if (isFTimer() ) {
                stopFTimer();
                enableRelay (false);
            }
            else {
                startFTimer();
                enableRelay (true);
            }
            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            checkTimeout ();

        default:
            break;
        }
    } else if (menuState == MENU_SELECT_PARAM) {

        buttonEnableLongPress(0);

        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            menuState = menuDisplay = MENU_CHANGE_PARAM;
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON2:
            incParamId();
            buttonRetrigger(BUTTON2_BIT, MENU_AUTOINC_DELAY);
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON3:
            decParamId();
            buttonRetrigger(BUTTON3_BIT, MENU_AUTOINC_DELAY);
            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            checkTimeout ();

        default:
            break;
        }
    } else if (menuState == MENU_CHANGE_PARAM) {

        buttonEnableLongPress(0);

        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            menuState = menuDisplay = MENU_SELECT_PARAM;
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON2:
            incParam();
            buttonRetrigger(BUTTON2_BIT, MENU_AUTOINC_DELAY);
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON3:
            decParam();
            buttonRetrigger(BUTTON3_BIT, MENU_AUTOINC_DELAY);
            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            checkTimeout();

        default:
            break;
        }
    } else if (menuState == MENU_SET_TIMER) {

        buttonEnableLongPress(0);

        switch (event) {
        case MENU_EVENT_PUSH_BUTTON1:
            menuState = menuDisplay = MENU_ROOT;
            storeParams();
            setDisplayOff (false);
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON2:
            incParam();
            buttonRetrigger(BUTTON2_BIT, MENU_AUTOINC_DELAY);
            timer = 0;
            break;

        case MENU_EVENT_PUSH_BUTTON3:
            decParam();
            buttonRetrigger(BUTTON3_BIT, MENU_AUTOINC_DELAY);
            timer = 0;
            break;

        case MENU_EVENT_CHECK_TIMER:
            if ( getButton2() || getButton3() ) {
                blink = false;
            } else {
                blink = (bool) ( (uint8_t) getUptimeTicks() & 0x80);
            }
            setDisplayOff (blink);
            checkTimeout();

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
