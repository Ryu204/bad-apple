# Description

Bad apple played on 8051 microcontroller study kit, powered by a computer server.

* MCU: **STC89C52**
* Kit: **8051 Pro-KIT** (Schematic in `img` folder)

The project is divided into 2 components: the firmware on the controller and the software on the computer.

The server will process video footage (including camera capture) and send data to the controller to process.

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

Then, build the server:

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