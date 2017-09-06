#include "Seed.h"



Seed::Seed(DetectFeatures& detectFeatures) : m_df(detectFeatures)
{
}

void Seed::init(const std::vector<std::vector<Cpoint> >& points) {

	m_ppoints.clear();
	m_ppoints.resize(m_df.getNumOfImages());
	
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		const int gheight = m_df.m_pos.getmgHeights(index);
		const int gwidth = m_df.m_pos.getmgWidths(index);
		m_ppoints[index].resize(gwidth * gheight);
	}

	readPoints(points);
}

void Seed::readPoints(const std::vector<std::vector<Cpoint> >& points) {
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		for (int i = 0; i < (int)points[index].size(); ++i) {
			Ppoint ppoint(new Cpoint(points[index][i]));
			ppoint->m_itmp = index;
			const int ix = ((int)floor(ppoint->m_icoord[0] + 0.5f)) / m_df.getmCsize();
			const int iy = ((int)floor(ppoint->m_icoord[1] + 0.5f)) / m_df.getmCsize();
			const int index2 = iy * m_df.m_pos.getmgWidths(index) + ix;
			m_ppoints[index][index2].push_back(ppoint);
		}
	}
}


