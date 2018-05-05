# hranol
Multiplatform image batch processing

Hranol is lightweight command line utility that takes image folders and filtering
parameters as input and applies these filters to all images. It was specifically
designed for batch processing grayscale video frames of high-speed camera.

Currently supported filters (see below for details):
- Static background removal (Mean filter)
- Contrast filter (Normalization)
- Mask filter

## Teaser
```
$  hranol -m "mask.png" -s 1.5 "Run 10-38-11"
```
Command applies mask `mask.png` to each image from target folder `Run 10-38-11`. It also computes the
average of all images and subtracts it from each image with factor `1.5`. All resulting images are
saved to `fltrd_Run 10-38-11` folder created inside `Run 10-38-11` folder.

## Prerequisites
You will need:
- OpenCV 3 (>= 3)
- CMake (>= 3.1)
- Compiler:
  - gcc (>= 8)
  - MSVC (>= 19.14 included in VS 2017 15.7)
  - Clang (not supported yet)
  
## Installation
Hranol can run under `Windows` and `Linux`. It is possible to compile it on `OS X` as well, but with `gcc 8`
rather than `Clang`. The reason is extensive use of `filesystem` library which is currently not supported
in `Clang` (5. May 2018).

### Windows
Installing `CMake` and `Visual Studio` is straighforward, but `OpenCV` might be a little tricky.

#### OpenCV
You should install pre-built OpenCv libraries. The easiest way to do this is by following the official
guide: [Installation OpenCV by Using the Pre-built Libraries](https://docs.opencv.org/3.2.0/d3/d52/tutorial_windows_install.html)
Don't forget to correctly set `OPENCV_DIR` environment variable and include `%OPENCV_DIR%\bin` into your `PATH`.

With all the prerequisites installed, you need to clone this repo:
```
git clone https://github.com/romeritto/hranol
```
Open the cloned folder in `Visual Studio` as `CMake project` (*File->Open->CMake*). You can now compile the project and run it.

### Linux
As with `Windows`, installing `OpenCV` can be a hassle.

#### OpenCV
The easy way is to use an installation script, like this one: https://github.com/milq/milq/blob/master/scripts/bash/install-opencv.sh

Having the prerequisites installad on your system, you can now clone the repo and install hranol:
```
git clone https://github.com/romeritto/hranol
cd hranol
sudo ./install-hranol.sh
```
# Install links
OpenCV
https://github.com/milq/milq



GCC 8
sudo apt-get update -y &&
sudo apt-get upgrade -y &&
sudo apt-get dist-upgrade -y &&
sudo apt-get install build-essential software-properties-common -y &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y &&
sudo apt-get update -y &&
sudo apt-get install gcc-8 g++-8 -y

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 700 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8

sudo update-alternatives --config gcc

select gcc-8
