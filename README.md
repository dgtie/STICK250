# HID Bootloader Firmware for STICK250 (PIC32MX250F128D)
<img style="float: right;" src="https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEhwi5ezDo73WuniMKqsYkIs551BFWiUmgmUbFy6ECa61eeGfr9_IWMujwp4PZQe9I8vc6m5fnBRJFGfXdZcbtFQlHvUgvQI6QyB6erLidGnredK7NUXovNIMFPnAgBopkmU0ToYB7bd_pboKBh6zISEbt10VzlJuOOAheYhE-JBQyHv6VDZDqvv8JU8/s320/program.png"  >
- based on Microchip Application Note AN1388
- for microcontroller PIC32MX250F128D
- 8k bytes in size, leave 112k to user application
- development board [STICK250](https://lamsworkshop.blogspot.com/2023/01/stick250-pic32mx250f128d-experiment.html)

![](https://github.com/dgtie/dgtie.github.io/blob/main/stick250.jpg?raw=true)

# Operation
When power up or reset, it reads the first instruction at user application startup code. If the value is 0xFFFFFFFF, it jumps to bootloader. Otherwise it checks the user button (B8). If button state is LOW, it jumps to bootloader. Otherwise it jumps to user application.

# udev rule
```
# AN1388 HID Bootloader
ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="003c", MODE="0666", ENV{ID_MM_DEVICE_IGNORE}="1", ENV{ID_MM_PORT_IGNORE}="1"
```

# Configuration Registers
User application should not change configuration registers. The bootloader has the registers as follow:

|Configuration Register|Value|
|-|-|
|DEVCFG3|0x8FFFFFFF|
|DEVCFG2|0xFFF979D9|
|DEVCFG1|0xFF74CDDB|
|DEVCFG0|0x7FFFFFFB|

# Directory Structure
- **root**: bootloader source code
- **prog**: programmer application source code
- **blink**: example user application (just blinks the on board LED)
- **usbuart**: sample application (makes STICK250 a USB to UART bridge)
- **pk2**: sample application (makes STICK250 a pickit 2 clone, 3.3 volts only)
- **stk500v2**: sample application (makes STICK250 an AVR programmer, 3.3 volts)

# Development Environment
- a PC with Ubuntu or a Raspberry Pi with Raspbian
- pic32-gcc compiler installed through PlatformIO CLI
- a programmer such as pickit 2 clone

# Memory Layout
||START|END|
|-|-|-|
|bootloader startup code| 0xBFC00000 | 0xBFC003FF |
|bootloader exception code| 0x9D01E000 | 0x9D01EFFF |
|bootloader main code| 0x9D01C000 | 0x9D01DFFF |
|user application startup code| 0xBFC00400 | 0xBFC007FF |
|user application exception code| 0x9D01F000 | 0x9D01FFFF |
|user application main code| 0x9D000000 | 0x9D01BFFF |

# Things learned
- EBASE needs to be 4K aligned

