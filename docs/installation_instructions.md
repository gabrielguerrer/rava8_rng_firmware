# Installation instructions

## Compiling

The RAVA firmware is developed and compiled using
([Visual Studio Code](https://code.visualstudio.com/)) together with the
[PlatformIO](https://platformio.org/) extension.

- [Install VS Code](https://code.visualstudio.com/docs/setup/setup-overview)
- [Install PlatformIO](https://platformio.org/install/ide?install=vscode)
  - On Linux, `python3-venv` is also required
- [Install git](https://git-scm.com/install/)

To build the firmware:
1. Find the [desired version](https://github.com/gabrielguerrer/rava8_rng_firmware/releases)
2. Download the firmware. In the following example, version 3.0.0 is cloned:
   ```
   git clone --branch 3.0.0 --depth 1 https://github.com/gabrielguerrer/rava8_rng_firmware.git
   ```
3. Open VS Code and navigate to: File → Open Folder. Then select the `rava8_rng_firmware` directory
4. When opening the project for the first time, PlatformIO automatically downloads the required
   toolchains and dependencies
5. Click the PlatformIO icon in the VS Code sidebar and select "Build"

After a successful compilation, the generated firmware file can be found at:
`rava8_rng_firmware/.pio/build/ATmega32U4/firmware.hex`

As an alternative to compiling the firmware from source, users can download the precompiled
`firmware/rava8.hex` file provided in this repository. The precompiled firmware includes health 
tests and USB CDC communication support. Users requiring a customized build including USART 
communication or peripheral interfaces can modify the options in `rava8_config.h` and compile the 
firmware themselves.


## Uploading

To upload new firmware through the USB port, the ATmega32U4 bootloader must first be activated.

According to the default fuse configuration used in the RAVA8 circuit
(`lfuse=0xff hfuse=0xd9 efuse=0xf3 lock=0xce`), firmware updates can only be performed through the
bootloader application. Furthermore, the bootloader itself can only be entered through a hardware
reset of the MCU, ensuring that physical access to the device is required for firmware updates.

The hardware reset is triggered by applying a brief 0 V pulse to the `RST` pin. A simple way to
achieve this is by momentarily shorting the `RST` and `GND` pins with a metallic tool while the
device is powered.

If successful, the operating system should detect a new USB device named `ATm32U4DFU`, indicating
that the bootloader is running and ready to receive firmware updates.

If the bootloader is missing or has been corrupted, instructions for reinstalling the bootloader
and restoring the fuse configuration can be found in the
[RAVA8 Device](https://github.com/gabrielguerrer/rava8_rng#documentation) documentation.

The following sections describe how to upload the firmware binary on different operating systems.
All procedures assume that the RAVA device is connected through USB and already running the
bootloader.

### Linux and macOs

Firmware uploading is performed using [AVRDUDE](https://github.com/avrdudes/avrdude/), an
open-source utility for programming AVR microcontrollers.

On **Linux** Debian-based distributions, AVRDUDE can be installed with:
```
sudo apt install avrdude
```
Depending on the distribution, additional permissions may be required to access USB devices. For
example, on Ubuntu, the user needs to be added to the `dialout` group with:
```
sudo usermod -a -G dialout $USER
```
After changing group membership, log out and log back in for the changes to take effect.

On **macOs**, first install [Homebrew](https://brew.sh/), which also installs the Xcode
command-line tools if needed. Then install AVRDUDE with:
```
brew install avrdude
```

Once AVRDUDE is installed, the firmware can be uploaded to a RAVA device running the bootloader
with:
```
avrdude -v -u -c flip1 -p m32u4 -P usb -U flash:w:/path_to_the/firmware.hex
```

It is also possible to upload the firmware directly from VS Code using the PlatformIO extension.
Open the firmware folder in VS Code, click the PlatformIO icon in the sidebar, and select "Upload".

### Windows

Despite multiple attempts, I haven't managed using AVRDUDE with flip1 programmer to upload the RAVA
firmware on Windows. If you know a reliable method, contributions are welcome.

An alternative approach is to use Microchip's
[Flip software](https://www.microchip.com/en-us/development-tool/flip) and its Java GUI interface.
Download and install FLIP (with JRE included if you don't have the Java runtime yet). Next, update
the Atm32U4DFU driver:
1. Open the Windows Device Manager
2. Locate the `ATm32U4DFU` device under "Other Devices"
3. Right-click the device and select "Update Driver"
4. Choose "Browse my computer" and select the directory `C:\Program Files (x86)\Atmel\Flip 3.4.7\usb`
5. Complete the installation

Within the FLIP application, proceed as follows:
1. Click Device → Select → ATMega32u4
2. Open the USB connection: Settings → Communication → USB → Open
3. If the message "AtLibUsbDfu.dll not found" appears, the ATm32U4DFU driver was not installed correctly
4. Load the firmware file: File → Load HEX File, and select the intended `.hex` file
5. Click the "Run" button inside the "Operations Flow" panel

A successful upload is indicated by green status indicators next to the Erase, Blank Check,
Program, and Verify labels.