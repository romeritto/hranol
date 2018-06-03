//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
//

#ifndef FILTER_H
#define FILTER_H

#include "HranolException.h"

#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <memory>
#include <vector>
#include <string>


class IFilter
{
public:
    virtual void apply_to(cv::Mat & img) = 0;
    // Returns string describing particular filter
    virtual std::string desc() const = 0;
    virtual ~IFilter() { };
};


// Interface for pure filters
// Pure filters do not need precomputation and have no side effects
class IFilterPure : public IFilter {};


// Interface for filters that need precomputation
class IFilterWithPrecomp : public IFilter
{
public:
    // Clears precomputed data
    virtual void clear() = 0;
    virtual void precomp_from(const cv::Mat img) = 0;
};

using PureFiltersVec = std::vector< std::unique_ptr< IFilterPure>>;
using PrecompFiltersVec = std::vector< std::unique_ptr< IFilterWithPrecomp>>;


// Filters

class MaskFilter : public IFilterPure
{
    std::string mask_fname_;
    cv::Mat mask_;

public:
    MaskFilter(std::string mask_fname)
        : mask_fname_(std::move(mask_fname))
    {
        mask_ = cv::imread(mask_fname_, cv::ImreadModes::IMREAD_GRAYSCALE);
        if (mask_.empty())
            throw HranolRuntimeException("Unable to open mask filter: \"" + mask_fname_ + "\"");
    }

    static auto create(std::string mask_fname) 
    {
        return std::make_unique< MaskFilter>(std::move(mask_fname));
    }

    virtual void apply_to(cv::Mat &img) 
    {
        cv::Mat dst;

        if (img.size() != mask_.size())
            throw HranolRuntimeException("Size or number of channels of image being masked and the mask did not match.");

        img.copyTo(dst, mask_);
        img = dst;
    }

    virtual std::string desc() const {
        return "Mask with source " + mask_fname_;
    }
};

// Contrast filter maps colors in range [beg_, end_] to [0, 255]
class ContrastFilter : public IFilterPure
{
    // Rescale range [beg_, end_]
    int beg_, end_;
    cv::Mat lut_;

public:
    ContrastFilter(int beg, int end)
        : beg_(beg), end_(end)
    {
        if (beg_ < 0 || end_ > 255 || beg_ > end_)
            throw HranolRuntimeException("Invalid range for contrast filter " + range_to_str_(beg_, end_));
    }

    static auto create(int beg, int end) {
        return std::make_unique< ContrastFilter>(beg, end);
    }

    virtual void apply_to(cv::Mat & img)
    {
        // Only char type matrices can be filtered with LUT
        if (img.depth() != CV_8U)
            throw HranolRuntimeException("ContrastFilter can only be applied to char type (grayscale) matrices.");

        if (lut_.empty())
            fill_lut_();

        cv::LUT(img, lut_, img);
    }

    virtual std::string desc() const {
        return "Contrast filter with range " + range_to_str_(beg_, end_);
    }

private:
    void fill_lut_()
    {
        lut_ = cv::Mat(1, 256, CV_8U);
        uchar* p_lut = lut_.ptr();
        for (int i = 0; i < 256; ++i)
        {
            if (i < beg_)
                p_lut[i] = 0;
            else if (i > end_)
                p_lut[i] = 255;
            else
                p_lut[i] = ((i - beg_ + 1) * 255) / (end_ - beg_ + 2);
        }
    }

    std::string range_to_str_(int b, int e) const {
        return "[" + std::to_string(b) + ", " + std::to_string(e) + "]";
    }
};


// BckgSubFilter subtracts the mean value of all images with factor subtraction_factor_
class BckgSubFilter : public IFilterWithPrecomp
{
    // Number of images summed
    size_t count_;

    // Accumulator to hold the running sum
    cv::Mat accumulator_;

    // The apply_to method uses this precomputed factored mean from aggregated data
    cv::Mat factored_mean_;
    bool is_factored_mean_valid_;

    double subtraction_factor_;

public:
    BckgSubFilter(double subtraction_factor) :
        count_(0), is_factored_mean_valid_(false), subtraction_factor_(subtraction_factor)
    {
        if (subtraction_factor <= 0)
            throw HranolRuntimeException("Background subtraction factor must be positive: " + std::to_string(subtraction_factor));
    }

    static auto create(double subtraction_factor) {
        return std::make_unique<BckgSubFilter>(subtraction_factor);
    }

    virtual void apply_to(cv::Mat &img)
    {
        // Do nothing if there were no images in the precomputation
        if (count_ == 0)
            return;

        if (!is_factored_mean_valid_)
        {
            // Intention: subtraction_factor_ * (accumulator_ / count_)
            // If the above equation was used, accumulator_ would have to be traversed twice
            factored_mean_ = accumulator_ / (count_ / subtraction_factor_);
            factored_mean_.convertTo(factored_mean_, CV_8U);
            is_factored_mean_valid_ = true;
        }

        if (img.size() != factored_mean_.size() || img.channels() != factored_mean_.channels())
            throw HranolRuntimeException("Size or number of channels channels of processed image and images used for precomputation did not match.");

        // Saturated subtraction
        img -= factored_mean_;
    }

    virtual void precomp_from(const cv::Mat img)
    {
        is_factored_mean_valid_ = false;
        ++count_;

        // Allocate new accumulator based on the size of input image
        if (accumulator_.empty())
            accumulator_ = cv::Mat::zeros(img.rows, img.cols, CV_32FC(img.channels()));

        if (img.size() != accumulator_.size() || img.channels() != accumulator_.channels())
            throw HranolRuntimeException("Size or number of channels of preprocessed image and accumulator did not match.");

        cv::accumulate(img, accumulator_);
    }

    virtual void clear()
    {
        count_ = 0;
        accumulator_ = cv::Mat();
        factored_mean_ = cv::Mat();
        is_factored_mean_valid_ = false;
    }

    virtual std::string desc() const {
        return "Background subtraction with factor " + std::to_string(subtraction_factor_);
    }
};

#endif // FILTER_H
