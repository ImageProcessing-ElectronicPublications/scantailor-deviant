# How to build the macOS version of Scan Tailor

## Downloading Prerequisites

First, download the following software.
Unless stated otherwise, take the latest stable version.

Here are the versions with which the v2025.02.20-alpha-2 was built:

### Required software

1. **Apple's developer tools**

    Site: <https://developer.apple.com/macos/>

    File: Xcode

    OR

    File: Command Line Tools for Xcode

2. **CMake**

    Site: <https://cmake.org/>

    File: cmake-4.0.0-rc5-macos-universal.dmg

3. **Qt5**

    Site: <https://www.qt.io/>

    File: qt-everywhere-src-5.12.12.tar.xz

4. **Boost**

    Site: <https://www.boost.org/>

    File: boost_1_87_0.tar.gz

5. **Eigen**

    Site: <https://eigen.tuxfamily.org/>

    File: eigen-3.4.0.tar.gz

6. **libjpeg-turbo**

    Site: <https://libjpeg-turbo.org/>

    File: libjpeg-turbo-3.1.0.tar.gz

7. **libpng**

    Site: <http://www.libpng.org/>

    File: libpng-1.6.47.tar.gz

8. **libtiff**

    Site: <http://www.libtiff.org/>

    File: libtiff-v4.7.0.tar.gz

### Optional software

1. **OpenJPEG**

    Site: <https://www.openjpeg.org/>

    File: v2.5.3.tar.gz

2. **Exiv2**

    Site: <https://exiv2.org/>

    File: v0.28.5.tar.gz

## Install Build Software

   Install **Xcode** or **Command Line Tools for Xcode**

## Get Dependencies

You can:

* Build all dependencies yourself via Xcode / CLTools
  
* Install ready-made ones using homebrew

## Build Scan Tailor

### Create a build folder

Make scantailor build folder:

```less
mkdir scantailor-deviant
cd scantailor-deviant
```

Unpack scantailor-deviant:

```less
tar xzf scantailor-deviant.tar.gz
```

Or clone it from github:

```less
git clone https://github.com/ImageProcessing-ElectronicPublications/scantailor-deviant
```

Run commands:

### Configure

Configure if all libraries were installed:

```less
export CMAKE_PREFIX_PATH=\
/opt/homebrew/Cellar/boost:\
/opt/homebrew/Cellar/eigen:\
/opt/homebrew/Cellar/exiv2:\
/opt/homebrew/Cellar/jpeg-turbo:\
/opt/homebrew/Cellar/libpng:\
/opt/homebrew/Cellar/libtiff:\
/opt/homebrew/Cellar/libmp:\
/opt/homebrew/Cellar/openjpeg:\
/opt/homebrew/Cellar/qt@5

cmake\
 -S scantailor-deviant\
 -B build\
 -D ENABLE_OPENGL=ON\
 -D ENABLE_EXIV2=ON\
 -D ENABLE_OPENJPEG=ON
```

Configure if optional libraries were not installed:

```less
export CMAKE_PREFIX_PATH=\
/opt/homebrew/Cellar/boost:\
/opt/homebrew/Cellar/eigen:\
/opt/homebrew/Cellar/exiv2:\
/opt/homebrew/Cellar/jpeg-turbo:\
/opt/homebrew/Cellar/libpng:\
/opt/homebrew/Cellar/libtiff:\
/opt/homebrew/Cellar/libmp:\
/opt/homebrew/Cellar/openjpeg:\
/opt/homebrew/Cellar/qt@5

cmake\
 -S scantailor-deviant\
 -B build\
 -D ENABLE_OPENGL=ON\
 -D ENABLE_EXIV2=OFF\
 -D ENABLE_OPENJPEG=OFF
 ```

### Build

```less
cmake\
 --build build\
 --parallel
```

### Install

```less
cmake\
 --install build\
 --prefix=$(pwd)/install
```

When finished, the ready-to-use program will be located in the /install folder.

### Build installation package

```less
cpack\
 -B ./pkg\
 --config ./build/CPackConfig.cmake
```

When finished, source archive will be located in the /pkg folder.

### Directory structure after build completes

```less
scantailor-deviant
├───build
│   └───...
├───install
│   └───scantailor-deviant.app
├───pkg
│   └───scantailor-deviant-0.2.14-Darwin.dmg
└───scantailor-deviant
    │   CMakeLists.txt
    │   config.h.in
    │   COPYING
    │   LICENSE
    │   README.md
    │   version.h
    ├───cmake
    │   └───...
    ├───debian
    │   └───...
    └───src
        └───...
```
