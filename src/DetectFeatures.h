#pragma once
#ifndef _DETECT_FEATURES
#define _DETECT_FEATURES

#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"Cpoint.h"
#include"Harris.h"
#include"UtilityM.h"
#include<set>
#include"Dog.h"
#include"Organizer.h"
#include"Detector.h"

class DetectFeatures
{
public:
	DetectFeatures();
	~DetectFeatures();

	void run(std::string path,std::string name,
			 const int csize, const int maxLevel);

	void RunFetureDetect(void);
	//int countImageIndex(void);


	std::vector<std::vector<Cpoint> > m_points;

	int getNumOfImages() {
		return alImgChar.size();
	}

	//getMetode
	int getmCsize() {
		return m_csize;
	}
	int getmLevel() {
		return m_level;
	}

	int getWidtByIndex(int ind, int level) {
		return imgWidth[ind][level];
	}

	int getHeightByIndex(int ind, int level) {
		return imgHeight[ind][level];
	}

protected:
	//const cv::Mat *m_Image;
	std::vector<cv::Mat> *m_Images;
	cv::Mat *m_Img;
	int m_csize;
	int m_level;

	std::vector<std::vector<unsigned char>> ImagesChar;
	std::vector<std::vector<std::vector<unsigned char>>> alImgChar;
	std::vector<std::vector<unsigned char>> masksChar;
	std::vector<std::vector<unsigned char>> edgesChar;

	std::vector<std::vector<int>> imgWidth;
	std::vector<std::vector<int>> imgHeight;
};




#endif // _DETECT_FEATURES




