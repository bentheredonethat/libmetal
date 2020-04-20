# libmetal

## Overview

Libmetal provides common user APIs to access devices, handle device interrupts
and request memory across the following operating environments:
  * Linux user space (based on UIO and VFIO support in the kernel)
  * RTOS (with and without virtual memory)
  * Bare-metal environments

## Project configuration
The configuration phase begins when the user invokes CMake. CMake begins by processing the CMakeLists.txt file and the cmake directory.
Some cmake options are available to help user to customize the libmetal to their
own project.

* **WITH_DOC** (default ON): Build with documentation. Add -DWITH_DOC=OFF in
cmake command line to disable.
* **WITH_EXAMPLES** (default ON): Build with application exemples. Add
-DWITH_DOC=OFF in cmake command line to disable the option.
* **WITH_TESTS** (default ON): Build with application tests. Add -DWITH_DOC=OFF
in cmake command line to disable the option.
* **WITH_DEFAULT_LOGGER** (default ON): Build with default trace logger. Add
-DWITH_DEFAULT_LOGGER=OFF in cmake command line to disable the option.
* **WITH_SHARED_LIB** (default ON): Generate a shared library. Add
-DWITH_SHARED_LIB=OFF in cmake command line to disable the option.
* **WITH_STATIC_LIB** (default ON): Generate a static library. Add
-DWITH_STATIC_LIB=OFF in cmake command line to disable the option.
*  **WITH_ZEPHYR** (default OFF): Build for Zephyr environment. Add
-DWITH_ZEPHYR=ON in cmake command line to enable the the option.

## Build Steps

### Building for Linux Host
```
 $ git clone https://github.com/OpenAMP/libmetal.git
 $ mkdir -p libmetal/<build directory>
 $ cd libmetal/<build directory>
 $ cmake ..
 $ make VERBOSE=1 DESTDIR=<libmetal install location> install
```

