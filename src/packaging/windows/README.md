# Ho to build the Windows version of Scan Tailor

## Downloading Prerequisites

First, download the following software.
Unless stated otherwise, take the latest stable version.

Here are the versions with which the 2023 release was built:

### Required software:

1. **Visual Studio С++ Native Desktop**

    Site: https://visualstudio.microsoft.com/

    File: Visual Studio Community 2022

2. **CMake**

    Site: https://cmake.org/

    File: cmake-3.27.4-windows-x86_64.msi

3. **Perl**

    Site: https://strawberryperl.com/

    File: strawberry-perl-5.32.1.1-64bit.msi

4. **Jom**

    Site: https://wiki.qt.io/Jom

    File: jom_1_1_4.zip

5. **Qt5**

    Site: https://www.qt.io/

    File: qt-everywhere-src-5.12.12.tar.xz

6. **Boost**

    Site: https://www.boost.org/

    File: boost_1_83_0.7z

7. **zlib**

    Site: https://zlib.net/

    File: zlib-1.3.tar.xz

8. **libjpeg-turbo**

    Site: https://libjpeg-turbo.org/

    File: libjpeg-turbo-3.0.0.tar.gz

9. **libpng**

    Site: http://www.libpng.org/

    File: libpng-1.6.40.tar.xz

10. **libtiff**

    Site: http://www.libtiff.org/

    File: tiff-4.6.0.tar.xz

### Optional software:

1. **OpenJPEG**

    Site: https://www.openjpeg.org/

    File: openjpeg-2.5.0.zip

2. **Exiv2**

    Site: https://exiv2.org/

    File: exiv2-0.27.7.zip

3. **NSIS**

    Site: http://nsis.sourceforge.net/

    File: nsis-3.09-setup.exe

## Build Dependencies

1. **Install software**

    Install **Visual Studio** , **CMake**, **Perl**
    Instal **NSIS** (optional)

2. **Create a build folder**

    From now on this document will be assuming the build folder is D:\Prog\.

    Create folders for all dependencies and for Scan Tailor.
    Create version subfolders within dependency folders.
    Dependencies will be installed into these subfolders.
    You should get a directory structure like this:

    ~~~ text
    D:\
    └─ Prog\
        ├─ boost\
        │   └─ 1.83.0\     
        ├─ exiv2\
        │   └─ 0.27.7\
        ├─ libjpeg-turbo\
        │   └─ 3.0.0\
        ├─ libpng\
        │   └─ 1.6.40\
        ├─ openjpeg\
        │   └─ 2.5.0\
        ├─ Qt\
        │   └─ 5.12.12\ 
        ├─ scantailor\
        ├─ tiff\
        │   └─ 4.6.0\
        └─ zlib\
            └─ 1.3\
    ~~~

3. **Build boost**

    Unpack boost to his install subfolder.
    You should get the following folder structure:

    ~~~ text
    D:\
    └─ Prog\
        └─ boost\
            └─ 1.83.0\
                ├─ boost\
                ├─ doc\
                ├─ libs\
                ...
                ├─ bootstrap.bat
                ...
    ~~~

    Go to \boost\1.83.0 directory and run commands:

    ~~~ text
    bootstrap.bat
    b2.exe variant=release
    ~~~

    After the build is complete, the \bin.v2 folder can be deleted.

