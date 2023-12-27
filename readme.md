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

## Unit Tests

Unit tests live in the `test` directory and use the [Unity](https://github.com/ThrowTheSwitch/Unity) framework. They are designed to run natively on the host (your laptop), not the target (the RP2040), so they are compiled and run differently. `test/CMakeLists.txt` defines an entirely separate build environment from the top level `CMakeLists.txt`, so only one will be used at a time. To run unit tests:

```bash
mkdir test/build
cd test/build
cmake ..
make
./tests
```

To add a new unit test, make a new file in `test` starting with `test_` (ie, `test/test_commands.c`), then in `test/main.c` add an include for the file and add the test function to `int main()`.

## Development

Things to keep in mind:

* When adding a new source file, make sure it's added to `CMakeLists.txt`, and if you want to use it in unit tests, `test/CMakeLists.txt` as well
