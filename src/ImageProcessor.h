//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
// 

#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include "ImageStore.h"
#include "Filter.h"

#include <memory>

// Stores filters and applies them to images
class ImageProcessor
{
    PureFiltersVec pure_filters_;
    PrecompFiltersVec precomp_filters_;

public:
    ImageProcessor() {}

    void add_filter(std::unique_ptr< IFilterPure> filter) {
        pure_filters_.push_back(std::move(filter));
    }

    void add_filter(std::unique_ptr< IFilterWithPrecomp> filter) {
        precomp_filters_.push_back(std::move(filter));
    }
    
    void apply_filters(IImageStore * imstore);

private:
    void create_log_(const IImageStore * imstore);
};
#endif // IMAGE_PROCESSOR_H
