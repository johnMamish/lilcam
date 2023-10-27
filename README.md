Power rails needed:
 - 3v3
    - STM32 VDDUSB   Note - must be sequenced properly.
                     Enough to have an 'enable' pin from the microcontroller plus an
 - 2v8
    - STM32 VDD      Note that I can expect it to consume ~100mA
    - HM01B0 IOVDD
    - HM01B0 AVDD
    - HM0360 IOVDD
    - HM0360 AVDD
 - 1v5
    - HM01B0 DVDD
 - 1v2
    - HM0360 DVDD

Schematic checklist
 - [ ] lens mounting holes
 - [ ] inter-camera sync signal
 - [ ] a few gpio for controlling external stuff
 - [ ] tvs diodes
 - [ ] i2c pullups
 - [ ] programming connector


Bringup todos
 - [ ] Verify power down sequencing
