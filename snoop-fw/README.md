# Motocompacto Mods

## Setup

1. Follow normal setup to install PlatformIO on your system, either in VS Code
   or in a way that you can access the `pio` command on your terminal.
    * Most of this readme is written using the `pio` tool, so if someone can
      add instructions for the VSCode process that would be helpful!


## Build/Flash

1. `pio run` to build.
    * PlatformIO will download the toolchain and associated tools.
2. `pio run -e <env> -t upload -t monitor` to build, flash, and monitor.
    * ***Alaways provide `-e <env>` or it'll build everything.***
    * PlatformIO will try to detect USB serial devices and probe them to find a
      board that matches what we have configured in `platformio.ini`.
    * You can do `pio run -e <env> --list-targets` to see what all options you can use
      with `-t`.
3. `pio device monitor` will open a serial monitor.
    * Also tries to auto-detect based on `platformio.ini`.
    * Can be configured with a bunch of `monitor_*` settings.

* What is `<env>`?
    * Check inside `platformio.ini` and look at the `[env:...]` lines, these
      are separate "environments" we have set up to build the firmware for
      different boards. Ideally we might get the firmware to detect the board
      it is on and configure itself, but it is still useful to understand
      how these work so we can do things like have special builds that enable
      sleep/special features.
    * For the full e-load board the `s3` env is the one to choose, the other envs
      in this project are just used for testing outside of that setup.

* Which port should I use on the ESP32-S3 devkit?
    * Both `USB` and `UART` ports work for uploading/monitoring but debugging
      only works on the `USB` port!

* What're the serial port monitor settings?
    * Same as every arduino, but `115200 bps` by default.


## Adjusting ESP32 System Configuration

* `pio run -e <env> -t menuconfig`

This should open a menu in your terminal where you can view and adjust a MASSIVE
amount of parameters.

We try to leave as many settings on the defaults as possible so framework updates
won't suck, so if you change something please use the `[D] Save minimal config`
option and then place the changes in the appropriate `sdkconfig.defaults.*` file/files.


## Debugging In Terminal (GDB)

The PlatformIO extension in VS Code has debugger integration, so try that first,
but this way should work basically everywhere too. If you're familiar with using
`pdb` to debug python scripts in a terminal then GDB will feel very familiar
since `pdb` is modeled after it!

1. `pio debug` to build a debug build.
2. `piodebuggdb -x .pioinit` to flash, start gdb, and attach it.
    * This is a shortcut for `pio debug --interface gdb -- -x .pioinit`
    * You must be using the `USB` port on the DevKitM-1.
    * If not then you need to have a supported debug probe attached, drivers
      installed, and the `debug_tool` set approriately in `platformio.ini`

After some noisy logs and maybe some PlatformIO python exceptions (this is annoying
but is just some janky stderr handling they're doing...) you should have a
gdb prompt!

There are lots of tutorials on how to use GDB but the very basics are:
* `b <location>` to set a breakpoint
    * `<filename>:<function>` is super useful
        * Ex. `main.cpp:loop` for the start of the loop function
        * Ex. `wifi.c:wifi_ap_init` for the AP init function in the ESP-IDF project.
    * You can do `<filename>:<linenumber>` and some other things.
    * There are also features like conditional breakpoints, tracepoints etc.
* `c` to `continue` until the next breakpoint
* `n` to go to the `next` line of code (not stepping into function calls, etc.)
* `s` to `step` to and/or into the next line of code
* `l` to `list` the lines of code where the app is currenly paused
* `p <name>` to `print` the value of a variable
* `q` to `quit` the debugger

And a ton more stuff. The way PlatformIO wraps it up is both handy (the `.pioinit`
script takes the boilerplate and cruft away) but a bit clunky with the way they
handle subprocess io which causes some noisy python exception output but doesn't
actually break the process.


## Code Formatting

This project is using `clang-format` with a minimal `.clang-format` config here.
Just install the tool or a plugin for your editor and use it before commiting
files please!


## ESP-IDF Documentation

See [here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html),
especially the "API Reference" section in the sidebar.
* A lot of stuff is under ["API Reference" -> "Peripherals API"](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/index.html)
* "Storage API" -> "Non-Volatile Storage Library" is interesting.
* "Networking APIs" -> "Wi-Fi" -> "Wi-Fi" is most of the basic wifi stuff.
* "Networking APIs" -> "ESP-NETIF" -> "ESP-NETIF" is the TCP/IP stack stuff.
* "Application Procols" has a lot of builtin stuff for things like MODBUS, MQTT, etc.

It is also useful to get comfortable looking at `~/.platformio/packages/framwwork-espidf`
to see the actual source and some examples. Some API's are badly documented so
knowing to look in here is very valuable.
