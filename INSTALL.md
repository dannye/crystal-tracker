# Install Guide

## Windows

### Build Crystal Tracker from source

You will need [Microsoft Visual Studio](https://visualstudio.microsoft.com/vs/); the Community edition is free.

#### Clone this repository

0. Clone [crystal-tracker](https://github.com/dannye/crystal-tracker). This will create the **crystal-tracker** folder.

#### Setting up FLTK

1. Clone [fltk release-1.3.8](https://github.com/fltk/fltk/tree/release-1.3.8) into lib\\**fltk**. (ie, `git clone -b release-1.3.8 https://github.com/fltk/fltk.git lib/fltk`)
2. Open lib\fltk\\**abi-version.ide** in a text editor such as Notepad and replace "`#undef FL_ABI_VERSION`" with "`#define FL_ABI_VERSION 10308`". Save it.
3. Open lib\fltk\ide\VisualC2010\\**fltk.sln** in Visual Studio 2022. (Other versions may or may not work, I haven't tried.)
4. A "Retarget Projects" dialog will open, since fltk.sln was made for Visual Studio 2010. Click OK to upgrade the Windows SDK version and platform toolset.
5. Go to **Build → Batch Build…**, check the projects **fltk**, **fltkimages**, **fltkjpeg**, **fltkpng**, and **fltkzlib** in the Release configuration, and click the **Build** button.
6. Move all the .lib files from lib\fltk\lib\\\*.lib up to lib\\\*.lib. (You may also choose the Debug config in the previous step and move the .lib files to lib\Debug\\\*.lib instead.)
7. Copy the lib\fltk\\**FL** folder to a new include\\**FL** folder.

#### Setting up PortAudio

8. Clone [portaudio v19.7.0](https://github.com/PortAudio/portaudio/tree/v19.7.0) into lib\\**portaudio**. (ie, `git clone -b v19.7.0 https://github.com/PortAudio/portaudio.git lib/portaudio`)
9. Open lib\portaudio\build\msvc\\**portaudio.sln** in Visual Studio 2022.
10. A "One-way upgrade" dialog will open, since portaudio.sln was made for Visual Studio 2005. Click OK to upgrade.
11. In the **Solution Explorer**, remove the 4 ASIO-related cpp files under **Source Files → hostapi → ASIO** from the project. (Right-click each of the 4 cpp files and click "Remove".)
12. In the **Project Properties** window, go to **Configuration Properties → General** and change the value of **Configuration Type** from "Dynamic Library (.dll)" to "Static Library (.lib)". Do this for the Release configuration, and also the Debug configuration if desired.
13. In the **Project Properties** window, go to **Configuration Properties → C/C++ → Preprocessor** and add `PA_USE_ASIO=0;` to the beginning of the **Processor Definitions** property. Do this for the Release configuration, and also the Debug configuration if desired.
14. Open lib\portaudio\build\msvc\\**portaudio.def** in a text editor and comment out the 4 lines beginning with "PaAsio_" by putting a semicolon (`;`) at the start of the line.
15. Set the Solution Configuration to Release and set the Solution Platform to Win32 then press **Build → Build Solution**. You may also build Debug|Win32 if desired.
16. Move lib\portaudio\build\msvc\Win32\Release\portaudio.lib to lib\portaudio.lib (or move lib\portaudio\build\msvc\Win32\Debug\portaudio.lib to lib\Debug\portaudio.lib if Debug build).
17. Open lib\portaudio\bindings\cpp\build\vc7_1\\**static_library.sln** in Visual Studio 2022.
18. A "One-way upgrade" dialog will open. Click OK to upgrade.
19. In the **Project Properties** window, go to **Configuration Properties → C/C++ → Code Generation** and change **Runtime Library** from "Multi-threaded DLL (/MD)" to "Multi-threaded (/MT)". For the Debug configuration, change it from "Multi-threaded Debug DLL (/MDd)" to "Multi-threaded Debug (/MTd)".
20. Set the Solution Configuration to Release then press **Build → Build Solution**. You may also build Debug|x86 if desired.
21. Move lib\portaudio\bindings\cpp\lib\portaudiocpp-vc7_1-r.lib to lib\portaudiocpp-vc7_1-r.lib (or move lib\portaudio\bindings\cpp\lib\portaudiocpp-vc7_1-d.lib to lib\Debug\portaudiocpp-vc7_1-d.lib if Debug build).
22. Copy the lib\portaudio\bindings\cpp\include\\**portaudiocpp** folder to a new include\\**portaudiocpp** folder.
23. Copy the header files from lib\portaudio\include into the include\\**portaudiocpp** folder as well.

#### Setting up libopenmpt

24. Clone [libopenmpt-0.6.3](https://github.com/OpenMPT/openmpt/tree/libopenmpt-0.6.3) into lib\\**openmpt**. (ie, `git clone -b libopenmpt-0.6.3 https://github.com/OpenMPT/openmpt.git lib/openmpt`)
25. Open lib\openmpt\build\vs2022win10\\**libopenmpt-small.sln** in Visual Studio 2022.
26. Retarget the 4 projects to your installed version of the Windows 10 SDK if necessary.
27. Set the Solution Configuration to Release and set the Solution Platform to Win32 then press **Build → Build Solution**. You may also build Debug|Win32 if desired.
28. Move the 4 .lib files from lib\openmpt\build\lib\vs2022win10\x86\Release to lib (or from lib\openmpt\build\lib\vs2022win10\x86\Debug to lib\Debug if Debug build).
29. Copy the public interface header files from lib\openmpt\libopenmpt into a new include\\**libopenmpt** folder. (Only libopenmpt.hpp, libopenmpt_config.h, and libopenmpt_version.h are required.)

#### Building Crystal Tracker

30. Open ide\\**crystal-tracker.sln** in Visual Studio 2022.
31. If the Solution Configuration dropdown on the toolbar says Debug, set it to **Release**.
32. Go to **Build → Build Solution** or press `Ctrl+Shift+B` to build the project. This will create bin\Release\\**crystaltracker.exe**. (A Debug build will create bin\Debug\\**crystaltrackerd.exe**.)


## Linux

### Install dependencies

You need at least g++ 7 for C++17 support.
g++ 8 is needed if building libopenmpt from source.

#### Ubuntu/Debian

Run the following commands:

```bash
sudo apt install make g++ git autoconf
sudo apt install zlib1g-dev libpng-dev libxpm-dev libx11-dev libxft-dev libxinerama-dev libfontconfig1-dev x11proto-xext-dev libxrender-dev libxfixes-dev
```

### Install and build Crystal Tracker

Run the following commands:

```bash
# Clone Crystal Tracker
git clone https://github.com/dannye/crystal-tracker.git
cd crystal-tracker

# Build FLTK 1.3.8 with the latest ABI enabled
# (even if you already have libfltk1.3-dev installed)
git clone -b release-1.3.8 https://github.com/fltk/fltk.git lib/fltk
pushd lib/fltk
./autogen.sh --prefix="$(realpath "$PWD/../..")" --with-abiversion=10308
make
make install
popd

# Build PortAudio v19.7.0
git clone -b v19.7.0 https://github.com/PortAudio/portaudio.git lib/portaudio
pushd lib/portaudio

./configure --prefix="$(realpath "$PWD/../..")"
make
make install

# ---
# for now, also install system-wide to make sure it's visible to libopenmpt in the next step
# it would be nice to avoid this though... setting CPPFLAGS/LDFLAGS did not work :(
make clean
./configure
make
sudo make install
# ---

cd bindings/cpp
./configure --prefix="$(realpath "$PWD/../../../..")"
make
make install

# ---
# for now, also install system-wide to make sure it's visible to libopenmpt in the next step
# it would be nice to avoid this though... setting CPPFLAGS/LDFLAGS did not work :(
make clean
./configure
make
sudo make install
# ---

popd
mv include/pa_linux_alsa.h include/portaudio.h include/portaudiocpp/

# Build libopenmpt-0.6.3
pushd lib
wget https://lib.openmpt.org/files/libopenmpt/src/libopenmpt-0.6.3+release.autotools.tar.gz
mkdir libopenmpt && tar xf libopenmpt-0.6.3+release.autotools.tar.gz -C libopenmpt --strip-components=1
cd libopenmpt
./configure --prefix="$(realpath "$PWD/../..")" \
	--without-mpg123 \
	--without-ogg \
	--without-vorbis \
	--without-vorbisfile \
	--without-sndfile \
	--without-flac
make CXX="g++-8 -std=c++17"
make install
popd

# Build Crystal Tracker
make

# Install Crystal Tracker
# (tested on Ubuntu and Ubuntu derivatives only; it just copies bin/crystaltracker
#  and res/app.xpm to system directories and creates the .desktop entry)
sudo make install
```
