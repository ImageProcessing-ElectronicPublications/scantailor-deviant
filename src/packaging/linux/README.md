# Ho to build the Linux version of Scan Tailor

## Install Prerequisites

First, install the following software.
Unless stated otherwise, take the latest stable version.

Here are the versions with which the 2023 release was built on ubuntu-20.04.6:

### Required software

1. **Base build tools**

    ~~~less
    sudo apt install build-essential
    ~~~

2. **CMake**

    ~~~less
    sudo apt install cmake
    ~~~

3. **Boost**

    ~~~less
    sudo apt install libboost1.71-dev
    sudo apt install libboost-test1.71-dev
    ~~~

4. **Qt5**

    ~~~less
    sudo apt install qtbase5-dev
    sudo apt install qttools5-dev
    sudo apt install libqt5svg5-dev
    ~~~

5. **zlib**

    ~~~less
   sudo apt install zlib1g-dev
    ~~~

6. **libjpeg**

    ~~~less
    sudo apt install libjpeg-dev
    ~~~

7. **libpng**

    ~~~less
    sudo apt install libpng-dev
    ~~~

8. **libtiff**

    ~~~less
    sudo apt install libtiff-dev
    ~~~

9. **libcanberra**

    ~~~less
    sudo apt install libcanberra-dev
    ~~~

### Optional software

1. **OpenJPEG**

    ~~~less
    sudo apt install libopenjp2-7-dev
    ~~~

2. **Exiv2**

    ~~~less
    sudo apt install libexiv2-dev
    ~~~

3. **CMake GUI**

    ~~~less
    sudo apt install cmake-qt-gui
    ~~~

## Build Scan Tailor

### Create a build directory

Goto /scantailor directory.

Unpack scantailor archive to /src subdirectory.

You should get the following directory structure:

~~~less
scantailor/
 └─ src/
     ├─ cmake/
     ├─ debian/
     ├─ src/
     ... 
     ├─ CMakeLists.txt
     ...
~~~

From /scantailor directory run commands:

### Configure

Configure if optional libraries were installed:

~~~less
cmake\
 -S ./src\
 -B ./build\
 -D CPACK_GENERATOR:STRING="DEB;TGZ"\
 -D ENABLE_EXIV2:BOOL=ON\
 -D ENABLE_OPENJPEG:BOOL=ON\
 -D CPACK_SYSTEM_NAME:STRING="ubuntu-24-amd64"
~~~

Configure if optional libraries were not installed:

~~~less
cmake\
 -S ./src\
 -B ./build\
 -D CPACK_GENERATOR:STRING="DEB;TGZ"\
 -D ENABLE_EXIV2:BOOL=OFF\
 -D ENABLE_OPENJPEG:BOOL=OFF\
 -D CPACK_SYSTEM_NAME:STRING="debian-12-arm64"
~~~

You can also configure build using GUI if it is installed:

~~~less
cmake-gui
~~~

### Build

~~~less
cmake\
 --build ./build\
 --config Release\
 --parallel
~~~

### Build installation package

You can create installation package:

~~~less
cpack\
 -B $(pwd)/pkg\
 --config ./build/CPackConfig.cmake
~~~

When finished, package will be located in the /pkg directory.

### Install

You can also install the program directly:

~~~less
cmake\
 --build ./build\
 --config Release\
 --target install
~~~

### Build source packages

~~~less
cpack\
 -G "TGZ;TXZ"\
 -B $(pwd)/pkg\
 --config ./build/CPackSourceConfig.cmake
~~~

When finished, source archives will be located in the /pkg directory.
