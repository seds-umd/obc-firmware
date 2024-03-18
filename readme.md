# OBC Firmware

## Project Structure

Directories:

* build - build artifacts, generated locally and not committed to git
* external - git submodules for any external code
* include - header files (.h)
* obc_tools - python tools for talking to firmware
* src - source files (.c)
* test - unit tests written using [Unity](https://github.com/ThrowTheSwitch/Unity)

Files:

* CMakeLists.txt - CMake config for project

## Modules

* `command_handler.c` - generic command handler to process commands
* `commands.c` - actual command functions that are registered with the command handler
* `logging.c` - functions for logging messages
* `openlst.c` - openlst UART driver, processes commands from openlst format and sends to command handler
* `scheduler.c` - schedules to run at certain intervals

## Unit Tests

Unit tests live in the `test` directory and use the [Unity](https://github.com/ThrowTheSwitch/Unity) framework. They are designed to run natively on the host (your laptop), not the target (the RP2040), so they are compiled and run differently. `test/CMakeLists.txt` defines an entirely separate build environment from the top level `CMakeLists.txt`, so only one will be used at a time. To run unit tests:

```bash
mkdir test/build
cd test/build
cmake ..
make
./tests
```

To add a new unit test:

* Write the test in a file starting with `test_` in the `test` directory (ex: `test/test_commands.c`)
* In `test/CMakeLists.txt`, add the .c file to the `add_executables` function, with all the other unit test files
* In `test/main.c`, add a function declaration for the test function at the top and then call the function with `RUN_TEST(test_function)`

[Unit test assertion cheat sheet](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsCheatSheetSuitableforPrintingandPossiblyFraming.pdf)

## Development

Things to keep in mind:

* When adding a new source file, make sure it's added to `CMakeLists.txt`, and if you want to use it in unit tests, `test/CMakeLists.txt` as well

## Python Interface

A Python interface is available to send and receive commands from a computer. This is useful for debugging and testing.

First time setup:

```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

Update (only if the command handler in the openlst repo has changed):

```bash
pip install -r requirements.txt --force-reinstall
```

To run:
* 0000 means broadcast HWID, useful for debugging
* Replace serial port with the actual serial port being used

```bash
./obc_tools/obc.py 0000 --port /dev/serial/by-id/usb-Raspberry_Pi_Picoprobe__CMSIS-DAP__E66038B7136AA739-if01
```

Once in the shell, some available commands are:

```python
obc.ping()
obc.reboot()
obc.gpio.mode(pin, mode)
obc.gpio.set(pin, val)
```

The shell supports tab completion, so you can type `obc.` and press tab to get a list of available commands.

To get more information about a given command, you can use IPython's `?` operator, ex: `obc.ping?`. This will return the docstring which should tell you what that command does and how to use it. If you do `obc.ping??`, it will show you the source code of the function.
