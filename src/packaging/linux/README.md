# Ho to build the Linux version of Scan Tailor

## Install Prerequisites

First, install the following software.
Unless stated otherwise, take the latest stable version.

Here are the versions with which the 2023 release was built on ubuntu-20.04.6:

### Required software

1. **Base build tools**

    ~~~ text
    sudo apt install build-essential
    ~~~

2. **CMake**

    ~~~ text
    sudo apt install cmake
    ~~~

3. **Boost**

    ~~~ text
    sudo apt install libboost1.71-dev
    sudo apt install libboost-test1.71-dev
    ~~~

4. **Qt5**

    ~~~ text
    sudo apt install qtbase5-dev
    sudo apt install qttools5-dev
    sudo apt install libqt5svg5-dev
    ~~~

5. **zlib**

    ~~~ text
   sudo apt install zlib1g-dev
    ~~~

6. **libjpeg**

    ~~~ text
    sudo apt install libjpeg-dev
    ~~~

7. **libpng**

    ~~~ text
    sudo apt install libpng-dev
    ~~~

8. **libtiff**

    ~~~ text
    sudo apt install libtiff-dev
    ~~~

9. **libcanberra**

    ~~~ text
    sudo apt install libcanberra-dev
    ~~~

### Optional software

1. **OpenJPEG**

    ~~~ text
    sudo apt install libopenjp2-7-dev
    ~~~

2. **Exiv2**

    ~~~ text
    sudo apt install libexiv2-dev
    ~~~

3. **CMake GUI**

    ~~~ text
    sudo apt install cmake-qt-gui
    ~~~

## Build Scan Tailor

### Create a build directory

Goto /scantailor directory.

Unpack scantailor archive to /src subdirectory.

You should get the following directory structure:

~~~ text
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

~~~ text
cmake -S ./src -B ./build -DENABLE_EXIV2:BOOL=ON -DENABLE_OPENJPEG:BOOL=ON
~~~

Configure if optional libraries were not installed:

~~~ text
cmake -S ./src -B ./build -DENABLE_EXIV2:BOOL=OFF -DENABLE_OPENJPEG:BOOL=OFF
~~~

You can also configure build using GUI if it is installed:

~~~ text
cmake-gui
~~~

### Build

~~~ text
cmake --build ./build --config Release --target scantailor-universal --parallel 4
cmake --build ./build --config Release --target scantailor-universal-cli --parallel 4
~~~

### Build installation package

You can create a deb installation package:

~~~ text
cpack -G DEB --config ./build/CPackConfig.cmake -B $(pwd)/pkg
~~~

When finished, package will be located in the /pkg directory.

### Install

You can also install the program directly:

~~~ text
cmake --build ./build --config Release --target install
~~~

### Build source packages

~~~ text
cpack -G "TGZ;TXZ" --config ./build/CPackSourceConfig.cmake -B $(pwd)/pkg
~~~

When finished, source archives will be located in the /pkg directory.
