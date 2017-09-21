#include "PatchOrganizer.h"
#include"DetectFeatures.h"
using namespace Patch;

Ppatch m_MAXDEPTH(new Cpatch());
Ppatch m_BACKGROUND(new Cpatch());

PatchOrganizer::PatchOrganizer(DetectFeatures& detectFeatures) : m_df(detectFeatures)
{
}


PatchOrganizer::~PatchOrganizer()
{
}


void PatchOrganizer::init() {
	std::cout << "width:" << m_df.getWidtByIndex(0, m_df.getmLevel()) << std::endl;
	m_pgrids.clear();   m_pgrids.resize(m_df.getNumOfImages());
	m_vpgrids.clear();  m_vpgrids.resize(m_df.getNumOfImages());
	m_dpgrids.clear();  m_dpgrids.resize(m_df.getNumOfImages());
	m_counts.clear();   m_counts.resize(m_df.getNumOfImages());

	m_gwidths.clear();  m_gwidths.resize(m_df.getNumOfImages());
	m_gheights.clear(); m_gheights.resize(m_df.getNumOfImages());
	
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		const int gwidth = (m_df.getWidtByIndex(index,m_df.getmLevel())
			+ m_df.getmCsize() - 1) / m_df.getmCsize();
		std::cout << "width:" << gwidth << std::endl;
		const int gheight = (m_df.getHeightByIndex(index, m_df.getmLevel())
			+ m_df.getmCsize() - 1) / m_df.getmCsize();
		m_gwidths[index] = gwidth;
		m_gheights[index] = gheight;

		if (index < m_df.getNumOfImages()) {
			m_pgrids[index].resize(gwidth * gheight);
			m_vpgrids[index].resize(gwidth * gheight);
			m_dpgrids[index].resize(gwidth * gheight);
			m_counts[index].resize(gwidth * gheight);
			fill(m_dpgrids[index].begin(), m_dpgrids[index].end(), m_MAXDEPTH);
		}
	}
}

void PatchOrganizer::clearCounts(void) {
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		std::vector<unsigned char>::iterator begin = m_counts[index].begin();
		std::vector<unsigned char>::iterator end = m_counts[index].end();
		while (begin != end) {
			*begin = (unsigned char)0;
			++begin;
		}
	}
}

void PatchOrganizer::setScales(Patch::Cpatch& patch) const {
	const float unit = m_df.m_optim.getUnit(patch.m_images[0], patch.m_coord);
	const float unit2 = 2.0f * unit;
	Vec4f ray = patch.m_coord - m_df.getPhoto(patch.m_images[0]).m_center;
	unitize(ray);

	const int inum = (std::min)(m_df.m_tau, (int)patch.m_images.size());

	// First compute, how many pixel difference per unit along vertical
	//for (int i = 1; i < (int)patch.m_images.size(); ++i) {
	for (int i = 1; i < inum; ++i) {
		Vec3f diff = m_df.getPhoto(patch.m_images[i]).project(patch.m_images[i],patch.m_coord, m_df.getmLevel()) -
			m_df.getPhoto(patch.m_images[i]).project(patch.m_images[i], patch.m_coord - ray * unit2, m_df.getmLevel());
		patch.m_dscale += norm(diff);
	}

	// set m_dscale to the vertical distance where average pixel move is half pixel
	//patch.m_dscale /= (int)patch.m_images.size() - 1;
	patch.m_dscale /= inum - 1;
	patch.m_dscale = unit2 / patch.m_dscale;

	patch.m_ascale = atan(patch.m_dscale / (unit * m_df.m_wsize / 2.0f));
}