# hranol
## Image processing library
I choosed to use `OpenCV` for the actual image processing in my project. The reason was that `OpenCV` adopted `C++11` features, had all the functionality I needed and has regular new releases. 

## Filesystem
For working with files and folders I decided to use new `filesystem` library from `C++17` standard. This introduced some problems, since `C++17` is not fully supported in all compilers yet. I only tested this project with `gcc 8` and `MSVC 19.14`. In `gcc 8`, `filesystem` clases are defined in `std::filesystem` namespace. However in `MSVC 19.14` these classes reside in `std::experimental::filesystem`. I dealt with this using simple preprocessor directive:
```
#ifdef _MSC_VER     // MSVC compiler
    namespace fs = std::experimental::filesystem;
#else
    namespace fs = std::filesystem;
#endif // _MSC_VER
```

## Processing arguments
I opted for using a third-party library for parsing the arguments. There's a lot of libraries to choose from, but I decided to use a library called [`args`](https://github.com/Taywee/args) simply because it was header-only and provided a lot of functionality with a lot of intuitive examples.

## Idea
The `main` function is located in `hranol.cpp` file. This file contains `Hranol` class that is responsible for parsing arguments and provides a method `process` that starts the actual image processing. Class `FolderCrawler` is used to crawl provided folders and bundles images found in each folder into an instance of `ImageStore` class. `ImageStore` contains paths to all images that should be processed in given folder. It contains methods to retrieve a single image which can be then edited and saved to a new destination. After retrieving an image from `ImageStore` we should apply filters to the image and save it. Essentially, this is exactly what `ImageProcessor` class does. First we specify which filters should be used using `add_filter` method. Then we can call `apply_filters` method that takes a pointer to `ImageStore` instance. This method applies previously specified filters to all images in provided `ImageStore` instance and saves them.

## class Hranol
Purpose of this class is to parse command line arguments using `parse` method and then based on the parsed arguments start the processing with `process` method. All parsed values are stored as data members of `Hranol` class. This is not scalable and if more options are introduced I think it would be better to use a special class that would store the parsed values.

## class Filter
I separated the logic of filters into `Filter.h` file. This file contains simple `IFilter` class which is an interface class for all filters.
```
class IFilter
{
public:
    virtual void apply_to(cv::Mat & img) = 0;
    // Returns string describing particular filter
    virtual std::string desc() const = 0;
};
```

Then I divided filters into two groups:
1. **Pure filters** that need no precomputation and have no side effects
2. **Precomputation filters** that have to go through every image of the set before being able to actually apply the filter

There are also interfaces for these two types:
```
// Interface for pure filters
class IFilterPure : public IFilter {};


// Interface for filters that need precomputation
class IFilterWithPrecomp : public IFilter
{
public:
    // Clears precomputed data
    virtual void clear() = 0;
    virtual void precomp_from(const cv::Mat img) = 0;
};
```

Note that **pure filters** do not need to extend the `IFilter` interface in any way. On the other hand `IFilterWithPrecomp` introduces `2` new methods to enable precomputation.

I thought about making `apply_to` a `const` method. However, that would mean extending the interface by more methods because currently `apply_to` often lazily sets class data members needed for `apply_to` method itself. E.g. of lazy computation from `ContrastFilter`:
```
virtual void apply_to(cv::Mat & img)
{
    // some code above
    // Lazy computation of transform matrix lut_
    if (lut_.empty())
        fill_lut_();

    // some code below
}
```
Since I wanted to keep the interfaces brief I opted for droping the `const`ness.
