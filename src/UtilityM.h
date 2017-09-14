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
#include"Camera.h"
#include"Cpoint.h"
#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<Cpoint> Ppoint;

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

	void transformImgFromUcharToMat(std::vector<unsigned char> v, cv::Mat& out, int width, int height);

	template<class T>
	void setF(const Camera& lhs, const Camera& rhs,
		TMat3<T>& F, const int level = 0) {
		const TVec4<T>& p00 = lhs.m_projection[level][0];
		const TVec4<T>& p01 = lhs.m_projection[level][1];
		const TVec4<T>& p02 = lhs.m_projection[level][2];

		const TVec4<T>& p10 = rhs.m_projection[level][0];
		const TVec4<T>& p11 = rhs.m_projection[level][1];
		const TVec4<T>& p12 = rhs.m_projection[level][2];

		F[0][0] = det(TMat4<T>(p01, p02, p11, p12));
		F[0][1] = det(TMat4<T>(p01, p02, p12, p10));
		F[0][2] = det(TMat4<T>(p01, p02, p10, p11));

		F[1][0] = det(TMat4<T>(p02, p00, p11, p12));
		F[1][1] = det(TMat4<T>(p02, p00, p12, p10));
		F[1][2] = det(TMat4<T>(p02, p00, p10, p11));

		F[2][0] = det(TMat4<T>(p00, p01, p11, p12));
		F[2][1] = det(TMat4<T>(p00, p01, p12, p10));
		F[2][2] = det(TMat4<T>(p00, p01, p10, p11));
	};

	template<class T>
	float computeEPD(const TMat3<T>& F, const TVec3<T>& p0, const TVec3<T>& p1) {
		TVec3<T> line = F * p1;
		const T ftmp = sqrt(line[0] * line[0] + line[1] * line[1]);
		if (ftmp == 0.0)
			return 0.0;

		line /= ftmp;
		return fabs(line * p0);
	};

	void WritePlySimple(std::string name, std::vector<std::vector<Ppoint>> point);

protected:
	drawCornerFor dc;

	
};


#endif // !_UTILITY_




