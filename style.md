* Header files start with `#pragma once` to prevent them from being repeated
    * Comment describing file can go above if it exists
* Includes at the top of a file are ordered as:
    * Header for the specific file (if there is one)
    * Headers for other things within the project
    * Headers for pico-sdk and other external libraries
    * Headers from the system (the ones that use <>)
    * Within each group, headers are sorted alphabetically
* Functions must have docstrings in the header file (see other header files for formatting)
