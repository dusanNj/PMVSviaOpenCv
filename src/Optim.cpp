#include "Optim.h"
#include"DetectFeatures.h"
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
		m_zaxes[index] = Vec3f(m_df.getPhoto(index).m_oaxis[0],
			m_df.getPhoto(index).m_oaxis[1],
			m_df.getPhoto(index).m_oaxis[2]);
		m_xaxes[index] = Vec3f(m_df.getPhoto(index).m_projection[0][0][0],
			m_df.getPhoto(index).m_projection[0][0][1],
			m_df.getPhoto(index).m_projection[0][0][2]);
		m_yaxes[index] = cross(m_zaxes[index], m_xaxes[index]);
		unitize(m_yaxes[index]);
		m_xaxes[index] = cross(m_yaxes[index], m_zaxes[index]);
	}

	m_ipscales.resize(m_df.getNumOfImages());
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		const Vec4f xaxe(m_xaxes[index][0], m_xaxes[index][1], m_xaxes[index][2], 0.0);
		const Vec4f yaxe(m_yaxes[index][0], m_yaxes[index][1], m_yaxes[index][2], 0.0);

		const float fx = xaxe * m_df.getPhoto(index).m_projection[0][0];
		const float fy = yaxe * m_df.getPhoto(index).m_projection[0][1];
		m_ipscales[index] = fx + fy;
	}
}

void Optim::collectImages(const int index, std::vector<int>& indexes)const {
	/* Find images with constraints m_angleThreshold, m_visdata,
	 m_sequenceThreshold, m_targets. Results are sorted by
	 m_distances.*/
	indexes.clear();
	Vec4f ray0 = m_df.photos[index].m_oaxis;
	ray0[3] = 0.0f;

	std::vector<Vec2f> candidates;
	// Search for only related images
	for (int i = 0; i < (int)m_df.m_visdata2[index].size(); ++i) {
		const int indextmp = m_df.m_visdata2[index][i];

	//if (m_df.m_tnum <= indextmp)
	//continue;
	if (m_df.m_sequenceThreshold != -1 &&
		m_df.m_sequenceThreshold < abs(index - indextmp))
		continue;

	Vec4f ray1 = m_df.photos[indextmp].m_oaxis;
	ray1[3] = 0.0f;

	if (ray0 * ray1 < cos(m_df.m_angleThreshold0))
		continue;
	candidates.push_back(Vec2f(m_df.photos[index].m_distances[index][indextmp], indextmp));
	}


}
