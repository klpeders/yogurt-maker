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

/**
 * Control functions for buttons.
 * The EXTI2 interrupt (5) is used to get buttons push events.
 * Timer-tick is used to handle anti-bounce, long-press and repeat.
 */

#include <stdint.h>
#include "stm8s003/gpio.h"
#include "buttons.h"
#include "menu.h"




#define ISBUTTON1(n) ((n) & BUTTON1_BIT)
#define ISBUTTON2(n) ((n) & BUTTON2_BIT)
#define ISBUTTON3(n) ((n) & BUTTON3_BIT)

#define BUTTON_T_ANTI_BOUNCE  2 //  2 * 32ms =  64ms
#define BUTTON_T_LONGPRESS   64 // 64 * 32ms =   2s
#define BUTTON_T_REPEAT      34 // 16 * 32ms = 512ms

static uint8_t guard_timer, pending_push;
static uint8_t settings_repeat_keys, settings_repeat_timeout;
static uint8_t settings_long_press;


/**
 * @brief Configure approptiate pins of MCU as digital inputs. Set
 *  initial value of status by getting current state of buttons.
 *  Configure external interrupt for these inputs.
 */
void initButtons()
{
//    PC_DDR &= ~(BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT); // Input
    PC_CR1 |= BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT; // Enable pull-up
    PC_CR2 |= BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT; // External IRQ enable

    EXTI_CR1 |= 0x20;   // generate interrupt on falling edge
}

/**
 * @brief Gets value of buttons status which was when last interrupt
 *  request being handled.
 * @return status byte of buttons
 */
uint8_t getButton()
{
    return ~BUTTONS_PORT & (BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT);
}

/**
 * @brief
 * @return
 */
bool getButton1()
{
    return getButton() & BUTTON1_BIT;
}

/**
 * @brief
 * @return
 */
bool getButton2()
{
    return getButton() & BUTTON2_BIT;
}

/**
 * @brief
 * @return
 */
bool getButton3()
{
    return getButton() & BUTTON3_BIT;
}


/**
 * @brief Resend key press event in 'timeout' ticks (timeout*32ms)
 *     Automatically handle:
 *     <press>
 *       call retrigger(t_repeat)
 *       <32 ticks(1sec)>
 *     <press>
 *       call retrigger(t_repeat)
 *       <t_repeat ticks>
 *     <press>
 *       call retrigger(t_repeat)
 *       ...
 * timeout: 1 - 32, for 32ms to 1024ms per repeat
 */
void buttonRetrigger(uint8_t keys, uint8_t timeout)
{
    settings_repeat_keys = keys;
    settings_repeat_timeout = BUTTON_T_REPEAT - timeout;
}


/**
 * @brief Request LONGPRESS from specified button.
 *     A long-press enabled button doesn't send release events
 *     PUSH event is send on release.
 *     LONGPRESS event send after T_LONGPRESS ticks.
 */
void buttonEnableLongPress(uint8_t keys)
{
    settings_long_press = keys;
}


/**
 * @brief Helper to send events to the menu state machine
 */
static uint8_t handleButtonEvent(uint8_t event_base, uint8_t buttons)
{
    uint8_t event = 0, status = 0;

    // Send appropriate event to menu.
    if ( (status = ISBUTTON1(buttons)) ) {
        event = event_base + 0;
    }
    else if ( (status = ISBUTTON2(buttons)) ) {
        event = event_base + 1;
    }
    else if ( (status = ISBUTTON3(buttons)) ) {
        event = event_base + 2;
    }
    if (event) {
        feedMenu (event);
    }
    return status;
}


/**
 * @brief Callback for checking button actions
 */
void refreshButtons()
{
    uint8_t event;
    uint8_t status;
    uint8_t pressed, released;

    if ( guard_timer == 0 )
        return;

    if ( guard_timer == 1 && (event = pending_push & ~settings_long_press) )  {

        // Filter out PUSH on long_press enabled keys:
        //   LONGPRESS is send after BUTTON_T_LONGPRESS timeout
        //   PUSH is send when a long_press enabled key is released
        //   only keys that emitted LONGPRESS event send release event
        // Otherwise Send PUSH event to menu

        status = handleButtonEvent(MENU_EVENT_PUSH_BUTTON1, pending_push & ~settings_long_press);
        pending_push &= ~status;
        return;
    }

    // Wait at least ANTI_BOUNCE ticks after last button press
    if ( guard_timer < BUTTON_T_ANTI_BOUNCE ) {
        guard_timer++;
        return;
    }


    // If key have been held for 'LONGPRESS' ticks, send LONGPRESS event.
    if ( guard_timer == BUTTON_T_LONGPRESS && (event = (pending_push & settings_long_press)) )  {

        // Send LONGPRESS event

        status = handleButtonEvent(MENU_EVENT_LONGPRESS_BUTTON1, event);
        pending_push &= ~status;
        return;
    }


    // If REPEAT is enabled send released event is key is still pressed

    pressed = ~PC_CR2 & (BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT);
    if ( guard_timer == BUTTON_T_REPEAT && (event = (pressed & settings_repeat_keys)) ) {

        guard_timer = settings_repeat_timeout;
        settings_repeat_keys = 0;

        // Send PUSH event to menu for keys where retrigger was requested

        status = handleButtonEvent(MENU_EVENT_PUSH_BUTTON1, event);
    }

    // Increment guard-timer.
    if (guard_timer != 255) guard_timer++;


    // Handle button RELEASE. Send PUSH on release of LONGPRESS enabled button

    released = BUTTONS_PORT & ~PC_CR2 & (BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT);
    status = 0;
    if ( (event = (released & ~pending_push)) ) {

        // Send RELEASE event to menu when key is released.

        status = handleButtonEvent(MENU_EVENT_RELEASE_BUTTON1, event);
    }
    else if ( (event = (released & pending_push)) ) {

        // Send PUSH event to menu when long_press key is released.

        status = handleButtonEvent(MENU_EVENT_PUSH_BUTTON1, event);
        pending_push &= ~status;
    }

    PC_CR2 |= status;

    if ((~PC_CR2 & (BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT)) == 0) {
        guard_timer = 0;
    }
}


/**
 * @brief This function is button's interrupt request handler
 *
 */
void EXTI2_handler() __interrupt (5)
{
    uint8_t buttons = ~BUTTONS_PORT & (BUTTON1_BIT | BUTTON2_BIT | BUTTON3_BIT);

    // save new pending button, disable IRQ for active buttons and enable timer
    pending_push |= PC_CR2 & buttons;
    PC_CR2 &= ~buttons;
    guard_timer = 1;
}

