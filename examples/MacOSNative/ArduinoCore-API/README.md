# ArduinoCore-API

[![Unit Tests](https://github.com/arduino/ArduinoCore-API/workflows/Unit%20Tests/badge.svg)](https://github.com/arduino/ArduinoCore-API/actions?workflow=Unit+Tests)
[![codecov](https://codecov.io/gh/arduino/ArduinoCore-API/branch/master/graph/badge.svg)](https://codecov.io/gh/arduino/ArduinoCore-API)
[![Spell Check status](https://github.com/arduino/ArduinoCore-API/actions/workflows/spell-check.yml/badge.svg)](https://github.com/arduino/ArduinoCore-API/actions/workflows/spell-check.yml)

This repository hosts the hardware independent layer of Arduino core. In other words it contains the abstract definition of the Arduino core API, consisting of hardware-independent header files that are then included and implemented by the various platform-specific cores.

Having a single place where the Arduino API is defined means that there is no longer a String implementation within every Arduino core (a String module within ArduinoCore-avr, a String module within ArduinoCore-samd, a String module within ArduinoCore-megaavr …) but rather one String implementation within ArduinoCore-API which all other cores utilise. This has the pleasant side effects that bugs fixed or features added within the ArduinoCore-API String implementation are automatically propagated to all cores utilizing ArduinoCore-API.

As of now, the following official cores are utilising ArduinoCore-API:

* [megaavr](https://github.com/arduino/ArduinoCore-megaAVR)
* [mbed](https://github.com/arduino/ArduinoCore-mbed)
* [samd](https://github.com/arduino/ArduinoCore-samd)
* [renesas](https://github.com/arduino/ArduinoCore-renesas)

There's an ongoing effort to port the others, while maintainers of third-party cores are strongly invited to follow the same route in order to stay up-to-date with the new language features. For backwards compatibility, every revision of this repo will increase the `ARDUINO_API_VERSION` define.

## Documentation

The Arduino API is documented in the official [language reference](https://www.arduino.cc/reference/en/), whose sources are located in [this repository](https://github.com/arduino/reference-en) and are open to contributions from the community.

## Support

This repository is not directly usable by final users. If you need assistance with Arduino, see the [Help Center](https://support.arduino.cc/) and browse the [forum](https://forum.arduino.cc).

## Development

### Bugs & Issues

If you want to report an issue with this core, you can submit it to the [issue tracker](https://github.com/arduino/ArduinoCore-API/issues) of this repository. Some rules apply:

* If your issue is about a specific hardware platform, report it to its repository. This one is only about discussing the generic API.
* Before posting, please check if the same problem has been already reported by someone else to avoid duplicates.
* Remember to include as much detail as you can about your hardware set-up, code and steps for reproducing the issue. Make sure you're using an original Arduino board.

### Contributions

Contributions are always welcome! You can submit them directly to this repository as Pull Requests. Please provide a detailed description of the problem you're trying to solve. We also appreciate any help in testing issues and patches contributed by other users.

### Unit testing

This repository includes a test suite that covers most of the API and that is designed to run on generic hardware, thus not requiring a development board. We call this _host-based unit-testing_. In order to test the features that are only defined but not implemented in this repository, mock implementations are included.

Please help us improve the coverage of the test suite!

#### To build and run unit tests

The unit tests are automatically built by GitHub as part of pull request checks (in `.github/workflows/unit-tests.yml`).

To build and run locally:

**Dependencies**

* [CMake](https://cmake.org/)
* [GCC](https://gcc.gnu.org/)

On (Ubuntu) Linux run:

```bash
sudo apt-get install build-essential cmake
```

From the project root:

```bash
cd test && mkdir build && cd build
cmake ..
make && bin/test-ArduinoCore-API
```

### Implementing ArduinoCore-API

In order to compile a core which is implementing ArduinoCore-API you'll need to copy/symlink the `api` directory to the target's `cores/arduino` directory as part of your development and release workflow. The most elegant and effective solution is to develop your core with `api` symlinked and produce the distributable archive by telling `tar` to follow symlinks. Example:

```bash
tar --exclude='*.git*' -cjhvf $yourcore-$version.tar.bz2 $yourcore/
```

The API is coded to the C++11 standard and the core's compiler must be able to support that version of the language.

Documentation for how to integrate with a Arduino core (which is necessary if you do not download the Arduino core via the Boards Manager) can be found here:
* [ArduinoCore-megaavr](https://github.com/arduino/ArduinoCore-megaavr#developing)
* [ArduinoCore-mbed](https://github.com/arduino/ArduinoCore-mbed#clone-the-repository-in-sketchbookhardwarearduino-git)
* [ArduinoCore-samd](https://github.com/arduino/ArduinoCore-samd/#developing)

## Support the project

This open source code is maintained by Arduino with the help of the community. We invest a considerable amount of time in testing code, optimizing it and introducing new features. Please consider [buying original Arduino boards](https://store.arduino.cc) to support our work on the project.

## License and credits

This code is licensed under the terms of the GNU LGPL 2.1. If you have questions about licensing please contact us at [license@arduino.cc](mailto:license@arduino.cc).

