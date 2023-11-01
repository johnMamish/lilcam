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
 - [ ] STM32
     - [ ] inter-camera sync signal
     - [ ] a few gpio for controlling external stuff
     - [x] programming connector
 - [ ] USB
     - [x] tvs diodes  (USB3300 has integrated TVS diodes)
     - [ ] ULPI transciever
           USB3320
         - [i] refclk
         - [x] gpio voltage levels acceptable?
 - [ ] image sensor bus switch
 - [ ] hm0360
     - [ ] i2c selection pins
     - [ ] mclk; should we provide an external clock?
     - [ ] xsleep, xshutdown, etc
     - [ ] strobe
     - [ ] int
 - [ ] hm01b0
     - [ ] mclk; should we provide an external clock?
     - [ ] strobe
     - [ ] int
 - [x] i2c pullups
 - [ ] lens mounting holes


Bringup todos
 - [ ] Verify power down sequencing