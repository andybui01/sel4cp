This is an experimental fork of sel4cp to evaluate the configuration of partitioned systems
using sel4cp. This also includes a prototype of multithreading.

# seL4 Core Platform

The purpose of the seL4 Core Platform (sel4cp) is to enable system designers to create static software systems based on the seL4 microkernel.

The seL4 Core Platform consists of three parts:

   * seL4 Core Platform libraries
   * seL4 Core Platform system initializer
   * seL4 Core Platform tool

The seL4 Core Platform is distributed as a software development kit (SDK).

This repository is the source for the sel4cp SDK.

If you are *developing* sel4cp itself this is the repo you want!

If you are a system designer and want to *use* the sel4cp SDK please download a pre-built SDK.
Please see the manual in the SDK for instructions on using the SDK itself.

The remainder of this README is for sel4cp developers.

## Developer system requirements

Development of sel4cp has primarily been performed on Ubuntu 18.04 LTS (x86_64).

This section attempts to list the packages or external development tools which are required during development.
At this stage it may be incomplete.
Please file an issue if additional packages are required.

* git
* make
* python3.9
* python3.9-venv
* musl-1.2.2
* aarch64-none-elf toolchain; tested with GCC 13.2.0 [Nov 2023]

There are no known packages for the toolchain that support TLS, hence it must be compiled from source with the following configuration for GCC:
`--target=aarch64-none-elf --disable-shared --disable-nls --disable-threads --enable-tls --enable-languages=c --enable-checking=release`

On Ubuntu 18.04 there are no packages available for musl-1.2.2; it must be compiled from source.
On Ubuntu 18.04 Python 3.9 is available via the *deadsnakes* PPA: https://launchpad.net/~deadsnakes/+archive/ubuntu/ppa
To use this:

    $ sudo add-apt-repository ppa:deadsnakes/ppa
    $ sudo apt update
    $ sudo apt install python3.9 python3.9-venv

Additonally, a number of Python libraries are needed.
These should be installed using `pip`.

    $ python3.9 -m venv pyenv
    $ ./pyenv/bin/pip install --upgrade pip setuptools wheel
    $ ./pyenv/bin/pip install -r requirements.txt

Note: It is a high priority of the authors to ensure builds are self-contained and repeatable.
A high value is placed on using specifically versioned tools.
At this point in time this is not fully realised, however it is a high priority to enable this in the near future.

## Building the SDK

    $ ./pyenv/bin/python build_sdk.py --no-test --sel4=<path to sel4>

Currently, we recommend using the `--no-test` option as the tests are not up to date, and this option skips testing of the tool.

## Using the SDK

After building the SDK you probably want to build a system!
Please see the SDK user manual for documentation on the SDK itself.

When developing the SDK it is helpful to be able to build examples system quickly for testing purposes.
The `dev_build.py` script can be used for this purpose.
This script is not included in the SDK and is just meant for use of use of sel4cp developers.

By default `dev_build.py` will use the example source directly from the source directory.
In some cases you may want to test that the example source has been correctly included into the SDK.
To test this pass `--example-from-sdk` to the build script.

By default `dev_build.py` will use the the sel4cp tool directory from source (in `tool/sel4coreplat`).
However, in some cases it is desirable to test the sel4cp tool built into the SDK.
In this case pass `--tool-from-sdk` to use the tool that is built into the SDK.

Finally, by default the `dev_build.py` script relies on the default Makefile dependecy resolution.
However, in some cases it is useful to force a rebuild while doing SDK development.
For example, the `Makefile` can't know about the state of the sel4cp tool source code.
To support this a `--rebuild` option is provided.

## SDK Layout

The SDK is delivered as a `tar.gz` file.

The SDK top-level directory is `sel4cp-sdk-$VERSION`.

The directory layout underneath the top-level directory is [OUTDATED]:

```
bin/
bin/sel4cp
bsp/$board/$config/include/
bsp/$board/$config/include/sel4cp.h
bsp/$board/$config/lib/
bsp/$board/$config/lib/libsel4cp.a
bsp/$board/$config/lib/sel4cp.ld
bsp/$board/$config/elf
bsp/$board/$config/elf/loader.elf
bsp/$board/$config/elf/kernel.elf
bsp/$board/$config/elf/monitor.elf
```

## Supported Configurations

## Release

In release configuration the loader, kernel and monitor do *not* perform any direct serial output.


## Debug

The debug configuration includes basic print output form the loader, kernel and monitor.
