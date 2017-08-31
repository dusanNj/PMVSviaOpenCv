#pragma once
#ifndef  _DOG_
#define _DOG_

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
#include"Harris.h"

class Dog : Detector
{
public:
	Dog();
	~Dog();

	void run(std::vector<std::vector<unsigned char>>& image,
		const std::vector<unsigned char> mask,
		const std::vector<unsigned char> edge,
		const int width, const int height,
		const int gspeedup,
		const float firstScale,   // 1.4f
		const float lastScale,    // 4.0f
		std::multiset<Cpoint> & result,
		std::vector<cv::Point2f>& dogCorners);

protected:
	float m_firstScale;
	float m_lastScale;

	std::vector<int> imgWidthDog;
	std::vector<int> imgHeightDog;


	void init(const std::vector<unsigned char>& image,
		const std::vector<unsigned char> mask,
		const std::vector<unsigned char> edge);

	void setRes(const float sigma,
		std::vector<std::vector<float> >& res);

	static void setDOG(const std::vector<std::vector<float> >& cres,
		const std::vector<std::vector<float> >& nres,
		std::vector<std::vector<float> >& dog);

	static int isLocalMax(const std::vector<std::vector<float> >& pdog,
		const std::vector<std::vector<float> >& cdog,
		const std::vector<std::vector<float> >& ndog,
		const int x, const int y);

	static int isLocalMax(const std::vector<std::vector<float> >& dog,
		const int x, const int y);


	static int notOnEdge(const std::vector<std::vector<float> >& dog, int x, int y);
};


#endif // ! _DOG_



