#pragma once
#ifndef _OPTION_
#define _OPTION_
#include <vector>
#include "patch.h"
#include"DetectFeatures.h"

class DetectFeatures;

class Optim
{
public:
	Optim(DetectFeatures& detectFeatures);
	virtual ~Optim() {};
	void init(void);
	void setAxesScales(void);



	//get i set
	std::vector<int> getmStatus() {
		return m_status;
	}
	void setmStatus(std::vector<int> m_status) {
		this->m_status = m_status;
	}


protected:
	DetectFeatures& m_df;
	static Optim* m_one;

	//-----------------------------------------------------------------
	// Axes
	std::vector<Vec3f> m_xaxes;
	std::vector<Vec3f> m_yaxes;
	std::vector<Vec3f> m_zaxes;
	// Scales
	std::vector<float> m_ipscales;

	std::vector<int> m_status;

};

#endif // !_OPTION_


