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
9. Apply the PortAudio patch that is provided with Crystal Tracker by running the following command from the root of the PortAudio directory (ie, lib\\**portaudio**): `git apply ../patches/portaudio.patch`  
  This fixes a Windows-only glitch that will be fixed in the next major release of PortAudio but is being fixed manually here for now. See https://github.com/PortAudio/portaudio/commit/ba486a3a8c9e7b2a7b217ab6e31a4c46c6ca38db for more details.
10. Open lib\portaudio\build\msvc\\**portaudio.sln** in Visual Studio 2022.
11. A "One-way upgrade" dialog will open, since portaudio.sln was made for Visual Studio 2005. Click OK to upgrade.
12. In the **Solution Explorer**, remove the 4 ASIO-related cpp files under **Source Files → hostapi → ASIO** from the project. (Right-click each of the 4 cpp files and click "Remove".)
13. In the **Project Properties** window, go to **Configuration Properties → General** and change the value of **Configuration Type** from "Dynamic Library (.dll)" to "Static Library (.lib)". Do this for the Release configuration, and also the Debug configuration if desired.
14. In the **Project Properties** window, go to **Configuration Properties → C/C++ → Preprocessor** and add `PA_USE_ASIO=0;` to the beginning of the **Processor Definitions** property. Do this for the Release configuration, and also the Debug configuration if desired.
15. Open lib\portaudio\build\msvc\\**portaudio.def** in a text editor and comment out the 4 lines beginning with "PaAsio_" by putting a semicolon (`;`) at the start of the line.
16. Set the Solution Configuration to Release and set the Solution Platform to Win32 then press **Build → Build Solution**. You may also build Debug|Win32 if desired.
17. Move lib\portaudio\build\msvc\Win32\Release\portaudio.lib to lib\portaudio.lib (or move lib\portaudio\build\msvc\Win32\Debug\portaudio.lib to lib\Debug\portaudio.lib if Debug build).
18. Open lib\portaudio\bindings\cpp\build\vc7_1\\**static_library.sln** in Visual Studio 2022.
19. A "One-way upgrade" dialog will open. Click OK to upgrade.
20. In the **Project Properties** window, go to **Configuration Properties → C/C++ → Code Generation** and change **Runtime Library** from "Multi-threaded DLL (/MD)" to "Multi-threaded (/MT)". For the Debug configuration, change it from "Multi-threaded Debug DLL (/MDd)" to "Multi-threaded Debug (/MTd)".
21. Set the Solution Configuration to Release then press **Build → Build Solution**. You may also build Debug|x86 if desired.
22. Move lib\portaudio\bindings\cpp\lib\portaudiocpp-vc7_1-r.lib to lib\portaudiocpp-vc7_1-r.lib (or move lib\portaudio\bindings\cpp\lib\portaudiocpp-vc7_1-d.lib to lib\Debug\portaudiocpp-vc7_1-d.lib if Debug build).
23. Copy the lib\portaudio\bindings\cpp\include\\**portaudiocpp** folder to a new include\\**portaudiocpp** folder.
24. Copy the header files from lib\portaudio\include into the include\\**portaudiocpp** folder as well.

#### Setting up libopenmpt

25. Clone [libopenmpt-0.6.3](https://github.com/OpenMPT/openmpt/tree/libopenmpt-0.6.3) into lib\\**openmpt**. (ie, `git clone -b libopenmpt-0.6.3 https://github.com/OpenMPT/openmpt.git lib/openmpt`)
26. Open lib\openmpt\build\vs2022win10\\**libopenmpt-small.sln** in Visual Studio 2022.
27. Retarget the 4 projects to your installed version of the Windows 10 SDK if necessary.
28. For each of the 4 projects, go to **Configuration Properties → C/C++ → Code Generation** and change **Spectre Mitigation** to "Disabled". Do this for the Release configuration, and also the Debug configuration if desired.
29. Set the Solution Configuration to Release and set the Solution Platform to Win32 then press **Build → Build Solution**. You may also build Debug|Win32 if desired.
30. Move the 4 .lib files from lib\openmpt\build\lib\vs2022win10\x86\Release to lib (or from lib\openmpt\build\lib\vs2022win10\x86\Debug to lib\Debug if Debug build).
31. Copy the public interface header files from lib\openmpt\libopenmpt into a new include\\**libopenmpt** folder. (Only libopenmpt.hpp, libopenmpt_ext.hpp, libopenmpt_config.h, and libopenmpt_version.h are required.)

#### Building Crystal Tracker

32. Open ide\\**crystal-tracker.sln** in Visual Studio 2022.
33. If the Solution Configuration dropdown on the toolbar says Debug, set it to **Release**.
34. Go to **Build → Build Solution** to build the project. This will create bin\Release\\**crystaltracker.exe**. (A Debug build will create bin\Debug\\**crystaltrackerd.exe**.)

**Note:** To build a 64-bit executable, you will need to create the x64 Solution Platform for fltk (fltk.sln) and portaudiocpp (static_library.sln). To do this, go to **Build → Configuration Manager…** and in the **Active solution platform:** dropdown select **<New…>**. Set the new platform to "x64" and copy settings from the 32-bit platform (either Win32 or x86). Make sure the "Create new project platforms" checkbox is checked and then click OK. Make sure the project configuration changes described above are also applied to this platform. portaudio.sln and libopenmpt-small.sln already have an x64 target.  
After building the x64 libs for fltk, portaudio, portaudiocpp, and openmpt, copy the .lib files to lib\\**x64** and lib\Debug\\**x64**.


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
./configure --prefix="$(realpath "$PWD/../..")" CXXFLAGS="-O2" CFLAGS="-O2"
make
make install
cd bindings/cpp
./configure --prefix="$(realpath "$PWD/../../../..")" CXXFLAGS="-O2" CFLAGS="-O2"
make
make install
popd

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
	--without-flac \
	CXX="g++-8" CXXFLAGS="-O2" CFLAGS="-O2" \
	PKG_CONFIG_PATH="$(realpath "$PWD/../../lib/pkgconfig")"
make
make install
popd

mv include/pa_linux_alsa.h include/portaudio.h include/portaudiocpp/

# Build Crystal Tracker
make

# Install Crystal Tracker
# (tested on Ubuntu and Ubuntu derivatives only; it just copies bin/crystaltracker
#  and res/app.xpm to system directories and creates the .desktop entry)
sudo make install
```


## Mac

Follow the ["Install and build"](#install-and-build-crystal-tracker) instructions for Linux, but when building libopenmpt from source use `CXX="clang++"` instead of `CXX="g++-8"`.

When relocating the PortAudio headers, there will be no pa_linux_alsa.h, so that step will just be: `mv include/portaudio.h include/portaudiocpp/`
