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


class Hranol {
	std::vector<std::string> folders_;
	bool recursive_;
	bool ram_friendly_;
	std::string fname_regex_;
	// Prefix of filtered folders
	std::string folder_prefix_;
	// Indicates whether folders starting with folder_prefix_ should be included
	bool incl_folder_prefix_; 

	ImageProcessor img_processor_;

public:
	Hranol() : recursive_(false), ram_friendly_(false), incl_folder_prefix_(false) { }

	void parse_from_cli(int argc, char **argv);
	void process();
};

void Hranol::parse_from_cli(int argc, char **argv) 
{
	args::ArgumentParser parser(
		"This is a batch image filtering utility. Filters supported are: TODO",
		"Please report any issues to the Github page: TODO");
	parser.Prog(argv[0]);
	args::ValueFlag<std::string> mask_file(parser, "file",
		"Apply a mask to every image. The mask size must match the sizes of all images.",
		{ 'm', "mask" });
	args::ValueFlag<double> static_bckg_rem_factor(parser, "factor",
		"The static background removal factor. You may use positive floating point values.",
		{ 's', "static-noise" });
	args::Group rescale(parser, "Rescaling range [b, e]:", args::Group::Validators::AllOrNone);
	args::ValueFlag<int> rescale_beg(rescale, "range begin",
		"",
		{ 'b', "rescale-begin" });
	args::ValueFlag<int> rescale_end(rescale, "range end",
		"",
		{ 'e', "rescale-end" });
	args::ValueFlag<std::string> fname_regex(parser, "filename regex",
		"If specified, only files matching given regex will be processed. Default TODO",
		{'f', "fname-regex"});
	args::ValueFlag<std::string> folder_prefix(parser, "filtered folder prefix",
		"Specifies prefix of a folder that will hold filtered images. Default value is \"fltrd_\"",
		{ 'p', "folder-prefix" });
	args::Flag incl_folder_prefix(parser, "include filtered folders",
		"If set folders found during recursive taversal starting with folder-prefix (specified with "
		"option -p) will be filtered. By default these folders are ignored so that folders don't "
		"get filtered twice.",
		{ 'i', "incl-fltrd" });
	args::Flag recursive(parser, "recursive", 
		"Process the input folders recursively, excluding folders with the prefix \"TODO\"",
		{ 'r', "recursive" });
	args::Flag ram_friendly(parser, "ram friendly",
		"By default all images from a single folder are stored in memory when the folder is being processed. If that is not possible, "
		"use this flag.",
		{ "ram-friendly" });
	args::PositionalList<std::string> folders(parser, "folders", "The list of folders to process.");
	args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });

	try
	{
		parser.ParseCLI(argc, argv);
	}
	catch (args::Help)
	{
		std::cout << parser;
		throw;
	}
	catch (const args::Error &e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		throw;
	}

	folders_ = std::vector<std::string>(args::get(folders));

	if (recursive)
		recursive_ = true;

	if (ram_friendly)
		ram_friendly_ = true;
	
	if (fname_regex)
		fname_regex_ = args::get(fname_regex);
	else
		fname_regex_ = ".*";

	if (folder_prefix)
		folder_prefix_ = args::get(folder_prefix);
	else
		folder_prefix_ = "fltrd_";

	if (incl_folder_prefix)
		incl_folder_prefix_ = true;

	if (mask_file)
		img_processor_.add_filter(std::move(MaskFilter::create(args::get(mask_file))));
	if (static_bckg_rem_factor)
		img_processor_.add_filter(std::move(StaticBckgFilter::create(args::get(static_bckg_rem_factor))));
	if (rescale_beg || rescale_end)
	{
		if (rescale_beg && rescale_end)
			img_processor_.add_filter(std::move(RescaleFilter::create(
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
		std::move(folder_prefix_),
		std::move(fname_regex_),
		incl_folder_prefix_,
		recursive_,
		ram_friendly_
	);
	while (crawler.has_next_run())
	{
		try 
		{
			auto store = crawler.get_next_run();
			img_processor_.apply_filters(store.get());
		}
		catch (const std::exception &e)
		{
			std::cout << "\nSkipping run:\n" << e.what() << std::endl;
		}
	} 
}

int main(int argc, char **argv)
{

	Hranol hranol;
	try 
	{
		hranol.parse_from_cli(argc, argv);
		hranol.process();
	}
	catch (args::Help)
	{
		// A help page was requested 
		return 0;
	}
	catch (args::Error)
	{
		// An exception was thrown while processing arguments
		// Error message is already displayed in Batcher class
		return 1;
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << "Unknown error" << std::endl;
		return 1;
	}

	/*
	for (int i = 0; i <= 39; ++i) {
		std::string x = std::to_string(i);
		for (int j = x.size(); j < 2; ++j)
			x = "0" + x;
		if (i % 10 == 0)
			std::cout << "\r" << i << std::endl;
		cv::Mat m1 = cv::imread("C:/Users/roman/Downloads/Run 13-56-55/13-56-55_colormap_4-10/fltrd_13-56-55_orig/13-56-55_orig" + x + ".png", cv::ImreadModes::IMREAD_GRAYSCALE);
		cv::Mat m2 = cv::imread("C:/Users/roman/Downloads/Run 13-56-55/13-56-55_colormap_4-10/13-56-55_colormap" + x + ".png", cv::ImreadModes::IMREAD_GRAYSCALE);

		cv::Mat dst;
		cv::subtract(m1, m2, dst);
		std::cout << cv::countNonZero(dst) << " " << cv::sum(dst) << std::endl;

		for (int i = 0; i < m1.rows; ++i)
			for (int j = 0; j < m1.cols; ++j) {
				if (abs(m1.at<uchar>(i, j) - m2.at<uchar>(i, j)) >= 2) {
					std::cout << (int)m1.at<uchar>(i, j) << " " << (int)m2.at<uchar>(i, j) << std::endl;
				}
			}

		getchar();
	}
	*/
}

/*
int main(int argc, char **argv)
{
	Hranol hranol;
	try {
		hranol.parse_from_cli(argc, argv);
	}
	catch (args::Help) {
		// A help page was requested 
		return 0;
	}
	catch (args::Error) {
		// An exception was thrown while processing arguments
		// Error message is already displayed in Batcher class
		return 1;
	}
	catch (const std::invalid_argument &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	
	MaskFilter mask("C:/Users/roman/Downloads/Run 17-28-19/mask.png");
	cv::Mat m1, m2 = cv::imread("C:/Users/roman/Downloads/Run 17-28-19/mask.png", cv::ImreadModes::IMREAD_GRAYSCALE);
	StaticBckgFilter f(2);

	clock_t begin = clock();

	for (int i = 0; i <= 39; ++i) {
		std::string x = std::to_string(i);
		for (int j = x.size(); j < 2; ++j)
			x = "0" + x;
		if (i % 10 == 0)
			std::cout << "\r" << i << std::endl;
		m1 = cv::imread("C:/Users/roman/Downloads/Run 13-56-55/13-56-55_orig/13-56-55_orig" + x + ".png", cv::ImreadModes::IMREAD_GRAYSCALE);
		f.precompute_from(m1);
	}

	clock_t end = clock();
	std::cout << double(end - begin) / CLOCKS_PER_SEC << std::endl;

	for (int i = 0; i <= 39; ++i) {
		std::string x = std::to_string(i);
		for (int j = x.size(); j < 2; ++j)
			x = "0" + x;
		if (i % 10 == 0)
			std::cout << "\r" << i << std::endl;
		m1 = cv::imread("C:/Users/roman/Downloads/Run 13-56-55/13-56-55_orig/13-56-55_orig" + x + ".png", cv::ImreadModes::IMREAD_GRAYSCALE);
		
		cv::Mat m3;
		m3 = m1.clone();
		
		f.apply_to(m1);
		m2 = cv::imread("C:/Users/roman/Downloads/Run 13-56-55/13-56-55_rm_avg_2/13-56-55_rm_avg" + x + ".png", cv::ImreadModes::IMREAD_GRAYSCALE);
		
		cv::Mat dst;
		cv::subtract(m2, m1, dst);
		std::cout << cv::countNonZero(dst) << " " << cv::sum(dst) << std::endl;
		
		for (int i = 0; i < m1.rows; ++i)
			for (int j = 0; j < m1.cols; ++j) {
				if (abs(m1.at<uchar>(i, j) - m2.at<uchar>(i, j)) >= 3) {
					std::cout << (int) m1.at<uchar>(i, j) << " " << (int) m2.at<uchar>(i, j) << std::endl;
					std::cout << (int) m3.at<uchar>(i, j) << std::endl;
				}
			}
		
		getchar();
	}
	
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	cv::imshow("Display window", m1);                   // Show our image inside it.
	cv::waitKey(0);
	getchar();
 
	return 0;
}
*/