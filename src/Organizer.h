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
	std::string pathToTxtsFile;
	
	int level;
public:
	int imgs;
	int m_minImageNum;
	std::vector<int> m_bindexes;
	int m_useBound;
	//int txts;

	std::vector<int> imgIndex; // U pmvs-u je ovo m_timages
	std::vector<int> m_oimages;
	std::vector<cv::Mat> images;
	std::vector<std::vector<int> > m_visdatatemp;
	std::vector<std::vector<int> > m_visdata2temp;
	//std::vector<int> txtIndex;
	std::string setTxtFileName(std::string nameInput);
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
		return (int)images.size();
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

	std::string getPathToTxt() {
		return pathToTxtsFile;
	}
	int getLevel() {
		return level;
	}
	//--------------------------------------
	void init();
	void readImages();
	void getFileFormDirectory(std::string pathToFolder, std::string extension, std::vector<std::string>& returnFileNameList);
	void initVisdata(void);
	void initVisdata2(void);
	//--------------------------------------


};


#endif // !_ORGANIZER_




