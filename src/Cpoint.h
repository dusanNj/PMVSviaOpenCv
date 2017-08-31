#pragma once
#ifndef _CPOINT_
#define _CPOINT_

#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include<vector>
#include"vec3.h"
#include"vec4.h"
class Cpoint
{
public:
	Cpoint(void);
	~Cpoint();
	Vec3f m_icoord;
	float m_response;

	//0: Harris
	//1:Dog

	int m_type;
	//tempporary variable, used to store original imaged id in initial match
	int m_itmp;

	Vec4f m_coord;

	bool operator < (const Cpoint& cps) const {
		return m_response < cps.m_response;
	}

	friend std::istream& operator >> (std::istream& istr, const Cpoint& cps);
	friend std::ostream& operator << (std::ostream& ostr, const Cpoint& cps);
};

bool SortCpoint(const Cpoint& a, const Cpoint& b);
std::istream& operator >> (std::istream& istr,  Cpoint& cps);
std::ostream& operator << (std::ostream& ostr, const Cpoint& cps);

#endif // !_CPOINT_





