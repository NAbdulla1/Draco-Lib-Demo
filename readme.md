# Draco Demo with C++ and Nodejs
This is a demo to use the [Draco](https://github.com/google/draco) library to **query Geometric Attributes by Attribute Metadata**. It is necessary in case of multiple Geometric Attributes of same type(like UV coordinate sets).

## C++
The C++ part demonstrates how to compress 3D Mesh Geometric data and decompress them using Attribute metadata. This is a cmake project, build as follows:
 - Run `mkdir build` at the root of this repo.
 - Run `cd build`
 - Run `cmake ..`
 - Run `cmake --build .` to create the `DracoDemo.exe` file
 - Run the exe file, it should create a file: `compressed_mesh.drc`

## Node.js
The Node.js part demonstrate how to decompress 3D Mesh Geometric data and query by Attribute metadata.
 - Go to the `node_decoder` directory.
 - Run `npm run dev` to start Express.js server.
 - Open the url(http://localhost:3000) to see the decoded data of the `compressed_mesh.drc` file.

### Notes on building the Draco library(`lib` and `include`)
 - We were not able to build the library by Visual Studio 2017
 - We were not able to build the library in Ubuntu 2024.4 LTS on WSL2
 - We have used [MSYS2 UCRT64](https://code.visualstudio.com/docs/cpp/config-mingw) on a Windows 11 machine
   - Install [MSYS2](https://github.com/msys2/msys2-installer/releases/download/2024-01-13/msys2-x86_64-20240113.exe) and open the terminal/shell
   - Install GCC MinGW: `pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain`
   - Install CMake: `pacman -S --needed mingw-w64-ucrt-x86_64-cmake`
   - Clone Draco repo: `git clone git@github.com:google/draco.git`
   - Checkout a release tag. Example `git checkout tags/1.5.6`
   - Navigate to the cloned repo directory in MSYS2 shell/terminal
   - Run `mkdir build` and `mkdir install`
   - Run `cd build`
   - Run `cmake .. -DCMAKE_INSTALL_PREFIX=../install`
   - Run `cmake --build .`
   - Run `cmake --install .`
   - Go to the install directory(`cd ../install`) and copy the `lib` and `include` directories to the root of this repository.
