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
	std::cout << "width:" << m_df.imgWidth[0][3-1] << std::endl;
	m_pgrids.clear();   m_pgrids.resize(m_df.getNumOfImages());
	m_vpgrids.clear();  m_vpgrids.resize(m_df.getNumOfImages());
	m_dpgrids.clear();  m_dpgrids.resize(m_df.getNumOfImages());
	m_counts.clear();   m_counts.resize(m_df.getNumOfImages());

	m_gwidths.clear();  m_gwidths.resize(m_df.getNumOfImages());
	m_gheights.clear(); m_gheights.resize(m_df.getNumOfImages());
	
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		const int gwidth = (m_df.getWidtByIndex(index,m_df.getmLevel())
			+ m_df.getmCsize() - 1) / m_df.getmCsize();
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