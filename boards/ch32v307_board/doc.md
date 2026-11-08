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
- Reset and BOOT0 buttons, plus a BOOT1 jumper
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
| TF card detect        | PD7 (TF-SW), active low                        |
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

### SPI

The SPI driver supports 8-bit, MSB-first master transfers in modes 0–3.
Chip select is controlled through GPIO; pass `SPI_CS_UNDEF` to manage it
in the application. DMA and hardware chip select are not implemented.

| RIOT bus     | Peripheral | SCK  | MISO | MOSI | APB clock at 144 MHz |
|:-------------|:-----------|:-----|:-----|:-----|:--------------------|
| `SPI_DEV(0)` | SPI1       | PA5  | PA6  | PA7  | 144 MHz             |
| `SPI_DEV(1)` | SPI2       | PB13 | PB14 | PB15 | 72 MHz              |

SPI1 is shared with the onboard W25Q32 Flash, whose CS is PA2. Initialize
PA2 as an output and keep it high when communicating with another device
on SPI1. SPI3 is not configured because its default PB3/PB4 pins overlap
JTAG and the PB4 user LED. Only default peripheral pin mappings are supported.

The driver selects the fastest APB divider from 2 to 256 that does not
exceed the requested SPI clock. Requests below APB / 256 are unsupported
and trigger an assertion when assertions are enabled. At the default CPU
clock, the available RIOT clock selections produce:

| Requested clock  | SPI1       | SPI2       |
|:-----------------|:-----------|:-----------|
| `SPI_CLK_100KHZ` | unsupported | unsupported |
| `SPI_CLK_400KHZ` | unsupported | 281.25 kHz |
| `SPI_CLK_1MHZ`   | 562.5 kHz  | 562.5 kHz  |
| `SPI_CLK_5MHZ`   | 4.5 MHz    | 4.5 MHz    |
| `SPI_CLK_10MHZ`  | 9 MHz      | 9 MHz      |

The divider uses the configured APB frequency, including when SDIO lowers
the CPU clock to 96 MHz. For 100 kHz operation, the corresponding APB clock
must be at most 25.6 MHz.

Build the interactive test with:

```shell
make BOARD=ch32v307_board -C tests/periph/spi
```

For a loopback test on SPI2, connect PB15 (MOSI) to PB14 (MISO), then use
`init 1 0 2 1 12` and `send hello`. The received bytes should match `hello`.
Repeat with modes 1, 2 and 3 (`init 1 <mode> 2 1 12`). A logic analyzer
can verify SCK polarity, phase, frequency and CS on PB12. SPI1 can instead
access the onboard Flash using `SPI_DEV(0)` and `GPIO_PIN(PORT_A, 2)` as CS.

### SD memory cards over SDIO

The `sdmmc` module supports SDSC, SDHC and SDXC memory cards through the
native SDIO controller. The board configuration uses a 4-bit bus:

| SD signal           | MCU pin |
|:--------------------|:--------|
| CLK                 | PC12    |
| CMD                 | PD2     |
| DAT0 (TF-D0)        | PC8     |
| DAT1 (TF-D1)        | PC9     |
| DAT2                | PC10    |
| DAT3                | PC11    |
| Card detect (TF-SW) | PD7     |

Select the TF card jumpers before using the slot. CMD and DAT0–DAT3 require
external pull-ups to 3.3 V (WCH recommends 47 kOhm). Ethernet cannot use PC8
and PC9 at the same time. For a 1-bit connection, set `dat1`, `dat2` and
`dat3` to `GPIO_UNDEF` in the board configuration.

Applications enable the interface with `USEMODULE += sdmmc` and use the
standard `sdmmc_*` functions. The controller transfers data through its FIFO
with hardware flow control; the SD clock is limited to 4 MHz. DMA and High
Speed mode are not implemented. Buffers do not need special alignment.

When SDIO is enabled, the default CPU/AHB clock is 96 MHz. The SDIO clock is
derived from HCLK with an 8-bit divider, so 144 MHz cannot provide the
400 kHz identification clock. An explicit `CLOCK_CORECLOCK` override must
keep HCLK at or below 102.8 MHz; incompatible values cause a build error.
Applications without SDIO retain the 144 MHz default.

Card detection uses TF-SW on PD7 with an internal pull-up and active-low
polarity: a low level indicates an inserted card. Insertion and removal
generate SDMMC card events. Reinitialize the card after insertion before
accessing it. The detect pin and polarity are configured by `cd`, `cd_mode`
and `cd_active`.

Build the interactive test application with:

```shell
make BOARD=ch32v307_board -C tests/drivers/sdmmc
```

After flashing, use `init`, `cid`, `csd`, `scr`, `size` and `read 0 1` to
check the card. The test's write, copy and erase commands overwrite card data.

### Watchdog

The `periph_wdt` feature uses the independent watchdog (IWDG), clocked by
the internal nominal 40 kHz LSI oscillator. It supports normal reset mode
with `min_time = 0` and timeouts from 1 to 26214 milliseconds. The driver
rounds up to the next watchdog tick; actual reset timing depends on LSI
oscillator accuracy and does not depend on the CPU/APB clock configuration.
Window mode, early warning callbacks and stopping a running IWDG are not
supported. Once started, it must be kicked periodically until reset.

Applications use `wdt_setup_reboot(0, timeout_ms)`, `wdt_start()` and
`wdt_kick()` from `periph/wdt.h`. Setup alone does not start a watchdog
that was disabled. No board pins or peripheral pin configuration are needed.

Build and flash the existing watchdog test:

```shell
make BOARD=ch32v307_board -C tests/periph/wdt all flash
```

In the serial console, run:

```text
range
setup 0 5000
start
kick
```

`kick` restarts the countdown. Without further kicks, the board should
reboot approximately five seconds later. To test a stalled application,
use `setup 0 1000` followed by `startloop` after the next reset.

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
