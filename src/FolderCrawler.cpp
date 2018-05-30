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
#include <string>
#include <vector>
#include <memory>

using namespace std;
namespace fs = std::filesystem;

fs::path find_subfolder_dest(const fs::path & cur_path, const string & dest_folder_prefix)
{
    string origin_fname = fs::canonical(cur_path).filename().string();

    // Finds destination folder name.
    auto dest = cur_path / (dest_folder_prefix + "_" + origin_fname);

    int i = 0;
    while (i < 100 && fs::exists(dest))
    {
        // yeah, nasty padding
        string idx = ((i < 10) ? "0" : "") + to_string(i);
        dest = dest / (dest_folder_prefix + idx + "_" + origin_fname);
        ++i;
    }

    // All possible names were used
    if (fs::exists(dest))
        throw HranolRuntimeException("Suitable name for output directory could not be found.");

    return dest;
}

unique_ptr< IImageStore> FolderCrawler::get_next_run()
{
    auto cur_pp = crawl_stack_.top();
    crawl_stack_.pop();
    
    fs::path cur_path = base_folders_[cur_pp.base_idx] / cur_pp.path;
    if (!fs::is_directory(cur_path))
        throw HranolRuntimeException("Path \"" + cur_path.string() + "\" is not a directory.");
    
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

            crawl_stack_.emplace(cur_pp.base_idx, cur_pp.path / f.path().filename()) ;
        }
        else if (fs::is_regular_file(f.status())) 
        {
            if (regex_match(f.path().filename().string(), fname_regex_))
                img_paths.push_back(f.path());
        }
    }
    
    std::filesystem::path dest;
    if (!output_folder_.empty())  // use output_folder
        dest = output_folder_ / fs::canonical(base_folders_[cur_pp.base_idx]).filename() / cur_pp.path;
    else                          // use subfolder
        dest = find_subfolder_dest(cur_path, folder_prefix_);

    dest = dest.lexically_normal();
    cur_path = cur_path.lexically_normal();

    if (ram_friendly_)
        return make_unique< OnDemandImageStore>(cur_path, dest, std::move(img_paths));
    else 
        return make_unique< RAMImageStore>(cur_path, dest, std::move(img_paths));
}