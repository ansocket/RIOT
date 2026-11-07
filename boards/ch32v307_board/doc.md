@defgroup   boards_ch32v307_board YD-CH32V307VCT6
@ingroup    boards
@brief      Support for the YD-CH32V307VCT6 development board

## Overview

The YD-CH32V307VCT6 development board is based on the WCH CH32V307VCT6
microcontroller in an LQFP100 package. It features:

- A 32-bit QingKe V4F RISC-V core with a single-precision FPU, running at up
  to 144 MHz
- 256 KiB of internal Flash and 64 KiB of SRAM (configurable memory split)
- Two USB Type-C connectors connected to the MCU's USB interfaces
- An RJ45 Ethernet connector using the MCU's internal 10 Mbit/s PHY
- A microSD (TF) card slot
- A W25Q32 SPI Flash (4 MiB) and a 24C64 I2C EEPROM (8 KiB)
- Two user LEDs (PA15 and PB4) and a power LED
- Reset, BOOT0, and USB user buttons, plus a BOOT1 jumper
- 8 MHz and 32.768 kHz crystals
- GPIO expansion headers and a four-pin programming/debug header

![YD-CH32V307VCT6 board and connectors][board-image]

## Hardware

The following table describes the MCU hardware capabilities. Availability in
RIOT depends on the implementation of the corresponding drivers and board
configuration.

| MCU               | CH32V307VCT6                                       |
|:----------------- |:-------------------------------------------------- |
| Vendor            | WCH                                                |
| Core              | QingKe V4F, 32-bit RISC-V with FPU                   |
| Package           | LQFP100                                            |
| RAM               | 64 KiB with the 256 KiB Flash / 64 KiB SRAM split    |
| Flash             | 256 KiB with the 256 KiB Flash / 64 KiB SRAM split   |
| Maximum frequency | 144 MHz                                            |
| GPIOs             | Up to 80                                           |
| Timers            | 10                                                 |
| ADC               | 2 x 12-bit ADC, up to 16 external channels          |
| DAC               | 2 x 12-bit channels                                |
| Serial interfaces | 3 USARTs and 5 UARTs                                |
| SPI / I2S         | 3 SPI interfaces, 2 with I2S support                 |
| I2C               | 2                                                  |
| CAN               | 2 x CAN 2.0B                                       |
| USB               | USB FS OTG and USB HS host/device with built-in PHY |
| Ethernet          | MAC with an internal 10 Mbit/s PHY                  |
| Other interfaces  | SDIO, FSMC, DVP                                    |
| GPIO logic level  | 3.3 V on this board                                |

The Flash/SRAM split is configurable through the device option bytes. The
selected split must match the linker configuration used by the application.
The external W25Q32 Flash and 24C64 EEPROM are separate from internal program
memory and SRAM.

### Board connections

| Function              | MCU pins / connection                         |
|:--------------------- |:--------------------------------------------- |
| User LED 1            | PA15, active high                             |
| User LED 2            | PB4, active high                              |
| USB user button       | PB3, active low                               |
| USB FS connector      | PA11 (D-), PA12 (D+)                           |
| USB HS connector      | PB6 (D+), PB7 (D-)                             |
| WCH two-wire debug    | PA13 (SWDIO), PA14 (SWCLK)                     |
| USART1 (default pins) | PA9 (TX), PA10 (RX)                            |
| SPI Flash             | PA2 (CS), PA5 (SCK), PA6 (MISO), PA7 (MOSI)     |
| I2C EEPROM            | PB10 (SCL), PB11 (SDA)                         |
| Ethernet PHY signals  | PC6 (RXP), PC7 (RXN), PC8 (TXP), PC9 (TXN)     |
| Ethernet LEDs         | PC0 (link), PC1 (activity)                     |

@note   The microSD interface shares PC8 and PC9 with Ethernet. Check the TF
        jumpers before using either interface; both cannot use these pins
        simultaneously. The linked partial schematic documents these
        connections.

## Flashing and debugging

Use an external WCH-LinkE programmer/debugger in RISC-V mode. The board's USB
Type-C connectors connect directly to the MCU and do not provide an onboard
debugger or USB-to-UART bridge.

Connect the programmer to the debug header according to its pin labels:

| WCH-LinkE signal | Board signal |
|:---------------- |:------------ |
| GND              | GND          |
| SWDIO / TMS      | PA13         |
| SWCLK / TCK      | PA14         |

Use 3.3 V target logic levels. If powering the board through USB, follow the
programmer's instructions for its target-voltage connection to avoid
connecting competing power supplies. Keep BOOT0 low for normal execution
from user Flash and reset the board after programming.

WCH-LinkUtility or the WCH programming tools supplied with MounRiver Studio
can program a compatible application image. OpenOCD use requires a version
and configuration supporting both WCH-Link and CH32V307.

## Connecting to serial output

Use an external USB-to-UART adapter with 3.3 V logic levels. For USART1 on its
default pins, connect the adapter's RX to PA9 (MCU TX), TX to PA10 (MCU RX),
and GND to GND.

Once the RIOT UART driver and board UART configuration are implemented for
these pins, open the serial terminal with the adapter's actual device path:

```shell
make BOARD=ch32v307_board -C examples/basic/hello-world PORT=/dev/ttyUSB0 term
```

## References

- [WCH CH32V307 product page][wch-product]
- [WCH CH32V307 documentation and examples][wch-sdk]
- [YD-CH32V307VCT6 partial schematic][board-schematic]
- [YD-CH32V307VCT6 Ethernet example and board notes][board-notes]
- [MounRiver Studio and WCH development tools][mounriver]

[board-image]: https://raw.githubusercontent.com/rtek1000/YD-CH32V307VCT6_ETHERNET-LWIP/main/IMG/YD-CH32V307VCT6-board-720x616.jpg
[board-schematic]: https://raw.githubusercontent.com/rtek1000/YD-CH32V307VCT6_ETHERNET-LWIP/main/IMG/CH32V307VCT6_partial_schematic.png
[board-notes]: https://github.com/rtek1000/YD-CH32V307VCT6_ETHERNET-LWIP
[wch-product]: https://www.wch-ic.com/products/CH32V307.html
[wch-sdk]: https://github.com/openwch/ch32v307
[mounriver]: https://www.mounriver.com/download
