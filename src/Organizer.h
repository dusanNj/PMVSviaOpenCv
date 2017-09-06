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
#include<Windows.h>

class Organizer
{
private:
	std::string pathTofile;
	std::string nameOfFile;
	std::string imagesFolder = "images\\";
	std::string pathTofolder;
	std::string extension;
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
	//getMetode

	std::string getPathTofolder() {
		return this->pathTofolder;
	}

	std::string getExtension() {
		return this->extension;
	}

	std::string getPathToFile() {
		return pathTofile;
	}
	std::string  getFileName() {
		return nameOfFile;
	}
	//--------------------------------------
	void init();
	void readImages();
	void getFileFormDirectory(std::string pathToFolder, std::string extension, std::vector<std::string>& returnFileNameList);
	//--------------------------------------


};


#endif // !_ORGANIZER_




