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

#include "adc.h"
#include "buttons.h"
#include "display.h"
#include "menu.h"
#include "params.h"
#include "relay.h"
#include "timer.h"

#define INTERRUPT_ENABLE    __asm rim __endasm;
#define INTERRUPT_DISABLE   __asm sim __endasm;
#define WAIT_FOR_INTERRUPT  __asm wfi __endasm;

void strConcat (unsigned char * from, unsigned char * to)
{
    unsigned char i, s;

    for (i = 0; to[i] != 0; i++);

    s = i;

    for (i = 0; from[i] != 0; i++) {
        to[s + i] = from[i];
    }

    to[s + i] = 0;
}

/**
 * @brief
 */
int main()
{
    static unsigned char* stringBuffer[7];
    static unsigned char* timerBuffer[5];
    unsigned char paramMsg[] = {'P', '0', 0};

    initMenu();
    initButtons();
    initParamsEEPROM();
    initDisplay();
    initADC();
    initRelay();
    initTimer();

    INTERRUPT_ENABLE

    // Loop
    while (true) {
        if (getUptimeSeconds() > 0) {
            setDisplayTestMode (false, "");
        }

        if (getMenuDisplay() == MENU_ROOT) {
//            if (getUptimeSeconds() & 0x08) {
//                int temp = getTemperature();
//                itofpa (temp, (char*) stringBuffer, 0);
//                setDisplayStr ( (char*) stringBuffer);
//
//                if (getParamById (PARAM_OVERHEAT_INDICATION) ) {
//                    if (temp < getParamById (PARAM_MIN_TEMPERATURE) ) {
//                        setDisplayStr ("LLL");
//                    } else if (temp > getParamById (PARAM_MAX_TEMPERATURE) ) {
//                        setDisplayStr ("HHH");
//                    }
//                }
//            } else {
            itofpa (getUptimeHours(), (char*) stringBuffer, 6);
            itofpa (getUptimeMinutes(), (char*) timerBuffer, 6);
            strConcat ( (char*) timerBuffer, (char*) stringBuffer);
            setDisplayStr ( (char*) stringBuffer);
            setDisplayDot (0, (getUptimeSeconds() & 1) );
//            }
        } else if (getMenuDisplay() == MENU_SET_THRESHOLD) {
            paramToString (PARAM_THRESHOLD, (char*) stringBuffer);
            setDisplayStr ( (char*) stringBuffer);
        } else if (getMenuDisplay() == MENU_SELECT_PARAM) {
            paramMsg[1] = '0' + getParamId();
            setDisplayStr ( (unsigned char*) &paramMsg);
        } else if (getMenuDisplay() == MENU_CHANGE_PARAM) {
            paramToString (getParamId(), (char*) stringBuffer);
            setDisplayStr ( (char *) stringBuffer);
        } else {
            setDisplayStr ("ERR");
            setDisplayOff ( (bool) ( (unsigned char) getUptimeTicks() & 0x80) );
        }

        WAIT_FOR_INTERRUPT
    };
}
