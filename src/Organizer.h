#pragma once
#ifndef _ORGANIZER_
#define _ORGANIZER_

#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include<fstream>
#include<string>
#include<exception>

class Organizer
{
private:
	std::string pathTofile;
	std::string nameOfFile;
	std::string imagesFolder = "images\\";
public:
	int imgs;
	//int txts;

	std::vector<int> imgIndex;
	std::vector<cv::Mat> images;
	//std::vector<int> txtIndex;

	Organizer(std::string path, std::string name);
	~Organizer();
	//setMetode
	std::vector<int> getImg() {
		return imgIndex;
	}

	std::vector<cv::Mat> getImages() {
		return images;
	}

	int getImagesCount() {
		return images.size();
	}
	//--------------------------------------
	void init();
	void readImages();


	//--------------------------------------


};


#endif // !_ORGANIZER_




