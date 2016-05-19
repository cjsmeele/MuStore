# MuStore block storage #

## Introduction ##

MuStore is a modular platform-independent C++ block storage driver.

The primary development target for this project is the
[Arduino Due](https://www.arduino.cc/en/Main/ArduinoBoardDue).

## Description ##

**TODO**

## Included modules ##

### Block storage backends ###

- Memory backend.
- File backend (using cstdio).

### Filesystem backends ###

- A (currently read-only) generic FAT driver with support for FAT12, FAT16 and FAT32.

## Author ##

[Chris Smeele](https://github.com/cjsmeele)

## License ##

Copyright (c) 2016, Chris Smeele.

MuStore is licensed under the GNU Lesser General Public License version
3 or higher (LGPLv3+). See the `LICENSE` file for details.
