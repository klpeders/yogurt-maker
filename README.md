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
1. Hold key 3 to start the timer. 


## Details

The device have 3 states, ROOT, SET TIMER, PARAMETER SELECT, PARAMETER CHANGE

```
ROOT -> SET_TIMER, when key 1 pressed
ROOT -> PARAMETER_SELECT, when key 1 held for 3 secs
ROOT -> . (start timer), when key 3 held for 3 secs

SET_TIMER -> . (inc time), when key 2 pressed
SET_TIMER -> . (dec time), when key 3 pressed
SET_TIMER -> ROOT, when key 1 pressed
SET_TIMER -> ROOT, when time_out(5 secs)

PARAMETER_SELECT -> .(inc parameter), when key 2 pressed
PARAMETER_SELECT -> .(dec parameter), when key 3 pressed
PARAMETER_SELECT -> ROOT, when time_out(5 secs)
PARAMETER_SELECT -> PARAMETER_CHANGE, when key 1 pressed

PARAMETER_CHANGE -> . (inc time), when key 2 pressed
PARAMETER_CHANGE -> . (dec time), when key 3 pressed
PARAMETER_CHANGE -> PARAMETER_SELECT, when key 1 pressed
PARAMETER_CHANGE -> ROOT, when time_out(5 secs)
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
