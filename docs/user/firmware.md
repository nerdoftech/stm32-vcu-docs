# Firmware and tools

## Ways to talk to the VCU

| Tool | Connects through | Good for |
|---|---|---|
| **Web interface** on the OpenInverter ESP8266 Wi-Fi module | The RS232/UART pins (1, 2) | Day-to-day setup: every parameter, live plots of spot values, firmware updates, saving and loading a JSON backup. |
| **Serial terminal** (any terminal program, 115200 baud) | The same UART, through a USB-serial adapter | Scripting, recovery, CAN map commands. |
| **CAN** (OpenInverter tools with a CAN adapter) | CAN1, node ID 3 | Configuring without Wi-Fi; the same commands over SDO. |

`UseRS232 = On` keeps the UART at a fixed speed for a plain RS232 terminal. Leave it Off
when you use the Wi-Fi module, which may switch to a faster rate.

## Terminal commands

| Command | Does |
|---|---|
| `get <name>` | Print one parameter or spot value. `get udc,opmode` prints several. |
| `set <name> <value>` | Change a parameter (range-checked). Takes effect immediately. |
| `save` | Store all parameters and the CAN map in flash. |
| `load` | Reload parameters from flash. |
| `defaults` | Load factory defaults into memory (flash is unchanged until `save`). |
| `all` | Print every parameter and value. |
| `list` | List names and units. |
| `atr` | Print min, max and default for every parameter. |
| `json` | Everything as JSON (what the web interface uses). |
| `stream <n> <names>` | Print values repeatedly, for logging. |
| `errors` | Print the error log with timestamps. |
| `can ...` | CAN mapping, see [CAN mapping](can-mapping.md). |
| `serial` | Print the chip's unique serial number. |
| `reset` | Reboot. |

Nothing is permanent until you `save`. A reboot without saving restores the last saved
settings.

## Updating firmware

1. **Back up your settings**: in the web interface, save the parameter JSON to your
   computer.
2. Get `stm32_vcu.bin` from the upstream
   [releases](https://github.com/damienmaguire/Stm32-vcu/releases), or build it
   ([Building](../dev/build.md)).
3. Update through the web interface's firmware page, or another method your bootloader
   supports.
4. After the reboot, check `version` and look over your settings.

Saved parameters are stored separately from the program and matched by ID, so they
normally survive an update. Parameters that are new in the update start at their
defaults.

!!! warning "Do not update or save while driving"
    Writing flash pauses the VCU's control loops for a moment. The firmware does not stop
    you from doing it in Run mode.

## Example configurations

The firmware repository's [`Parameters/`](https://github.com/damienmaguire/Stm32-vcu/tree/master/Parameters)
folder has real parameter files (BMW E31/E39/E46 builds, a truck, an L200). They are
useful to compare against, but don't load one blindly: throttle calibration, voltages
and bus assignments are specific to each vehicle.
