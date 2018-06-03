//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
// 

#ifndef IMAGE_STORE_H
#define IMAGE_STORE_H

#include "opencv2/core/mat.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <memory>


// IImageStore is an interface class for accessing the images that will be filtered.
//
// You can access a single image using load() method and save the changes using save().
// After finishing the work with the image, you have to release() it, otherwise you can't
// load() another image.
// It is designed this way because OnDemandImageStore can store only one image. If you
// release() the current one, you can load() another one.
class IImageStore 
{
protected:
    std::filesystem::path origin_;
    std::filesystem::path dest_;
    bool dest_created_;
    const std::vector< std::filesystem::path> img_paths_;

    cv::Mat read_img(const std::filesystem::path & s);
    void save_img(cv::Mat img, const std::filesystem::path & img_src);
    void create_dest();

public:
    IImageStore(
        std::filesystem::path origin,
        std::filesystem::path dest,
        std::vector< std::filesystem::path> img_paths)
        : origin_(std::move(origin)), dest_(std::move(dest)), dest_created_(false),
        img_paths_(std::move(img_paths))
    {}

    virtual ~IImageStore() { }

    virtual size_t size() const {
        return img_paths_.size();
    }

    std::filesystem::path get_origin() const {
        return origin_;
    }

    std::filesystem::path get_dest() const {
        return dest_;
    }

    std::string get_img_path(size_t i) const;
    
    virtual cv::Mat & load(size_t i) = 0;
    virtual void release(size_t i) = 0;
    virtual void save(size_t i) = 0;
};


// RAMImageStore loads all of the images to imgs_ vector. Therefore multiple gets of the same
// image are fast.
class RAMImageStore : public IImageStore
{
    std::vector< cv::Mat> imgs_;

public:
    RAMImageStore(std::filesystem::path origin,
        std::filesystem::path dest,
        std::vector< std::filesystem::path> img_paths)
        : IImageStore(std::move(origin), std::move(dest), std::move(img_paths))
    { 
        imgs_.resize(img_paths_.size());
    }
    virtual cv::Mat & load(size_t i);
    virtual void release(size_t i);
    virtual void save(size_t i);
};


// OnDemandImageStore loads only one image. When an image is loaded, subsequent load() of a different
// image from the loaded one must be preceded with release() of the currently loaded image.
class OnDemandImageStore : public IImageStore
{
    cv::Mat loaded_img_;
    size_t loaded_img_idx_;
    bool is_img_loaded_;

public:
    OnDemandImageStore(std::filesystem::path origin,
        std::filesystem::path dest,
        std::vector< std::filesystem::path> img_paths)
        : IImageStore(std::move(origin), std::move(dest), std::move(img_paths)), is_img_loaded_(false)
         {}
    
    virtual cv::Mat & load(size_t i);
    virtual void release(size_t i);
    virtual void save(size_t i);
};

#endif // IMAGE_STORE_H
