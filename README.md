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
 - [x] STM32
     - [x] inter-module sync signal
     - [x] a few gpio for controlling external stuff
     - [x] programming connector
     - [x] a couple LEDs and buttons
     - [x] BOOT0 / reset button
 - [x] USB
     - [x] tvs diodes  (USB3300 has integrated TVS diodes)
     - [x] ULPI transciever
           USB3320
         - [x] refclk
               going to wait to start layout to decide which pin will work the best for this.
               STM32 can't generate clock on ULPI bus.
         - [x] gpio vol] i2c selection pins
 - [x] image sensor bus switch
     - [x] muxes
     - [x] camera select signal
 - [x] hm0360
     - [x] i2c selection pins
     - [x] mclk; should we provide an external clock?
           yes.
     - [x] xsleep, xshutdown, etc
     - [x] strobe
     - [x] int
 - [x] hm01b0
     - [x] mclk; should we provide an external clock?
     - [x] strobe
     - [x] int
 - [x] i2c pullups
 - [x] Crystal capacitor selection
 - [ ] check ground vias

#### PCB checklist
 - [x] lens mounting holes
 - [ ] add current sensing?


Bringup todos
 - [ ] Verify power down sequencing