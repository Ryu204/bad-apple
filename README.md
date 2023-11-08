# Description

Bad apple played on 8051 microcontroller study kit, powered by a computer server.

* MCU: **STC89C52**
* Kit: **8051 Pro-KIT** (Schematic in `img` folder)

The project is divided into 2 components: the firmware on the controller and the software on the computer.

The server will process video footage (including camera capture) and send data to the controller to process.

# Installation

## Requirements

* `SDCC` (Small Device C Compiler)
* `CMake` version 3.23 or higher
* C++ compiler capable of `c++20` standard

### Dependencies

* [OpenCV](https://github.com/opencv/opencv)
* [SFML](https://github.com/SFML/SFML)
* [CSerialPort](https://github.com/itas109/CSerialPort)

## Build

It's possible to generate a binary of the firmware. However please note it's only compatible with the specific hardware as shown in `img` folder.

Build the firmware:

```bash
# Currently in bad-apple folder
cmake --preset=base -DCMAKE_C_COMPILER=sdcc --DCMAKE_SYSTEM_NAME=Generic
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

# Usage

You can try calling the executable and see what parameters must be passed. That part is horrible though. After a long time fiddling around I found some good example:

```bash
# Currently in bad-apple/src/svr
sudo build/bad-apple-server /dev/ttyUSB0 19200 0.2 0.4 0.7 0.9 -frsc/bad-apple.mp4
sudo build/bad-apple-server /dev/ttyUSB0 19200 0.2 0.4 0.7 0.9 -frsc/fire.mp4
sudo build/bad-apple-server /dev/ttyUSB0 19200 0.2 0.4 0.6 0.85 -c
```

## Result

Coming soon...
