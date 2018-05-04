//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
// 

#include "ImageProcessor.h"
#include "ImageStore.h"
#include "Filter.h"

#include "opencv2/core/core.hpp"

#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip> // put_time
#include <chrono>

using namespace std;

void ImageProcessor::apply_filters(IImageStore * imstore)
{
	// Print currently processing folder 
	cout << "\"" << imstore->get_origin().string() << "\":" << endl;

	// Clear precomputed data from previous run in precomp_filters
	// Pure filters do not store any run-specific data so they do not
	// have to be cleared
	for (auto&& of : precomp_filters_)
		of->clear();

	auto store_sz = imstore->size();
	 
	// If there is no work to do, return
	if (store_sz == 0)
		return;

	if (!precomp_filters_.empty()) 
	{
		for (size_t i = 0; i < store_sz; ++i)
		{
			cout << "\r\tPrecomputing: " << to_string(i + 1) << " / " << to_string(store_sz) << flush;
			try 
			{
				cv::Mat & img = imstore->get(i);

				for (auto&& of : precomp_filters_) 
					of->precomp_from(img);
				
				imstore->release(i);
			}
			catch (HranolException &e)
			{
				e.append("\nPrecomputing failed for image: " + imstore->get_img_path(i));
				throw;
			}
		}
		// Endline after "Precopmuting: ..." message 
		cout << endl;
	}

	for (size_t i = 0; i < imstore->size(); ++i)
	{
		cout << "\r\tFiltering: " << to_string(i + 1) << " / " << to_string(store_sz) << flush;
		try 
		{
			cv::Mat & img = imstore->get(i);

			for (auto&& of : precomp_filters_)
				of->apply_to(img);

			for (auto&& of : pure_filters_)
				of->apply_to(img);
			
			imstore->save(i);
			imstore->release(i);
		}
		catch (HranolException &e)
		{
			e.append("\nApplying filter(s) failed for image: " + imstore->get_img_path(i));
			throw;
		}
	}
	// Endline after "Filtering: ..." message
	cout << endl;

	create_log_(imstore);	
}

void ImageProcessor::create_log_(const IImageStore * imstore)
{
	auto log_path = imstore->get_dest() / "fltrd_info.txt";
	ofstream log(log_path, ofstream::out);

	// Print time
	auto now = chrono::system_clock::now();
	auto now_c = chrono::system_clock::to_time_t(now);
	log << "Filtered on " << std::put_time(std::localtime(&now_c), "%c") << endl;
	
	// Filters used
	log << "Filters used: " << endl;
	for (auto&& of : precomp_filters_)
		log << " - " << of->desc() << endl;

	for (auto&& of : pure_filters_)
		log << " - " << of->desc() << endl;

	log.close();
}
