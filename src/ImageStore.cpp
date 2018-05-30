//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
// 

#include "ImageStore.h"
#include "HranolException.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <string>
#include <filesystem>
#include <stdexcept>
#include <cassert>

using namespace std;
namespace fs = std::filesystem;

inline bool validate_idx(size_t i, size_t sz)
{
    // i >= 0 is satisfies because i is of type size_t 
    return i < sz;
}

std::string IImageStore::get_img_path(size_t i) const
{
    assert(validate_idx(i, this->size()));
    return img_paths_[i].string();
}

cv::Mat IImageStore::read_img(const fs::path & p)
{
    cv::Mat ret = cv::imread(p.string(), cv::ImreadModes::IMREAD_GRAYSCALE);
    if (ret.empty())
        throw HranolRuntimeException("Reading image: \"" + p.string() + "\" failed.");

    return ret;
}

void IImageStore::save_img(const cv::Mat img, const fs::path & img_src)
{
    if (!dest_created_)
        create_dest();

    fs::path img_dest = dest_ / img_src.filename().string();
    try {
        cv::imwrite(img_dest.string(), img);
    }
    catch (const runtime_error & e) {
        throw HranolRuntimeException("Writing image: \"" + img_dest.string() + "\" failed: " + e.what());
    }
}

void IImageStore::assign_dest(const string & dest_folder_prefix)
{
    string origin_fname = fs::canonical(
        fs::absolute(origin_)
    ).filename().string();

    // Finds destination folder name.
    dest_ = origin_ / (dest_folder_prefix + "_" + origin_fname);
    int i = 0;
    while (i < 100 && fs::exists(dest_))
    {
        // yeah, nasty padding
        string idx = ((i < 10) ? "0" : "") + to_string(i);
        dest_ = origin_ / (dest_folder_prefix + idx + "_" + origin_fname);
        ++i;
    }

    // All possible names were used
    if (fs::exists(dest_))
        throw HranolRuntimeException("Suitable name for output directory could not be found.");
}

void IImageStore::create_dest()
{
    if (dest_created_)
        return;

    fs::create_directories(dest_);
    dest_created_ = true;
}

cv::Mat & RAMImageStore::load(size_t i)
{
    assert(validate_idx(i, this->size()));

    if (imgs_[i].empty())
        imgs_[i] = read_img(img_paths_[i]);

    return imgs_[i];
}

void RAMImageStore::release(size_t i)
{
    assert(validate_idx(i, this->size()));
    // All images are stored in RAM so release does nothing
}

void RAMImageStore::save(size_t i)
{
    validate_idx(i, this->size());
    save_img(imgs_[i], img_paths_[i]);
}

cv::Mat & OnDemandImageStore::load(size_t i)
{
    assert(validate_idx(i, this->size()));
    assert(!is_img_loaded_ || i == loaded_img_idx_);

    if (!is_img_loaded_)
    {
        loaded_img_ = read_img(img_paths_[i]);
        loaded_img_idx_ = i;
        is_img_loaded_ = true;
    }

    return loaded_img_;
}

void OnDemandImageStore::release(size_t i)
{
    assert(validate_idx(i, this->size()));
    assert(i == loaded_img_idx_);

    is_img_loaded_ = false;
}

void OnDemandImageStore::save(size_t i)
{
    assert(validate_idx(i, this->size()));
    assert(is_img_loaded_ && i == loaded_img_idx_);

    save_img(loaded_img_, img_paths_[loaded_img_idx_]);
}
