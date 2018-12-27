# yogurt-maker
The firmware with extended functionality for W1209 thermostat board that allows to control the yogurt maker device.

This project is modified version of https://github.com/mister-grumbler/w1209-firmware project.

See additional info at https://github.com/mister-grumbler/yogurt-maker/wiki

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
