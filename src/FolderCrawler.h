//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
// 

#ifndef FOLDER_CRAWLER_H
#define FOLDER_CRAWLER_H

#include "ImageStore.h"

#include <regex>
#include <vector>
#include <string>
#include <filesystem>
#include <stack>
#include <memory>

// FolderCrawler is used to crawl (potentially recursively) folders in crawl_stack_ and picks
// the files that should be filtered.
class FolderCrawler {
    // PathPair contains index to base_folders_ vector and path extension from this base 
    struct PathPair {
        size_t base_idx;
        std::filesystem::path path;

        PathPair(size_t base_idx_p, std::filesystem::path path_p)
            : base_idx(base_idx_p), path(std::move(path_p)) { }
    };

    bool recursive_;
    bool ram_friendly_;
    std::regex fname_regex_;
    std::string output_folder_;
    std::string folder_prefix_;
    bool incl_folder_prefix_;

    std::vector< std::string> base_folders_;
    std::stack< PathPair> crawl_stack_;

public:
    FolderCrawler(
        std::vector< std::string> folders,
        std::string output_folder,  // get-by-value because it will be moved
        std::string folder_prefix,  // same as above
        const std::string & fname_regex_str,
        bool incl_folder_prefix,
        bool recursive, 
        bool ram_friendly) 
        : recursive_(recursive), ram_friendly_(ram_friendly), fname_regex_(fname_regex_str),
        output_folder_(std::move(output_folder)), folder_prefix_(std::move(folder_prefix)),
        incl_folder_prefix_(incl_folder_prefix), base_folders_(std::move(folders))

    {
        // Loop is reversed so that first folders in the original vector
        // will be first in the stack
        int folders_count = (int) base_folders_.size();
        for (int i = folders_count - 1; i >= 0; --i)
            crawl_stack_.emplace((size_t) i, ".");    // "." to represent origin itself
    }

    // Inspects a single folder and returns IImageStore that contains
    // all files from the folder that matched fname_regex_
    std::unique_ptr< IImageStore> get_next_run();

    bool has_next_run() {
        return !crawl_stack_.empty();
    }
};

#endif // FOLDER_CRAWLER_H
