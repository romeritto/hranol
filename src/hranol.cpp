//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
// 

#include "../thirdparty/args/args.hxx"
#include "Filter.h"
#include "FolderCrawler.h"
#include "ImageProcessor.h"
#include "ImageStore.h"

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>


class Hranol
{
    std::vector< std::string> folders_;
    bool recursive_;
    bool ram_friendly_;
    std::string fname_regex_;
    // Prefix of filtered folders
    std::string folder_prefix_;
    // Indicates whether folders starting with folder_prefix_ should be included
    bool incl_folder_prefix_; 

    ImageProcessor img_processor_;

public:
    // Defaults
    Hranol() : 
        recursive_(false),
        ram_friendly_(false),
        fname_regex_(".*\\.(jpe?g|gif|tif|tiff|png|bmp)"),
        folder_prefix_("fltrd"),
        incl_folder_prefix_(false) { }

    void parse_from_cli(int argc, char **argv);
    void process();
};

void Hranol::parse_from_cli(int argc, char **argv) 
{
    args::ArgumentParser parser(
        "Hranol -- batch image processing utility. By default only images in given folders are processed "
        "and the output is saved to a subfolder with prefix \"" + folder_prefix_ + "\". Supported filters: "
        "static background subtraction, contrast filter (normalization) and mask filter. Each filter is used when a corresponding filter-specific "
        "option is set. Only grayscale images are supported.",
        "Visit the project page for further information: https://github.com/romeritto/hranol");
    parser.Prog(argv[0]);
    args::ValueFlag<std::string> mask_file(parser, "file",
        "Apply mask to every image. The mask size must match the sizes of all images.",
        { 'm', "mask" });
    args::ValueFlag<double> subtraction_factor(parser, "subtraction factor",
        "Static background subtraction factor. Computes an average of all images (from single folder) and "
        "subtracts this average from each image with given factor. You may use positive floating point "
        "values for the factor.",
        { 's', "static-noise" });
    args::Group rescale(parser, "Rescaling range [b, e] for contrast filter. Pixel values in range [b, e] will be mapped to [0, 255]:");
    args::ValueFlag<int> rescale_beg(rescale, "range begin",
        "",
        { 'b', "rescale-begin" });
    args::ValueFlag<int> rescale_end(rescale, "range end",
        "",
        { 'e', "rescale-end" });
    args::ValueFlag<std::string> fname_regex(parser, "filename regex",
        "If specified, only files matching given regex will be processed. Default value is \"" + fname_regex_ + "\" "
        "(matches common image files). Use ECMAScript regex syntax.",
        {'f', "fname-regex"});
    args::ValueFlag<std::string> folder_prefix(parser, "filtered folder prefix",
        "Specifies prefix of subfolder that will hold filtered images. Default value is \"" + folder_prefix_ + "\"",
        { 'p', "folder-prefix" });
    args::Flag incl_folder_prefix(parser, "include filtered folders",
        "If set folders found during recursive traversal starting with folder-prefix (specified with "
        "option -p) will be processed. By default these folders are ignored so that they don't "
        "get filtered twice.",
        { 'i', "incl-fltrd" });
    args::Flag recursive(parser, "recursive", 
        "Process input folders recursively.",
        { 'r', "recursive" });
    args::Flag ram_friendly(parser, "ram friendly",
        "By default all images from a single folder are stored in memory when the folder is being processed. If that is not possible "
        "due to small RAM space, use this flag.",
        { "ram-friendly" });
    args::PositionalList<std::string> folders(parser, "folders", "List of folders to process.");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });

    try {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help & e) {
        std::cout << parser;
        throw;
    }

    folders_ = std::vector<std::string>(args::get(folders));

    if (recursive)
        recursive_ = true;

    if (ram_friendly)
        ram_friendly_ = true;
    
    if (fname_regex)
        fname_regex_ = args::get(fname_regex);

    if (folder_prefix)
        folder_prefix_ = args::get(folder_prefix);

    if (incl_folder_prefix)
        incl_folder_prefix_ = true;

    if (mask_file)
        img_processor_.add_filter(std::move(MaskFilter::create(args::get(mask_file))));
    
    if (subtraction_factor)
        img_processor_.add_filter(std::move(BckgSubFilter::create(args::get(subtraction_factor))));

    if (rescale_beg || rescale_end)
    {
        if (rescale_beg && rescale_end)
            img_processor_.add_filter(std::move(ContrastFilter::create(
                args::get(rescale_beg),
                args::get(rescale_end)
            )));
        else
            throw HranolRuntimeException("Both range begin and end must be specified for rescale filter.");
    }
}

void Hranol::process()
{
    FolderCrawler crawler(
        folders_,
        folder_prefix_,
        fname_regex_,
        incl_folder_prefix_,
        recursive_,
        ram_friendly_
    );
    while (crawler.has_next_run())
    {
        try {
            auto store = crawler.get_next_run();
            img_processor_.apply_filters(store.get());
        }
        catch (const std::exception &e) {
            std::cout << "\nError (skipping run):\n" << e.what() << std::endl;
        }
    } 
}

int main(int argc, char **argv)
{

    Hranol hranol;
    try {
        hranol.parse_from_cli(argc, argv);
        hranol.process();
    }
    catch (const args::Help & e) {
        // Help page was requested 
        return 0;
    }
    catch (const args::Error & e) {
        // An exception was thrown while processing arguments
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return 1;
    }
}
