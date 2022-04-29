
Firmware for automated Section and Rate control with the data coming from AgOpenGPS. Has support for a monochrome display to show realtime Rate application data.


# Features
* Completely new firmware. Uses a multi-threaded aproach with clear ownership of data and almost no public
  accessible variables. Tries to use other libraries as much as possible and is clearly structured with meaningful names.
* Cool and confortable WebUI, creates automatically a hotspot on first start, a Wifi to connect to can be configured in the WebUI
* Everything is configured from the WebUI, no conditional compiling and changing of the source code needed. Sane defaults are used: everything has to
  be activated to work, like PWM-drivers
* Settings are stored into flash on user request. New features normaly require an apply&reboot cycle to activate, the WebUI clearly shows this.
* The IP address is static, no searching for current WebUI address.
* Automatic or manual section and rate control is supported. Use GPIO input to switch between manual and automatic.
* Has an AgOpenGPS icon on startup screen. Thanks @Vili https://discourse.agopengps.com/t/i-need-a-aog-icon-in-uncompressed-bmp-format-for-a-display/8783/14?u=thegtknerd

# Caveats
* As this software is fearly new, not so much testing is done. If you find bug, please open an issue on github.
* **As the WebUI is quite taxing on the Websocket implementation; the ESP32 can crash without warning if left connected too long. Without open connection to a browser,
  no such crashes are documented. On my hardware, a longtime stresstest of more the five days of uptime was completed successfully, if the tab with
  the WebUI is closed after using it.** The cause of the crashes is in the implementation of the used TCP-stack/Websocket-API. Not much can be done about it,
  as it is the default implementation which comes with framework (ESPAsyncWebServer), is really fast/performant and is used by the library to generate the WebUI.
* The width of the sections need to be updated in AgOpenGPS and the WebUI, as AgOpenGPS does not (yet) send the section widths over UDP.
* Land application and fluid application data is collected and calulated over 30 seconds for better accuracy. This makes for slow response time of valves to avoid overapplication/underapplication.

# Schematics

![Schematics](doc/schema.png)

The configuration has to match the connections for the IBT2 (PWM, enable) and the GPIO switches. This schematics is with ESP32DevKitC modules from Espressif, but you
can use other brands, if you adjust the firmware for it. This is usualy done by changing the `board` in `platformio.ini`.  The recommended PCB is https://github.com/AOG-addendum/PCB-modules/tree/master/Section%20control, use the latest version for best results.

An example of a itemlist is below:

Amount | Id | Supplier | Description
--- | ---- | --------- | ----------------------------------------------------------------------------
1x  | 3405 | Espressif | ESP32 DevKitC Board (with headers)

## Alternations:
* Use other modules, as long as the chips on it are the same.

> Please consider buying quality components by genuine resellers to support the development of new cool stuff. Make sure you get the right module, cheap knock offs sometimes have other, less precise/powerfull but compatible chips on them, like the ads1015 on the ads1115 module.

## Windows
Follow this [guide](http://iot-bits.com/esp32/esp32-flash-download-tool-tutorial/), but enter the files/addresses as below:

Address   | File
--------- | ----------------------
`0x1000`  | `bootloader_dio_40m.bin`
`0x8000`  | `partitions.bin`
`0xe000`  | `boot_app0.bin`
`0x10000` | `firmware.bin`

Tick the option to enable the flashing of these files and press on `Start`. The defaults should work, but depending on your version of the ESP32, other settings are necessary, which you can find in the documentation of the manufacturer.

## Linux / command line
Install `esptool.py`, preferably with the packet management system of your distribution. Open a new terminal window and change to the directory with the already decompressed archive. Then enter:
```
esptool.py --chip esp32 --before default_reset --after hard_reset write_flash --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin
```

## OTA update
If you already have flashed a version of it on you ESP32, you can navigate to the update page by clicking on the link in the last tab. Then updload the file `firmware.bin`. Attention: this firmware uses a custom flash-partition-layout, so it has to be flashed once by a flasher as described above. Using another OTA flasher (like an OTA example) doesn't work.


# Installation
TL;DR: If you want to do your own development, you have to install [platformio](https://platformio.org/), clone the repository with all the submodules, build and upload it.

## Windows
### Warning before you start
Read this file through, before starting to half-ass it. It is not so hard to get a working system, just give it enough time and install it in this order.
Some packets take a real long time to install with no visible progress. Just wait until finished.

### Install Prerequisites
1. install atom: https://atom.io/
1. inside atom:
   1. click on "install a package"
   1. search for "platformio-ide" and install it
   1. install clang as desribed on the website
   1. it's taking forever, so please be patient
   1. restart as asked
   1. open the platformio home tab (if not opened automaticaly, use the menu)
      1. go to "Platforms" (left side tile)
      1. choose the "Embedded" tab
      1. install the "Esspressiv 32" platform
1. install git: https://git-scm.com/downloads
   1. use the defaults in the setup, define atom as the default editor
1. install all needed drivers for your platform. This is usualy done by installing the CP210x-driver, but consult the documentation of the manufacturer of your esp32.

### Downloading the repository
1. open a folder in the explorer, preferably not too deep inside the drive. `C:\` or a folder under it should work
1. right click on it and choose "Git Bash Here"
1. enter `git clone --recursive https://github.com/eringerli/esp32-aog.git`

### Compiling
1. open the created folder above from the platformio home
1. click build (the tile with the tick), the missing dependencies should be installed automaticaly

### Upload to the ESP32
1. connect the ESP32 over USB
1. click on upload (the tile with the arrow)

Alternatively you can use the OTA-update in the WebUI: go to the last tab and upload a new image.

## Linux
Install `platformio` with the package management system of your distribution and find a guide to help you set it up for your particular editor/IDE.

To do it on the console, clone the repository with all the submodules (`git clone` with `--recurse-submodules`), then change into the cloned folder and enter:
```
platformio run -t upload
```

This takes care of all the required libraries and uploads it to a connected ESP32.

## Configuration

All configuration is done in the WebUI. To connect to the created hotspot of the esp32, using a mobile device is normally the simplest solution.

## Wifi connection mode

The IP address is static. If connecting to the WebUI directly by the ESP32 hotspot, the IP address will be 192.168.1.1 If you are connecting to the WebUI through the same network that the ESP32 is connected, it will be something like 192.168.xxx.71

After login in to the WebUI, you can then change the wifi to whatever you like. The ESP32 tries to login, and if that fails, makes a new hotspot called something like 'Section Rate Control XXXXXXXX'. The ending is unique to each and every ESP32 to avoid confusion. The ESP32 has automatic detection of connection modes, if the network it is trying to connect to comes up after a few minutes, it will automatically connect and turn off the created hotspot. If you connect to the ESP32 hotspot, it will stop trying to connect to the primary network. If the primary Wifi goes away after connection was ostablished, it will try to reconnect every few seconds.

**The configuration is saved as long as there is no complete clearing of the flash of the esp32.** To  reset to the defaults, you either press the button in the WebUi or erase the flash of the esp32 with platformio/your editor. A new upload of the firmware doesn't affect the saved settings. 

## Updating the repository
As there are sometimes new repositories/submodules added, enter the following to update:
1. go to the esp32-aog repository and open a git bash-terminal
1. `git pull`
1. `git submodule sync`
1. `git submodule init`
1. `git submodule update`

Repeat them as needed.