4. **Build Qt**

    Goto \Qt\5.12.12 folder.

    Unpack Qt archive to \src subfolder.

    Unpack jom archive to \jom subfolder.

    Create \build subfolder.

    In the build subfolder, create a build.cmd file with the following contents:

    ~~~ text
    @EM Set paths
    SET PATH=%CD%\qtbase\bin;%CD%\gnuwin32\bin;%PATH%

    @REM Congigure Qt
    ..\src\configure -prefix %CD%\.. -opensource -confirm-license -platform win32-msvc -release -mp -opengl desktop -nomake examples -nomake tests

    @REM Build packages: Core Gui Widgets Xml Network OpenGL Svg
    ..\jom\jom module-qtbase

    @REM Build packages: LinguistTools
    ..\jom\jom module-qttools

    @REM Build additional image formats
    ..\jom\jom module-qtimagefo

    @REM Install packages: Core Gui Widgets Xml Network OpenGL
    ..\jom\jom module-qtbase-install_subtargets

    @REM Install packages: LinguistTools
    ..\jom\jom module-qttools-install_subtargets

    @REM Install additional image formats
    ..\jom\jom module-qtimageformats-install_subtargets
    ~~~

    You should get the following folder structure:

    ~~~ text
    D:\
    └─ Prog\
        └─ Qt\
            └─ 5.12.12\
                ├─ build\
                │   └─ build.cmd
                ├─ jom\
                │   └─ jom.exe
                └─ src\
                    ├─ coin\
                    ├─ gnuwin32\
                    ...
                    ├─ qtbase\
                    ...
                    ├─ configure.bat
                    ...
    ~~~

    From the Start menu, launch the shortcut 
    "x64 Native Tools Command Prompt for VS 2022".
    When the developer console opens, 
    go to the Qt build folder and run the batch file.

    ~~~ text
    D:
    CD D:\Prog\Qt\5.12.12\build\
    build.cmd
    ~~~

    When finished, the folder structure will look like this:

    ~~~ text
    D:\
    └─ Prog\
        └─ Qt\
            └─ 5.12.12\
                ├─ bin\
                ├─ build\
                ├─ doc\
                ├─ include\
                ├─ jom\
                ├─ lib\
                ├─ mkspecs\
                ├─ phrasebooks\
                ├─ plugins\
                ├─ qml\
                └─ src\
    ~~~

    The \build and \src folders can be deleted.

5. Build **zlib**

    Goto \zlib\1.3 folder.

    Unpack zlib archive to \src subfolder.

    You should get the following folder structure:

    ~~~ text
    D:\
    └─ Prog\
        └─ zlib\
            └─ 1.3\
                └─ src\
                    ├─ amiga\
                    ├─ contrib\
                    ├─ doc\
                    ... 
                    ├─ CMakeLists.txt
                    ...
    ~~~

    From \zlib\1.3 folder folder run commands:

    ~~~ text
    cmake -S .\src -B .\build -DCMAKE_INSTALL_PREFIX:PATH=%CD%
    cmake --build .\build --config Release --target install --parallel 4
    ~~~

    When finished, the folder structure will look like this:

    ~~~ text
    D:\
    └─ Prog\
        └─ zlib\
            └─ 1.3\
                ├─ bin\
                │  └─ zlib.dll
                ├─ build\
                ├─ include\
                │  └─ *.h
                ├─ lib\
                │  └─ *.lib
                ├─ share\
                └─ src\
    ~~~

    The \build and \src folders can be deleted.

6. Build **libjpeg-turbo**

    Goto \libjpeg-turbo\3.0.0 folder.

    Unpack \libjpeg-turbo archive to \src subfolder.

    You should get the following folder structure:

    ~~~ text
    D:\
    └─ Prog\
        └─ libjpeg-turbo\
            └─ 3.0.0\
                └─ src\
                    ├─ cmakescripts\
                    ├─ doc\
                    ├─ fuzz\
                    ... 
                    ├─ CMakeLists.txt
                    ...
    ~~~

    From libjpeg-turbo\3.0.0 folder run commands:

    ~~~ text
    cmake -S .\src -B .\build -DCMAKE_INSTALL_PREFIX:PATH=%CD%
    cmake --build .\build --config Release --target install --parallel 4
    ~~~

    When finished, the folder structure will look like this:

    ~~~ text
    D:\
    └─ Prog\
        └─ libjpeg-turbo\
            └─ 3.0.0\
                ├─ bin\
                │  ├─ jpeg62.dll
                │  ...
                ├─ build\
                ├─ include\
                │  └─ *.h
                ├─ lib\
                │  └─ *.lib
                ├─ share\
                └─ src\
    ~~~

    The \build and \src folders can be deleted.

