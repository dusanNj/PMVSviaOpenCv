#pragma once
#ifndef _OPTION_
#define _OPTION_
#include <vector>
#include "patch.h"


class DetectFeatures;

class Optim
{
public:
	Optim(DetectFeatures& detectFeatures);
	virtual ~Optim() {};
	void init(void);
	void setAxesScales(void);
	void collectImages(const int index, std::vector<int>& indexes) const;
	//TODO:5a
	//int preProcess(Patch::Cpatch& patch, const int id, const int seed);
	void addImages(Patch::Cpatch& patch) const;
	//TODO:4a
	/*void constraintImages(Patch::Cpatch& patch, const float nccThreshold,
		const int id);*/

	//get i set
	std::vector<int> getmStatus() {
		return m_status;
	}
	void setmStatus(std::vector<int> m_status) {
		this->m_status = m_status;
	}

	DetectFeatures& m_df;
protected:
	
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


