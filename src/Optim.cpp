#include "Optim.h"

Optim* Optim::m_one = NULL;

Optim::Optim(DetectFeatures& detectFeatures) : m_df(detectFeatures)
{
	m_one = this;
	m_status.resize(35);
	fill(m_status.begin(), m_status.end(), 0);
}

void Optim::init(void) {
	setAxesScales();
}


void Optim::setAxesScales(void) {
	m_xaxes.resize(m_df.getNumOfImages());
	m_yaxes.resize(m_df.getNumOfImages());
	m_zaxes.resize(m_df.getNumOfImages());
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		m_zaxes[index] = Vec3f(m_df.m_pss.m_photos[index].m_oaxis[0],
			m_fm.m_pss.m_photos[index].m_oaxis[1],
			m_fm.m_pss.m_photos[index].m_oaxis[2]);
		m_xaxes[index] = Vec3f(m_fm.m_pss.m_photos[index].m_projection[0][0][0],
			m_fm.m_pss.m_photos[index].m_projection[0][0][1],
			m_fm.m_pss.m_photos[index].m_projection[0][0][2]);
		m_yaxes[index] = cross(m_zaxes[index], m_xaxes[index]);
		unitize(m_yaxes[index]);
		m_xaxes[index] = cross(m_yaxes[index], m_zaxes[index]);
	}

	m_ipscales.resize(m_df.getNumOfImages());
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		const Vec4f xaxe(m_xaxes[index][0], m_xaxes[index][1], m_xaxes[index][2], 0.0);
		const Vec4f yaxe(m_yaxes[index][0], m_yaxes[index][1], m_yaxes[index][2], 0.0);

		const float fx = xaxe * m_fm.m_pss.m_photos[index].m_projection[0][0];
		const float fy = yaxe * m_fm.m_pss.m_photos[index].m_projection[0][1];
		m_ipscales[index] = fx + fy;
	}
}