7. Build **libpng**

    Goto \libpng\1.6.40 folder.

    Unpack libpng archive to \src subfolder.

    You should get the following folder structure:

    ~~~ text
    D:\
    └─ Prog\
        └─ libpng\
            └─ 1.6.40\
                └─ src\
                    ├─ arm\
                    ├─ ci\
                    ├─ contrib\
                    ... 
                    ├─ CMakeLists.txt
                    ...
    ~~~

    From \libpng\1.6.40 folder run commands:

    ~~~ text
    cmake -S .\src -B .\build -DCMAKE_INSTALL_PREFIX:PATH=%CD% -DCMAKE_PREFIX_PATH:PATH="d:\Prog\zlib\1.3"
    cmake --build .\build --config Release --target install --parallel 4
    ~~~

    When finished, the folder structure will look like this:

    ~~~ text
    D:\
    └─ Prog\
        └─ libpng
            └─ 1.6.40\
                ├─ bin\
                │  ├─ libpng16.dll
                │  ...
                ├─ build\
                ├─ include\
                │  └─ *.h
                ├─ lib\
                │  └─ *.lib
                ├─ share\
                └─ src\
    ~~~

    The \build and \src folders can be deleted.

8. Build **libtiff**

    Goto \tiff\4.6.0 folder.

    Unpack tiff archive to \src subfolder.

    You should get the following folder structure:

    ~~~ text
    D:\
    └─ Prog\
        └─ tiff\
            └─ 4.6.0\
                └─ src\
                    ├─ build\
                    ├─ cmake\
                    ├─ config\
                    ... 
                    ├─ CMakeLists.txt
                    ...
    ~~~

    From \tiff\4.6.0 folder run commands:

    ~~~ text
    cmake -S .\src -B .\build -DCMAKE_INSTALL_PREFIX:PATH=%CD% -DCMAKE_PREFIX_PATH:PATH="d:\Prog\zlib\1.3;d:\Prog\libjpeg-turbo\3.0.0"
    cmake --build .\build --config Release --target install --parallel 4
    ~~~

    When finished, the folder structure will look like this:

    ~~~ text
    D:\
    └─ Prog\
        └─ tiff\
            └─ 4.6.0\
                ├─ bin\
                │  ├─ tiff.dll
                │  ...
                ├─ build\
                ├─ include\
                │  └─ *.h
                ├─ lib\
                │  └─ *.lib
                ├─ share\
                └─ src\
    ~~~

    The \build and \src folders can be deleted.

9. Build **OpenJPEG** (optional)

    Goto \openjpeg\2.5.0 folder.

    Unpack tiff archive to \src subfolder.

    You should get the following folder structure:

    ~~~ text
    D:\
    └─ Prog\
        └─ openjpeg\
            └─ 2.5.0\
                └─ src\
                    ├─ cmake\
                    ├─ doc\
                    ├─ scripts\
                    ... 
                    ├─ CMakeLists.txt
                    ...
    ~~~

    From \openjpeg\2.5.0 folder run commands:

    ~~~ text
    cmake -S .\src -B .\build -DCMAKE_INSTALL_PREFIX:PATH=%CD% -DBUILD_CODEC:BOOL=FALSE -DCMAKE_PREFIX_PATH:PATH="d:\Prog\zlib\1.3;d:\Prog\libpng\1.6.40;d:\Prog\tiff\4.6.0"
    cmake --build .\build --config Release --target install --parallel 4
    ~~~

    When finished, the folder structure will look like this:

    ~~~ text
    D:\
    └─ Prog\
        └─ openjpeg\
            └─ 2.5.0\
                ├─ bin\
                │  ├─ openjp2.dll
                │  ...
                ├─ build\
                ├─ include\
                │  └─ openjpeg-2.5\
                │      └─ *.h
                ├─ lib\
                │  └─ *.lib
                └─ src\
    ~~~

    The \build and \src folders can be deleted.

