#pragma once
#ifndef  _HARISS_
#define _HARISS_

#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"Detector.h"
#include<set>
#include"vec3.h"
#include"Image.h"
#include"UtilityM.h"

class Harris : public Detector
{
public:
	Harris(int MaxLevel);
	~Harris();

	void run( std::vector<std::vector<unsigned char>>& image,
		const std::vector<unsigned char> mask,
		const std::vector<unsigned char> edge,
		const int width, const int height,
		const int gspeedup, const float sigma,
		std::vector<cv::Point2f>& cornerPos,
		std::multiset<Cpoint> & result);

	void init(const std::vector<unsigned char>& image,
		const std::vector<unsigned char>& mask,
		const std::vector<unsigned char>& edge);

	void setDerivatives(void);
	void preprocess(void);
	void preprocess2(void);
	void setResponse(void);





protected:
	float m_sigmaD;
	float m_sigmaI;
	int maxLevelCount;


	std::vector<float> m_gaussD;
	std::vector<float> m_gaussI;

	//std::vector<int> imgWidth;
	//std::vector<int> imgHeight;


	std::vector<std::vector<unsigned char>> imgUCharL;


	std::vector<std::vector<Vec3f> > m_dIdx;
	std::vector<std::vector<Vec3f> > m_dIdy;
	std::vector<std::vector<float> > m_dIdxdIdx;
	std::vector<std::vector<float> > m_dIdydIdy;
	std::vector<std::vector<float> > m_dIdxdIdy;

	std::vector<std::vector<float> > m_response;




};


#endif // _HARISS_






