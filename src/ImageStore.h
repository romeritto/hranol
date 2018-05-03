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
// You can access a single image using get() method and save the changes using save().
// After finishing the work with the image, you have to release() it, otherwise you can't
// get() another image.
// It is designed this way because OnDemandImageStore can store only one image. If you
// release() the current one, you can get() another one.
class IImageStore 
{
protected:
	using fs_path = std::experimental::filesystem::path;

	fs_path origin_;
	fs_path dest_;
	bool dest_created_;
	std::string dest_folder_prefix_;
	const std::vector<fs_path> img_paths_;

	cv::Mat read_img(const fs_path & s);
	void save_img(cv::Mat img, const fs_path & img_src);
	void assign_dest();
	void create_dest();

public:
	IImageStore(
		fs_path origin,
		std::vector<fs_path> img_paths,
		std::string folder_prefix)
		: origin_(std::move(origin)), img_paths_(std::move(img_paths)),
		dest_folder_prefix_(std::move(folder_prefix)), dest_created_(false) 
	{ 
		assign_dest();
	}

	virtual size_t size() const
	{
		return img_paths_.size();
	}

	fs_path get_origin() const
	{
		return origin_;
	}

	fs_path get_dest() const
	{
		return dest_;
	}

	std::string get_img_path(size_t i) const;
	virtual cv::Mat & get(size_t i) = 0;
	virtual void release(size_t i) = 0;
	virtual void save(size_t i) = 0;
};


// RAMImageStore loads all of the images to imgs_ vector. Therefore multiple gets of the same
// image are fast.
class RAMImageStore : public IImageStore
{
	std::vector<cv::Mat> imgs_;

public:
	RAMImageStore(std::string origin, std::vector<fs_path> img_paths, std::string folder_prefix)
		: IImageStore(std::move(origin), std::move(img_paths), std::move(folder_prefix))
	{ 
		imgs_.resize(img_paths_.size());
	}
	virtual cv::Mat & get(size_t i);
	virtual void release(size_t i);
	virtual void save(size_t i);
};


// OnDemandImageStore loads only one image. When an image is loaded, subsequent get() of a different
// image from the loaded one must be preceded with release() of the currently loaded image.
class OnDemandImageStore : public IImageStore
{
	cv::Mat loaded_img_;
	size_t loaded_img_idx_;
	bool is_img_loaded_;

public:
	OnDemandImageStore(std::string origin, std::vector<fs_path> img_paths, std::string folder_prefix)
		: IImageStore(std::move(origin), std::move(img_paths), std::move(folder_prefix)),
		is_img_loaded_(false) { }
	
	virtual cv::Mat & get(size_t i);
	virtual void release(size_t i);
	virtual void save(size_t i);
};

#endif // IMAGE_STORE_H
