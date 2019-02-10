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
#include "adc.h"
#include "buttons.h"
#include "display.h"
#include "menu.h"
#include "params.h"
#include "relay.h"
#include "timer.h"

#ifndef INTERRUPT_ENABLE
#define INTERRUPT_ENABLE()    do {__asm rim __endasm; } while(0)
#endif

#ifndef INTERRUPT_DISABLE
#define INTERRUPT_DISABLE()   do {__asm sim __endasm; } while(0)
#endif

#ifndef WAIT_FOR_INTERRUPT
#define WAIT_FOR_INTERRUPT()  do {__asm wfi __endasm; } while(0)
#endif


/**
 * @brief
 */
int main()
{
    static char stringBuffer[7];
    static char paramMsg[] = {'P', '0', 0};
    bool reset_once = true;

    initMenu();
    initButtons();
    initParamsEEPROM(false);
    initDisplay();
    initADC();
    initRelay();
    initTimer();

    INTERRUPT_ENABLE();

    setDisplayTestMode (false, "");
    setDisplayStr ("rdy");

    // Loop
    while (true) {

        switch ( getMenuDisplay() ) {

        case MENU_INIT:
            if (getButton2() && getButton3() && reset_once) {
                initParamsEEPROM(true);
                setDisplayStr ("RST");
                reset_once = false;
            }
            break;

        case MENU_ROOT:
            // Alternately show values for temperature and fermentation timer
            // if it is running.
            if (isRelayEnabled() && getUptimeSeconds() & 0x08) {
                stringBuffer[0] = 0;

                if (isFTimer() ) {
                    // Making blink the dot in between the hours and minutes.
                    if ( (getUptimeTicks() & 0x100) ) {
                        uptimeToString ( stringBuffer, "Ttt");
                    } else {
                        uptimeToString ( stringBuffer, "T.tt");
                    }
                } else {
                    // Show "n.t.r." -> no timer is running
                    setDisplayStr ("n.t.r.");
                    continue;
                }

                setDisplayStr ( stringBuffer);
            } else {
                int temp = getTemperature();
                itofpa (temp, stringBuffer, 0);
                setDisplayStr ( stringBuffer);

                if (getParamById (PARAM_OVERHEAT_INDICATION) ) {
                    if (temp < getParamById (PARAM_MIN_TEMPERATURE) ) {
                        setDisplayStr ("LLL");
                    } else if (temp > getParamById (PARAM_MAX_TEMPERATURE) ) {
                        setDisplayStr ("HHH");
                    }
                }
            }
            break;

        case MENU_SET_TIMER:
             paramToString (PARAM_FERMENTATION_TIME, stringBuffer);
             setDisplayStr ( stringBuffer);
             break;

        case MENU_SELECT_PARAM:
              paramMsg[1] = '0' + getParamId();
              setDisplayStr ( paramMsg);
              break;

        case MENU_CHANGE_PARAM:
              paramToString (getParamId(), stringBuffer);
              setDisplayStr ( stringBuffer);
              break;

        default:
              setDisplayStr ("ERR");
              setDisplayOff ( (bool) ( (uint8_t) getUptimeTicks() & 0x80) );
        }

        WAIT_FOR_INTERRUPT();
    };
}
