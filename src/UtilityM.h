#pragma once
#ifndef _UTILITYM_
#define _UTILITYM_

#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"Detector.h"
#include<set>
#include"vec3.h"
#include<fstream>
#include<string>



class UtilityM
{
public:
	enum drawCornerFor {
		dog,
		harris,
		
	};
	

	UtilityM();
	~UtilityM();

	void drawCorners(cv::Mat& img, std::vector<cv::Point2f>& corn,int n);
	void convertImgToUcharArray(cv::Mat in, int width, int height,std::vector<unsigned char>& out );
	
	void setDrawCorner(drawCornerFor d) {
		dc = d;
	}

	drawCornerFor getDrawC()const {
		return dc;
	}

protected:
	drawCornerFor dc;
	
};


#endif // !_UTILITY_