### Cross Compiling for Linux Target
Use [meta-openamp](https://github.com/openamp/meta-openamp) to build
libmetal library.
Use package `libmetal` in your yocto config file.

### Building for Baremetal

To build on baremetal, you will need to provide a toolchain file. Here is an
example toolchain file:
```
    set (CMAKE_SYSTEM_PROCESSOR "arm"              CACHE STRING "")
    set (MACHINE "zynqmp_r5" CACHE STRING "")

    set (CROSS_PREFIX           "armr5-none-eabi-" CACHE STRING "")
    set (CMAKE_C_FLAGS          "-mfloat-abi=soft -mcpu=cortex-r5 -Wall -Werror -Wextra \
       -flto -Os -I/ws/xsdk/r5_0_bsp/psu_cortexr5_0/include" CACHE STRING "")

    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
    SET(CMAKE_AR  "gcc-ar" CACHE STRING "")
    SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
    SET(CMAKE_C_ARCHIVE_FINISH   true)

    include (cross-generic-gcc)
```
* Note: other toolchain files can be found in the  `cmake/platforms/` directory.
* Compile with your toolchain file.
```
    $ mkdir -p build-libmetal
    $ cd build-libmetal
    $ cmake <libmetal_source> -DCMAKE_TOOLCHAIN_FILE=<toolchain_file>
    $ make VERBOSE=1 DESTDIR=<libmetal_install> install
```

### Building for Zephyr
The [zephyr-libmetal](https://github.com/zephyrproject-rtos/libmetal)
implements the libmetal for the Zephyr project. It is mainly  a fork of this repository, with some add-ons for integration in the Zephyr project.

Following instruction is only to be able to run test application on a QEMU running
a Zephyr environment.

As Zephyr uses CMake, we build libmetal library and test application as
targets of Zephyr CMake project. Here is how to build libmetal for Zephyr:
```
    $ export ZEPHYR_GCC_VARIANT=zephyr
    $ export ZEPHYR_SDK_INSTALL_DIR=<where Zephyr SDK is installed>
    $ source <git_clone_zephyr_project_source_root>/zephyr-env.sh

    $ cmake <libmetal_source_root> -DWITH_ZEPHYR=on -DBOARD=qemu_cortex_m3 \
      [-DWITH_TESTS=on]
    $ make VERBOSE=1 all
    # If we have turned on tests with "-DWITH_TESTS=on" when we run cmake,
    # we launch libmetal test on Zephyr QEMU platform as follows:
    $ make VERBOSE=1 run
```

## Interfaces

The following subsections give an overview of interfaces provided by libmetal.

### Platform and OS Independent Utilities

These interfaces do not need to be ported across to new operating systems.

#### I/O

The libmetal I/O region abstraction provides access to memory mapped I/O and
shared memory regions.  This includes:
  * primitives to read and write memory with ordering constraints, and
  * ability to translate between physical and virtual addressing on systems
    that support virtual memory.

#### Log

The libmetal logging interface is used to plug log messages generated by
libmetal into application specific logging mechanisms (e.g. syslog).  This
also provides basic message prioritization and filtering mechanisms.

#### List

This is a simple doubly linked list implementation used internally within
libmetal, and also available for application use.

#### Other Utilities

The following utilities are provided in lib/utilities.h:
  * Min/max, round up/down, etc.
  * Bitmap operations
  * Helper to compute container structure pointers
  * ... and more ...

#### Version

The libmetal version interface allows user to get the version of the library.

### Top Level Interfaces

The users will need to call two top level interfaces to use libmetal APIs:
  * metal_init - initialize the libmetal resource
  * metal_finish - release libmetal resource

Each system needs to have their own implementation inside libmetal for these
two APIs to call:
  * metal_sys_init
  * metal_sys_finish

For the current release, libmetal provides Linux userspace and bare-metal
implementation for metal_sys_init and metal_sys_finish.

For Linux userspace, metal_sys_init sets up a table for available shared pages,
checks whether UIO/VFIO drivers are avail, and starts interrupt handling
thread.

For bare-metal, metal_sys_init and metal_sys_finish just returns.

### Atomics

The libmetal atomic operations API is consistent with the C11/C++11 stdatomics
interface.  The stdatomics interface is commonly provided by recent toolchains
including GCC and LLVM/Clang.  When porting to a different toolchain, it may be
necessary to provide an stdatomic compatible implementation if the toolchain
does not already provide one.

### Alloc

libmetal provides memory allocation and release APIs.

### Locking

libmetal provides the following locking APIs.

#### Mutex

libmetal has a generic mutex implementation which is a busy wait.  It is
recommended to have OS specific implementation for mutex.

The Linux userspace mutex implementation uses futex to wait for the lock
and wakeup a waiter.

#### Condition Variable
libmetal condition variable APIs provide "wait" for user applications to wait
on some condition to be met, and "signal" to indicate a particular even occurs.

#### Spinlock
libmetal spinlock APIs provides busy waiting mechanism to acquire a lock.

### Shmem

libmetal has a generic static shared memory implementation.  If your OS has a
global shared memory allocation, you will need to port it for the OS.

The Linux userspace shmem implementation uses libhugetlbfs to support huge page
sizes.

### Bus and Device Abstraction

libmetal has a static generic implementation.  If your OS has a driver model
implementation, you will need to port it for the OS.

The Linux userspace abstraction binds the devices to UIO or VFIO driver.
The user applications specify which device to use, e.g. bus "platform" bus,
device "f8000000.slcr", and then the abstraction will check if platform UIO
driver or platform VFIO driver is there.  If platform VFIO driver exists,
it will bind the device to the platform VFIO driver, otherwise, if UIO driver
exists, it will bind the device to the platform UIO driver.

The VFIO support is not yet implemented.

### Interrupt

libmetal provides APIs to register an interrupt, disable interrupts and restore
interrupts.

The Linux userspace implementation will use a thread to call select() function
to listen to the file descriptors of the devices to see if there is an interrupt
triggered.  If there is an interrupt triggered, it will call the interrupt
handler registered by the user application.

### Cache

libmetal provides APIs to flush and invalidate caches.

The cache APIs for Linux userspace are empty functions for now as cache
operations system calls are not avaiable for all architectures.

### DMA

libmetal DMA APIs provide DMA map and unmap implementation.

After calling DMA map, the DMA device will own the memory.
After calling DMA unmap, the cpu will own the memory.

For Linux userspace, it only supports to use UIO device memory as DMA
memory for this release.

### Time
libmetal time APIs provide getting timestamp implementation.

### Sleep
libmetal sleep APIs provide getting delay execution implementation.

### Compiler

This API is for compiler dependent functions.  For this release, there is only
a GCC implementation, and compiler specific code is limited to atomic
operations.
