#### Electrical
 - [ ] voltage divider going into op-amp for power supply sequencing has incorrect
       value on top. Should be < ~7.5kOhm
 - [ ] ULPI transciever might benefit from a reset line connected to MCU. This
 - [ ] Silkscreen around buttons
 - [ ] No boot0 button, but extra user button (?)
 - [ ] Mounting holes for lens mount too small
 - [ ] Two of the broken out pins should be a DFU-capable
 - [-] Sometimes camera doesnt start on initial power-on, but needs a reset cycle. Why?
       This was because the hm01b0 pixclk being configured as gated. The STM32 has an undocumented requirement that the DCMI pixclk be continuous
 - [ ] Change clock refsel bits on PHY
 - [ ] Add some sort of 'board id' resistors so that firmware can tell which board its on

#### Electrical tentative
 - [ ] Also break out FS usb port?
 - [ ] Add current monitoring for camera power rails
 - [ ] Swap MCU to chip with more flash?
 - [ ] External RAM (buffer images, fun)
 - [ ] External Flash (?)
