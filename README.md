# ksfeeder

## How to build (Windows)

1. Download [mingw](https://sourceforge.net/projects/mingw/), [cmake](https://cmake.org/download/) and install
2. Copy and rename ` C:\MinGW\bin\mingw32-make.exe ` to ` C:\MinGW\bin\make.exe `
3. Add ` C:\MinGW\bin ` and ` C:\Program Files\CMake\bin ` to PATH
4. Open terminal and check

    ```bash
    gcc -v
    make -v
    cmake -version
    ```

5. build

    ```bash
    cd ksfeeder
    mkdir build
    cd build
    cmake -G "MinGW Makefiles" ../
    make clean
    make
    ```
