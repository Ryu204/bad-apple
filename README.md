# Description

Bad apple played on 8051 microcontroller study kit, powered by a computer server.

* MCU: **STC89C52**
* Kit: **8051 Pro-KIT** (Schematic in `img` folder)

The project is divided into 2 components: the firmware on the controller and the software on the computer.

The server will process video footage (including camera capture) and send data to the controller to process.

# Mechanism

## Shift register

This KIT uses 74HC595 shift register to control a 8x8 LED matrix. Input pins are clock, latch and serial data. The register outputs 8 bits periodically. This gift demonstrates how this shift register works:

![](https://github.com/Ryu204/bad-apple/blob/main/img/nguyen-ly-hoat-dong-74HC595.gif)

(source in image)

At each clock's pulse, corresponding bit value in data pin will be pushed into an 8-bit queue. Once the latch pin is set, the register uses new value from the queue.

## Scanning

In order to display an image on the matrix, we need to scan and set LED status on each row continously. This is my approach:
1. Set data bit to **1**
2. Latch, now the register outputs **0b10000000**, only first row can be lit
3. Set data bit to **0**
4. Update values in the first row, effectively render it
5. Latch, register outputs **0b01000000**
6. Continue updating values in second row
7. Repeat from 5. until all rows are rendered

The project also features PWM, please refer to source code or [Result](#result) section.

## UART

Media data is processed using the code in `src/svr` (will be refered to as "server"). The server compresses each frame into a 8x8 image with 4-level greyscale. Then, the data is sent to the microcontroller.

Here, I used a primal hand-shaking protocol:

1. Server on, starts processing and waits for signals from MCU
2. MCU sends signal `uint8_t 0xFF` to server, requests a whole frame
3. Server receives signal, waits for row signal
4. MCU sends signal `uint8_t 0xFE` to  server, request a row in that frame
5. Server receives signal, sends 8 `uint8_t`-s to MCU
6. MCU receives data for a row, requests next row until all rows are cached into a buffer
7. MCU requests the next frame

Why don't receive the whole frame at once, you may ask. Because there is a problem about interrupts.

## Interrupts

I used 2 types of interrupt: **timer** and **external**.

Timer to steadily call the display function. Remember, to make the frame visible we have to draw it multiple times a second.

External to handle UART communication.

Interrupt priority is the problem here. Communication with server requires precise and steady timer to ensure consistent baudrate. Hence, display must have lower priority. However, if we don't call display function regularly enough, the frame will happen to be dimmed. 

I did my best to balance these two sides. My method is to separate a frame into multiple rows, and call display directly after each row gets its data (normally, display is called by timer interrupt).

# Installation

## Requirements

* `SDCC` (Small Device C Compiler) version `4.2` or higher
* `CMake` version 3.23 or higher
* C++ compiler capable of `c++20` standard

### Dependencies

* [OpenCV](https://github.com/opencv/opencv)
* [SFML](https://github.com/SFML/SFML)
* [CSerialPort](https://github.com/itas109/CSerialPort)
* glad - an OpenGL loader (files included in source code)

## Build

It's recommended you get familiar with CMake build process.

### Firmware

It's possible to generate a binary of the firmware. However please note it's only compatible with the specific hardware as shown in `img` folder.

Build the firmware:

On Windows most likely CMake will default the generator to Visual Studio, which will disregard the sdcc usage. If that's the case you must use something like MinGW Makefiles or Ninja.

```bash
# Currently in bad-apple folder
cmake --preset=base -DCMAKE_C_COMPILER=sdcc -DCMAKE_SYSTEM_NAME=Generic # -G"MinGW Makefiles" or -G"Ninja"
cmake --build build
```

The binary is `build/bad-apple.hex`.

Depending on whether or not your CMake packages can be located via environment variables, you may have to add SDCC's include folder manually. One way to achieve that is to use CMake configure presets.

### Server

To build the server, first you must build and install 3 dependencies above in your local machine.

```bash
# Move to server folder
cd src/svr
cmake --preset=base
cmake --build build
```

You may have to provide environment variables to CMake, such as `SFML_DIR` or so.

Once both firmware and server are built, flash the firmware into the microcontroller using external tools. [stcgal](https://github.com/grigorig/stcgal) did the job for me.

**Important:** In order for the firmware to behave correctly, you must enable **6T double-speed** mode of the microcontroller via the flasher tool.

# Usage

You can try calling the executable and see what parameters must be passed. That part is horrible though. Some things you would like to note:
* The configurations of `timer1` in source code were calculated so the perfect baud rate is `19200 bps`.
* Brightness thresholds must be 4 numbers between 0 and 1 in increasing order.
* The last parameter can be either `-c` (open camera) or `-f<file_name>` (open video).

After a long time fiddling around I found some good examples:

```bash
# Currently in bad-apple/src/svr
sudo build/bad-apple-server /dev/ttyUSB0 19200 0.2 0.4 0.7 0.9 -frsc/bad-apple.mp4
sudo build/bad-apple-server /dev/ttyUSB0 19200 0.2 0.4 0.7 0.9 -frsc/fire.mp4
sudo build/bad-apple-server /dev/ttyUSB0 19200 0.2 0.4 0.6 0.85 -c
```

## Result
[![Video](https://i.imgur.com/Ik74nzT.png)](https://youtu.be/O3pgFUubkxc)


## Known limitations
* If the video source is too small, the texture on preview window won't be created correctly.
* Software's source code is a mix of ketchup and spaghetti, should not be further inspected in any case.
