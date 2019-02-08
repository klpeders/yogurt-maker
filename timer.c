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
 * Control functions for timer.
 * The TIM4 interrupt (23) is used to get signal on update event.
 */

#include <stdint.h>
#include <string.h>

#include "timer.h"
#include "stm8s003/clock.h"
#include "stm8s003/timer.h"
#include "adc.h"
#include "display.h"
#include "params.h"
#include "menu.h"
#include "relay.h"

#define TICKS_IN_SECOND     500
#define BITS_FOR_TICKS      9
#define BITS_FOR_SECONDS    6
#define BITS_FOR_MINUTES    6
#define BITS_FOR_HOURS      5
#define BITS_FOR_DAYS       6
#define SECONDS_FIRST_BIT   (BITS_FOR_TICKS + 1)
#define MINUTES_FIRST_BIT   (BITS_FOR_TICKS + BITS_FOR_SECONDS + 1)
#define HOURS_FIRST_BIT     (BITS_FOR_TICKS + BITS_FOR_SECONDS + BITS_FOR_MINUTES + 1)
#define DAYS_FIRST_BIT      (BITS_FOR_TICKS + BITS_FOR_SECONDS + BITS_FOR_MINUTES + BITS_FOR_HOURS + 1)
#define BITMASK(L)          ( ~ (0xFFFFFFFF << (L) ) )
#define NBITMASK(L)         (0xFFFFFFFF << (L) )

/**
 * Uptime counter
 * |--Day--|--Hour--|--Minute--|--Second--|--Ticks--|
 * 31      26       21         15         9         0
 */
static uint32_t uptime;
/**
 * |--Hour--|--Minute--|
 * 11       6          0
 */
static uint16_t fTimer;
static uint8_t fTimerSeconds;


/**
 * @brief Initialize timer's configuration registers and reset uptime.
 */
void initTimer()
{
    CLK_CKDIVR = 0x00;  // Set the frequency to 16 MHz
    TIM4_PSCR = 0x07;   // CLK / 128 = 125KHz
    TIM4_ARR = 0xF5;    // 125KHz /  250(0xFA) = 500Hz
    TIM4_IER = 0x01;    // Enable interrupt on update event
    TIM4_CR1 = 0x05;    // Enable timer
    resetUptime();
    fTimer = 0;
}

/**
 * @brief Starts fermentation timer.
 */
void startFTimer()
{
    fTimer = ( (getParamById (PARAM_FERMENTATION_TIME) - 1) << BITS_FOR_MINUTES) + 59;
    fTimerSeconds = getUptimeSeconds();
}

/**
 * @brief Stops fermentation timer.
 */
void stopFTimer()
{
    fTimer = 0;
}

/**
 * @brief Gets minutes part of the fermentation timer current value.
 * @return number of minutes remaining until end of that hour.
 */
uint8_t getFTimerMinutes()
{
    return (uint8_t) (fTimer & BITMASK (BITS_FOR_MINUTES) );
}

/**
 * @brief Gets hours part of the fermentation timer current value.
 * @return number of hours remaining.
 */
uint8_t getFTimerHours()
{
    return (uint8_t) (fTimer >> BITS_FOR_MINUTES);
}

/**
 * @brief Checks fermentation timer to be active.
 * @return True if fermentation timer is active.
 */
bool isFTimer()
{
    return fTimer != 0;
}

/**
 * @brief Sets value of uptime counter to zero.
 */
void resetUptime()
{
    uptime = 0;
}

/**
 * @brief Gets raw value of bit-mapped uptime counter.
 * |--Day--|--Hour--|--Minute--|--Second--|--Ticks--|
 * 31      26       21         15         9         0
 * @return value of uptime counter.
 */
uint32_t getUptime()
{
    return uptime;
}

/**
 * @brief Gets ticks part of uptime counter.
 * @return ticks part of uptime.
 */
uint16_t getUptimeTicks()
{
    return (uint16_t) (uptime & BITMASK (BITS_FOR_TICKS) );
}

/**
 * @brief Gets seconds part of time being passed since last reset.
 * @return seconds part of uptime.
 */
uint8_t getUptimeSeconds()
{
    return (uint8_t) ( (uptime >> SECONDS_FIRST_BIT) & BITMASK (BITS_FOR_SECONDS) );
}

#ifdef CONFIG_ENABLE_FULL_UPTIME
/**
 * @brief Gets minutes part of time being passed since last reset.
 * @return minutes part of uptime.
 */
uint8_t getUptimeMinutes()
{
    return (uint8_t) ( (uptime >> MINUTES_FIRST_BIT) & BITMASK (BITS_FOR_MINUTES) );
}

/**
 * @brief Gets hours part of time being passed since last reset.
 * @return hours part of uptime.
 */
uint8_t getUptimeHours()
{
    return (uint8_t) ( (uptime >> HOURS_FIRST_BIT) & BITMASK (BITS_FOR_HOURS) );
}

