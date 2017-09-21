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
	int preProcess(Patch::Cpatch& patch, const int id, const int seed);
	void addImages(Patch::Cpatch& patch) const;
	void constraintImages(Patch::Cpatch& patch, const float nccThreshold,
		const int id);
	void setINCCs(const Patch::Cpatch& patch,
		std::vector<float> & inccs,
		const std::vector<int>& indexes,
		const int id, const int robust);
	void getPAxes(const int index, const Vec4f& coord, const Vec4f& normal,
		Vec4f& pxaxis, Vec4f& pyaxis) const;
	float getUnit(const int index, const Vec4f& coord) const;
	int grabTex(const Vec4f& coord, const Vec4f& pxaxis, const Vec4f& pyaxis,
		const Vec4f& pzaxis, const int index, const int size,
		std::vector<float>& tex) const;
	int grabSafe(const int index, const int size, const Vec3f& center,
		const Vec3f& dx, const Vec3f& dy, const int level) const;
	void normalize(std::vector<float>& tex);
	float dot(const std::vector<float>& tex0,
		const std::vector<float>& tex1) const;
	static inline float robustincc(const float rhs) {
		return rhs / (1 + 3 * rhs);
	}
	bool refinePatchBFGS(Patch::Cpatch& patch, const int id,
		const int time, const int ncc);
	//get i set
	std::vector<int> getmStatus() {
		return m_status;
	}
	void setmStatus(std::vector<int> m_status) {
		this->m_status = m_status;
	}

	void sortImages(Patch::Cpatch& patch) const;
	void computeUnits(const Patch::Cpatch& patch,
		std::vector<int>& indexes,
		std::vector<float>& units,
		std::vector<Vec4f>& rays) const;
	// Grabbed texture
	std::vector<std::vector<std::vector<float> > > m_texsT; // last is 7x7x3 patch

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