10. Build **Exiv2** (optional)

    Goto \exiv2\0.27.7 folder.

    Unpack exiv2 archive to \src subfolder.

    You should get the following folder structure:

    ~~~ text
    D:\
    └─ Prog\
        └─ exiv2\
            └─ 0.27.7\
                └─ src\
                    ├─ ci\
                    ├─ cmake\
                    ├─ contrib\
                    ... 
                    ├─ CMakeLists.txt
                    ...
    ~~~

    From \exiv2\0.27.7 folder run commands:

    ~~~ text
    cmake -S .\src -B .\build -DCMAKE_INSTALL_PREFIX:PATH=%CD% -DEXIV2_ENABLE_WIN_UNICODE:BOOL=ON -DEXIV2_ENABLE_XMP:BOOL=OFF -DEXIV2_ENABLE_PRINTUCS2=OFF -DEXIV2_ENABLE_LENSDATA=OFF -DCMAKE_PREFIX_PATH:PATH="d:\Prog\zlib\1.3"
    cmake --build .\build --config Release --target install --parallel 4
    ~~~

    When finished, the folder structure will look like this:

    ~~~ text
    D:\
    └─ Prog\
        └─ exiv2\
            └─ 0.27.7\
                ├─ bin\
                │  ├─ exiv2.dll
                │  ...
                ├─ build\
                ├─ include\
                │  └─ exiv2
                │      └─ *.h
                ├─ lib\
                │  └─ *.lib
                ├─ share\
                └─ src\
    ~~~

    The \build and \src folders can be deleted.

## Build Scan Tailor

### Create a build folder

Goto \scantailor folder.

Unpack scantailor archive to \src subfolder.

You should get the following folder structure:

~~~ text
D:\
└─ Prog\
    └─ scantailor\
        └─ src\
            ├─ cmake\
            ├─ debian\
            ├─ src\
            ... 
            ├─ CMakeLists.txt
            ...
~~~

From \scantailor folder run commands:

### Configure

Configure if all libraries were installed:

~~~ text
cmake -S .\src -B .\build -DCMAKE_INSTALL_PREFIX:PATH=%CD%\install -DENABLE_EXIV2:BOOL=ON -DENABLE_OPENGL:BOOL=ON -DENABLE_OPENJPEG:BOOL=ON -DCMAKE_PREFIX_PATH:PATH="d:\Prog\boost\1.83.0;d:\Prog\exiv2\0.27.7;d:\Prog\libjpeg-turbo\3.0.0;d:\Prog\libpng\1.6.40;d:\Prog\openjpeg\2.5.0;d:\Prog\Qt\5.12.12;d:\Prog\tiff\4.6.0;d:\Prog\zlib\1.3"
~~~

Configure if optional libraries were not installed:

~~~ text
cmake -S .\src -B .\build -DCMAKE_INSTALL_PREFIX:PATH=%CD%\install -DENABLE_EXIV2:BOOL=OFF -DENABLE_OPENGL:BOOL=ON -DENABLE_OPENJPEG:BOOL=OFF -DCMAKE_PREFIX_PATH:PATH="d:\Prog\boost\1.83.0;d:\Prog\libjpeg-turbo\3.0.0;d:\Prog\libpng\1.6.40;d:\Prog\Qt\5.12.12;d:\Prog\tiff\4.6.0;d:\Prog\zlib\1.3"
~~~

### Build

~~~ text
cmake --build .\build --config Release --target scantailor-deviant --parallel 4
cmake --build .\build --config Release --target scantailor-deviant-cli --parallel 4
~~~

### Install

~~~ text
cmake --build .\build --config Release --target install --parallel 4
~~~

When finished, the ready-to-use program will be located in the \install folder.

### Build installation packages

If NSIS was installed:

~~~ text
cpack -G "NSIS;ZIP;7Z" --config ./build/CPackConfig.cmake -B %CD%\pkg
~~~

If NSIS was not installed:

~~~ text
cpack -G "ZIP;7Z" --config ./build/CPackConfig.cmake -B %CD%\pkg
~~~

When finished, packages will be located in the \pkg folder.

### Build source package

~~~ text
cpack -G "ZIP" --config ./build/CPackSourceConfig.cmake -B %CD%\pkg
~~~

When finished, source archive will be located in the \pkg folder.