/**
 * @brief Gets amount of days being passed since last reset.
 * @return amount of days.
 */
uint8_t getUptimeDays()
{
    return (uint8_t) ( (uptime >> DAYS_FIRST_BIT) & BITMASK (BITS_FOR_DAYS) );
}
#endif

/**
 * @brief Constructs string that represents current uptime using given format.
 * @param strBuff
 *  A pointer to a string buffer where the result should be placed.
 * @param format
 *  Day - D | d, Hour - H | h, Minute - M | m, Second - S | s.
 * In place of capital leter the actual value will be shown even if this
 * value is zero. The small leter will be replaced by actual value only
 * if this value is non-zero.
 *  Due to the limited display size, only the "." character is allowed
 * as a separator.
 * Example: "dd.hH.MM" for 00 days, 10 hours and 02 minutes will produce
 * ".10.02" result.
 */
void uptimeToString (char *strBuff, const char *format)
{
    uint8_t i, j, v;
    char f[3];

    for (i = 0; format[i] != 0; i++) {
        switch (format[i]) {
#ifdef CONFIG_ENABLE_FULL_UPTIME
        case 'd':
        case 'D':
            v = getUptimeDays();
            j = 1;

            if (format[i + 1] == 'D') {
                j++;
                i++;
            }

            break;

        case 'h':
        case 'H':
            v = getUptimeHours();
            j = 1;

            if (format[i + 1] == 'H') {
                j++;
                i++;
            }

            break;

        case 'm':
        case 'M':
            v = getUptimeMinutes();
            j = 1;

            if (format[i + 1] == 'M') {
                j++;
                i++;
            }

            break;

        case 's':
        case 'S':
            v = getUptimeSeconds();
            j = 1;

            if (format[i + 1] == 'S') {
                j++;
                i++;
            }

            break;
#endif
        case 't':
            v = getFTimerMinutes();
            j = 1;

            if (format[i + 1] == 't') {
                j++;
                i++;
            }

            break;

        case 'T':
            v = getFTimerHours();
            j = 1;

            if (format[i + 1] == 'T') {
                j++;
                i++;
            }

            break;

        default:
            f[0] = format[i];
            f[1] = 0;
            strcat (strBuff, f);
            continue;
        }

        xitoa(v, f, j);

        // Append substring at the end of the resulting string
        strcat (strBuff, f);
    }

}

/**
 * @brief This function is timer's interrupt request handler
 * so keep it small and fast as much as possible.
 */
void TIM4_UPD_handler() __interrupt (23)
{
    TIM4_SR &= ~TIM_SR1_UIF; // Reset flag

    if ( ( (uint16_t) (uptime & BITMASK (BITS_FOR_TICKS) ) ) >= TICKS_IN_SECOND) {
        uptime &= NBITMASK (SECONDS_FIRST_BIT);
        uptime += (unsigned long) 1 << SECONDS_FIRST_BIT;

#ifdef CONFIG_ENABLE_FULL_UPTIME
        // Increment minutes count when 60 seconds have passed.
        if ( ( (uint8_t) (uptime >> SECONDS_FIRST_BIT) & BITMASK (BITS_FOR_SECONDS) ) == 60) {
            uptime &= NBITMASK (MINUTES_FIRST_BIT);
            uptime += (unsigned long) 1 << MINUTES_FIRST_BIT;
        }

        // Increment hours count when 60 minutes have passed.
        if ( ( (uint8_t) (uptime >> MINUTES_FIRST_BIT) & BITMASK (BITS_FOR_MINUTES) ) == 60) {
            uptime &= NBITMASK (HOURS_FIRST_BIT);
            uptime += (unsigned long) 1 << HOURS_FIRST_BIT;
        }

        // Increment days count when 24 hours have passed.
        if ( ( (uint8_t) (uptime >> HOURS_FIRST_BIT) & BITMASK (BITS_FOR_HOURS) ) == 24) {
            uptime &= NBITMASK (DAYS_FIRST_BIT);
            uptime += (unsigned long) 1 << DAYS_FIRST_BIT;
        }
#endif
        // Decrement fermentation timer value.
        if (isFTimer() && fTimerSeconds == getUptimeSeconds() ) {
            if (getFTimerMinutes() > 0) {
                fTimer--;

                // Disable the relay functionality when the fermentation timer is exhausted.
                if (fTimer == 0) {
                    enableRelay (false);
                }
            } else {
                fTimer = ( (getFTimerHours() - 1) << BITS_FOR_MINUTES) + 59;
            }
        }
    }

    uptime++;

    // Try not to call all refresh functions at once.
    buzzRelay ();

    if ( ( (uint8_t) getUptimeTicks() & 0x0F) == 1) {
        refreshMenu();
    } else if ( ( (uint8_t) getUptimeTicks() & 0xFF) == 2) {
        startADC();
    } else if ( ( (uint8_t) getUptimeTicks() & 0xFF) == 3) {
        refreshRelay();
    }

    refreshDisplay();
}
