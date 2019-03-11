# yogurt-maker

The firmware with extended functionality for W1209 thermostat board that allows to control the yogurt maker device.

This project is modified version of https://github.com/mister-grumbler/w1209-firmware project.

See additional info at https://github.com/mister-grumbler/yogurt-maker/wiki

# Usage

The 3 keys on the W1209 is as follows.

* Key 1 is used *Set* / *Menu*
* Key 2 and 3 is *Up* and *Down* respectively.

## Quick start

1. Hold key 2 and key 3 while powering on to restore defaults
1. Press key 1, then use key 2 and key 3 to change the fermentation time.
1. Press key 1 again to start the timer.
1. Or from the main display, hold key 3 to start the timer.


## Details

The device have 3 states, ROOT, SET TIMER, PARAMETER SELECT, PARAMETER CHANGE

```
ROOT -> SET_TIMER, when key 1 pressed
ROOT -> PARAMETER_SELECT, when key 1 long-press
ROOT -> TIMER_RUNNING, when key 3 long-press

SET_TIMER -> self (inc time), when key 2 pressed
SET_TIMER -> self (dec time), when key 3 pressed
SET_TIMER -> TIMER_RUNNING, when key 1 pressed
SET_TIMER -> ROOT, when time_out(5 secs)

PARAMETER_SELECT -> self (inc parameter), when key 2 pressed
PARAMETER_SELECT -> self (dec parameter), when key 3 pressed
PARAMETER_SELECT -> ROOT, when time_out(5 secs)
PARAMETER_SELECT -> PARAMETER_CHANGE, when key 1 pressed

PARAMETER_CHANGE -> self (inc parameter), when key 2 pressed
PARAMETER_CHANGE -> self (dec parameter), when key 3 pressed
PARAMETER_CHANGE -> PARAMETER_SELECT, when key 1 pressed
PARAMETER_CHANGE -> ROOT, when time_out(5 secs)

TIMER_RUNNING -> self
TIMER_RUNNING -> TIMER_FINISHED, when FT time-out

TIMER_FINISHED -> self (beep)
TIMER_FINISHED -> ROOT, when any key pressed
```


The list of application parameters with default values:

 Name|Def| Description
 :--:|:--:|---------------------------------------------
 P0  | C | Cooling/Heating
 ||         (relay ON when temperature is over(C)/below(H) threshold value)
 P1  | 2 | 0.1 ... 15.0 - Hysteresis
 P2  | 50| 30 ... 70 - Maximum allowed temperature value
 P3  | 20| 10 ... 45 Minimum allowed temperature value
 P4  | 0 | 7.0 ... -7.0 Correction of temperature value
 P5  | 0 | 0 ... 10 Relay switching delay in minutes
 P6  |Off| On/Off Indication of overheating
 P7  | 44| 30.0 ... 55.0 Threshold value in degrees of Celsius
 FT  | 8h| 1h ... 15h Fermentation time in hours
[Parameters]


# Flashing

First make sure the flash is open:

```bash
stm8flash -c stlinkv2 -p STM8S003F3P6 -u
Determine OPT area
Due to its file extension (or lack thereof), "Workaround" is considered as RAW BINARY format!
Unlocked device. Option bytes reset to default state.
Bytes written: 11
```

Now the device can be flashed with new firmware:

```bash
stm8flash -c stlinkv2 -p STM8S003F3P6  -w Build/yogurtmaker.ihx
Determine FLASH area
Due to its file extension (or lack thereof), "Build/yogurtmaker.ihx" is considered as INTEL HEX format!
6729 bytes at 0x8000... OK
Bytes written: 6729
```


Hardware Modifications
======================

Replace the Relay with a Triac
------------------------------

###Power supply###


###Triac###



Buzzer
------

A timer really need a buzzer to alert when the brew is finished. Unfortunately
all the GPIO's are used, so the only option is to reuse one.

###Multiplex keys###

One of the most wasteful aspects of this design is the 'keyboard', with very
little effort the keys could be added to the display multiplex.

With one GPIO up to 8 keys could be supported. Without additional components
(diodes) there will be display corruption when multiple keys are pressed.
Another option would be to blank the display when a key is pressed.

###Analog keys###

Use a AD input and a resistor divider chain. Simple. Complicated SW. Require additional
components, and doesn't readily support multiple pressed keys.


These options all require difficult brain surgery. What other options are available?


###Unused display segment###

A trick often used in digital alarm clocks is to a unused segment to drive the
speaker. A digital clock will display 12:34, the : is always on, and the 
10-hour display can only show ' ', '1', '2' that leaves segment F unused.

So many optimized designs use 7 segment drivers and 4 display drivers and
connect the speaker between segF and disp0. Keyboard is implemented with
keys between display drives and a input only pin.

On the W1209 segment P on display 1 isn't used (or don't have to be used), making
it a good candidate.

```
   _3_       _2_       _1_
   <A>       <A>       <A>
  F   B     F   B     F   B
   <G>       <G>       <G>
  E   C     E   C     E   C
   <D> (P)   <D> (P)   <D> (P)
```
[W1209 Segments]


###Use NTC GPIO as speaker###

The main issue multiplexing with the speaker with a digital signal is that
any switch will generate a sound. The NTC input is a slowly changing input
so this would be a safe place to multiplex the speaker.

Obviously the speaker would have to use a coupling capacitor to avoid interfering
with the measurement.

Benefit is that when brew is over, we don't really need the temperature (heat is off)
so we are free to switch the input to output and play our tune.

In the w1209 this doesn't work directly because of the large decoupling capacitor
on the NTC GPIO.


###Selecting a solution###

The absolutely simplest solution on the w1209 is to connect the speaker between
segment P and display 0 lines.

GPIO |Function | CPU|7SEG|Notes
:---:|:-------:|---:|---:|:---------
PA.1 | SEG_F   |  5 | 10 | -
PA.2 | SEG_B   |  6 |  7 | -
PC.6 | SEG_G   | 16 |  5 | HS
PC.7 | SEG_C   | 17 |  4 | HS
PD.1 | SEG_E   | 18 |  1 | HS (SWIM)
PD.2 | SEG_P   | 19 |  3 | HS (Speaker)
PD.3 | SEG_D   | 20 |  2 | HS
PD.5 | SEG_A   |  2 | 11 | HS (TX)
PB.4 | DIGIT_1 | 12 |  8 | OD (Speaker)
PB.5 | DIGIT_2 | 11 |  9 | OD
PD.4 | DIGIT_3 |  1 | 12 | HS


