//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
// 

#include "FolderCrawler.h"
#include "ImageStore.h"
#include "HranolException.h"

#include <filesystem>
#include <iostream>
#include <regex>
#include <vector>
#include <memory>

using namespace std;
namespace fs = std::filesystem;

unique_ptr< IImageStore> FolderCrawler::get_next_run()
{
    if (folders_.empty())
        return nullptr;

    fs::path cur_path = folders_.top();
    folders_.pop();
    
    if (!fs::is_directory(cur_path))
        throw HranolRuntimeException("Path \"" + cur_path.string() + "\" is not a directory.");
    
    string cur_path_fname = cur_path.filename().string();
    vector< fs::path> img_paths;

    for (auto&& f : fs::directory_iterator(cur_path))
    {
        if (fs::is_directory(f.status()))
        {
            if (!recursive_)
                continue;

            // Skips directory if it starts with the folder_prefix_
            // Trick used:
            // rfind(key, 0) returns the position of the first character of the last match
            // starting at posisition 0. If no match is found, string::npos is returned.
            if (!incl_folder_prefix_ && f.path().filename().string().rfind(folder_prefix_, 0) == 0)
                continue;

            folders_.push(f.path());
        }
        else if (fs::is_regular_file(f.status())) 
        {
            if (regex_match(f.path().filename().string(), fname_regex_))
                img_paths.push_back(f.path());
        }
    }
    
    if (ram_friendly_) 
        return make_unique< OnDemandImageStore>(cur_path.string(), std::move(img_paths), folder_prefix_);
    else 
        return make_unique< RAMImageStore>(cur_path.string(), std::move(img_paths), folder_prefix_); 
}