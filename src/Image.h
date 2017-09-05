#pragma once
#ifndef _IMAGE_
#define _IMAGE_

#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"Detector.h"
#include<set>
#include"vec3.h"
#include<cmath>
#include"mat4.h"

class Image
{
public:
	Image();
	~Image();

	void buildImage(const int filter, int w, int h, std::vector<unsigned char>& img);
	void buildImageByLevel(const int filter, std::vector<int> w, std::vector<int> h, std::vector<std::vector<unsigned char>>& img);

	void setWidthHeightByLevel(cv::Mat im, std::vector<std::vector<int>>& withLevel, 
								std::vector<std::vector<int>>& heightLevel) {
		std::vector<int> tempVecW;
		std::vector<int> tempVecH;
			int wtemp, htemp;
			
				for (int j = 0; j < 3; j++) {
					if (j==0) {
						tempVecW.push_back(im.cols);
						tempVecH.push_back(im.rows);
					}
					else {
						wtemp = tempVecW[j - 1] / 2;
						htemp = tempVecH[j - 1] / 2;
						tempVecW.push_back(wtemp);
						tempVecH.push_back(htemp);
					}
		}
				withLevel.push_back(tempVecW);
				heightLevel.push_back(tempVecH);
				tempVecW.clear();
				tempVecH.clear();
	}
protected:
	std::vector<unsigned char>  m_images;

	std::vector<int> m_widths;

	std::vector<int> m_heights;

};


#endif // !_IMAGE_


