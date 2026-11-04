# WPI ECE 2049 Starter Code

This repository contains the STM32 starter code project for use in the ECE 2049 lab assignments. The starter code integrates the [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) and the [Adafruit ST7735](https://github.com/adafruit/Adafruit-ST7735-Library) libraries which have been ported specifically for the WPI ECE 2049 course. This library is not original to this course, rather it is an adaptation of the official libraries as they are written and published online. These libraries are not official releases nor are they maintained by Adafruit and/or its partners. The WPI ECE department manages the release and updates to this port. The Adafruit GFX library is distributed with the BSD license while the Adafruit ST7735 library is distributed with the MIT license, both of which allow redistribution of their code within projects given the original copyright notice is included. As such, the original BSD license for the Adafruit GFX library is included in the [LICENSE](LICENSE) file while the original MIT license for the Adafruit ST7735 library is included in the [LICENSE_MIT](LICENSE_MIT) file.

## Overview

This starter code should contain everything that you need to get started on any one of the lab assignments for this course. It already has the graphics library set up and configured so that you can immediately begin calling graphics commands. 

It also includes a hardware-based true random number generator for use with random conditions in the labs. This random number generator generates 32-bit random integers between 0 and the 32-bit integer limit (4,294,967,295). It can be called using the function call shown in the example below, where `hrng` is the struct containing the information about the peripheral that is already defined and `randNum` is the variable used to store the random number:

```
uint32_t randNum;

HAL_RNG_GenerateRandomNumber(&hrng, &randNum);
```

## API Reference

The API reference manual for display functions can be found at this link: https://wpi-ece2049-reimagined-mqp.github.io/ECE2049-STM32-Adafruit-Graphics-Library/files.html, which is updated with every update to the graphics library repository. Two different systems, and therefore APIs, are described in this documentation. Graphics functions are described in the Adafruit_ST7735_API.h file while button and backlight functions are described in the Adafruit_TFTShield18_API.h file.
