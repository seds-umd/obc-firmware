# OBC Firmware

## Project Structure

Directories:

* build - build artifacts, generated locally and not committed to git
* external - git submodules for any external code
* include - header files (.h)
* src - source files (.c)
* test - unit tests written using [Unity](https://github.com/ThrowTheSwitch/Unity)

Files:

* CMakeLists.txt - CMake config for project

## Modules

* `commands_ground.c` - commands from ground through openLST
* `commands_pib.c` - commands from PIB
* `filesystem.c` - interface between flash and LittleFS filesystem
* `scheduler.c` - schedules tasks
* `openlst.c` - openlst UART driver

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
