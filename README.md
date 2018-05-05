# hranol
Multiplatform image batch processing

Hranol is lightweight command line utility that takes image folders and filtering
parameters as input and applies these filters to all images. It was designed
for batch processing grayscale video frames of high-speed camera.

Currently supported filters (see below for details):
- Background subtraction (Mean filter)
- Contrast filter (Normalization)
- Mask filter

## Teaser
```
$  hranol -m "particles/mask.bmp" -s 1.1 "monitor"
```
Command applies mask `mask.png` to each image from target folder `monitor`. It also computes the
average of all images and subtracts it from each image with factor `1.1`. All resulting images are
saved to `fltrd_monitor` folder created inside `monitor`.

## Prerequisites
You will need:
- OpenCV 3
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
You should install pre-built OpenCV libraries. The easiest way to do this is by following the official
guide: [OpenCV installation by Using the Pre-built Libraries](https://docs.opencv.org/3.2.0/d3/d52/tutorial_windows_install.html)
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

Having the prerequisites installed on your system, you can now clone the repo and install hranol:
```
git clone https://github.com/romeritto/hranol
cd hranol
cmake -H. -Bbuild
sudo cmake --build build --target install
```

## Filters
Below is a description of 3 built-in filters.

### Background subtraction (Mean filter)
Use this filter to remove static background. It averages images in the folder and then subtracts the average multiplied by *factor* from each image. In hranol, you can invoke this filter by using `-s[factor]` option, where *factor* is a positive floating point value.

### Contrast filter (Normalization)
Filter changes the range of pixel intensity values. Grayscale images have their intensity values in range *[0, 255]*. The filter takes a range *[b, e]* and maps it to the original *[0, 255]*. It assigns a new value `In` to each pixel with intensity `I` using the following rules:
- `(I < b) -> In = 0`
- `(I > e) -> In = 255`
- `otherwise In = (255 * (I - b + 1)) / (e - b + 2)`

You can use this filter in hranol by specifying `-b[range begin]` and `-e[range end]` options.

### Mask filter
Use this filter to mask your images. You should provide a path to *mask image* with option `-m[mask file path]`. The *mask image* has to be of the same size as all of the input images. Masking algorithm is simple, *mask image* non-zero elements indicate which image elements need to be copied.

## What does hranol do
After the input is parsed, all folders are processed separately. For each *input folder* output images are written to the *output folder* which is located in the *input folder*. Name of the *output folder* is same as *input folder*, but prefixed with *fltrd_* (default behaviour, can be changed with `-p` option). A small log file named *fltrd_info.txt* is also stored in the *output folder*.

Filters are applied in the following order: 
1. Background subtraction
2. Contrast filter
3. Mask filter

## Examples
### Printing help
```
$ hranol -h
    hranol [folders...] {OPTIONS}

    Hranol -- batch image processing utility. By default only images in given
    folders are processed and the output is saved to a subfolder with prefix
    "fltrd". Supported filters: static background subtraction, contrast filter
    (normalization) and mask filter. Each filter is used when a corresponding
    filter-specific option is set. Only grayscale images are supported.

  OPTIONS:

      -m[file], --mask=[file]           Apply mask to every image. The mask size
                                        must match the sizes of all images.
      -s[subtraction factor],
      --static-noise=[subtraction
      factor]                           Static background subtraction factor.
                                        Computes an average of all images (from
                                        single folder) and subtracts this
                                        average from each image with given
                                        factor. You may use positive floating
                                        point values for the factor.
      Rescaling range [b, e] for
      contrast filter. Pixel values in
      range [b, e] will be mapped to
      [0, 255]:
        -b[range begin],
        --rescale-begin=[range begin]
        -e[range end],
        --rescale-end=[range end]
      -f[filename regex],
      --fname-regex=[filename regex]    If specified, only files matching given
                                        regex will be processed. Default value
                                        is ".*\.(jpe?g|gif|tif|tiff|png)"
                                        (matches common image files). Use
                                        ECMAScript regex syntax.
      -p[filtered folder prefix],
      --folder-prefix=[filtered folder
      prefix]                           Specifies prefix of subfolder that will
                                        hold filtered images. Default value is
                                        "fltrd"
      -i, --incl-fltrd                  If set folders found during recursive
                                        traversal starting with folder-prefix
                                        (specified with option -p) will be
                                        processed. By default these folders are
                                        ignored so that they don't get filtered
                                        twice.
      -r, --recursive                   Process input folders recursively.
      --ram-friendly                    By default all images from a single
                                        folder are stored in memory when the
                                        folder is being processed. If that is
                                        not possible due to small RAM space, use
                                        this flag.
      folders...                        List of folders to process.
      -h, --help                        Display this help menu
      "--" can be used to terminate flag options and force all following
      arguments to be treated as positional options

    Visit the project page for further information:
    https://github.com/romeritto/hranol
```

### Basic filtering
```
$ hranol -m "examples/mask.png" -s 1.3 -b 5 -e 9 -r examples/particles
```
This command recursively (`-r` flag) processes `particles` folder, **masking** each image with `mask.png`. It also uses **Background subtraction** with factor `1.3` and **Contrast filter** with range *[5, 9]*. Results can be found in folders `examples/particles/run1/fltrd_run1` and `examples/particles/run1/fltrd_run2`.

If any folder starts with prefix `fltrd`, it is ignored. That means running this very same command twice would not process `examples/particles/run1/fltrd_run1` and `examples/particles/run1/fltrd_run2` folders. You can override this behaviour with flag `-i`.

### Using regex for image names
In `examples/monitor` we would wish to run following command:
```
$ hranol -m "examples/monitor/mask.bmp" examples/monitor`
```
The above command applies `mask.bmp` to images in `examples/monitor`. However, since `mask.png` is located in `monitor` folder it would also get filtered. This unwanted and `mask.png` can either be moved away from the `monitor` directory or we can use `-f[regex]` option to specify a regex that all processed filenames have to match:
```
$ hranol -m "examples/monitor/mask.bmp" -f "(?!^mask.bmp$)" examples/monitor`
```
Use [ECMAScript regex syntax](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Regular_Expressions).

### Changing output folder prefix
```
$ hranol -s1 -p"bckg_rem" -f"(?!^mask.bmp$)" --ram-friendly examples/monitor
```
The command removes static background with factor `1` and stores the result in `examples/monitor/bckg_rem_monitor` folder. It is also run in `ram-friendly` mode which means that while precomputing the average of all images for background subtraction filter, the images are not kept in RAM. Thus, when the images are actually filtered (when the average is subtracted), the images have to be loaded from disk again.
