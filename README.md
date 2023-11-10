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

#### Schematic checklist
 - [ ] STM32
     - [ ] inter-module sync signal
     - [ ] a few gpio for controlling external stuff
     - [x] programming connector
     - [x] a couple LEDs and buttons
     - [ ] BOOT0 / reset button
 - [ ] USB
     - [x] tvs diodes  (USB3300 has integrated TVS diodes)
     - [ ] ULPI transciever
           USB3320
         - [i] refclk
               going to wait to start layout to decide which pin will work the best for this.
               STM32 can't generate clock on ULPI bus.
         - [x] gpio vol] i2c selection pins
     - [ tage levels acceptable?
 - [ ] image sensor bus switch
     - [x] muxes
     - [i] camera select signal
 - [ ] hm0360
     - [x] i2c selection pins
     - [ ] mclk; should we provide an external clock?
     - [i] xsleep, xshutdown, etc
     - [x] strobe
     - [i] int
 - [ ] hm01b0
     - [i] mclk; should we provide an external clock?
     - [i] strobe
     - [i] int
 - [x] i2c pullups

#### PCB checklist
 - [ ] lens mounting holes


Bringup todos
 - [ ] Verify power down sequencing