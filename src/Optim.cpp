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
	std::cout << "ray:" << ray0 << std::endl;
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

	Vec4f ray1 = m_df.getPhoto(indextmp).m_oaxis;
	ray1[3] = 0.0f;
	std::cout << "ray1:" << ray1 << std::endl;
	if (ray0 * ray1 < cos(m_df.m_angleThreshold0))
		continue;
	std::cout << "m_distances:" << m_df.mdf_distance[index][indextmp] << std::endl;
	candidates.push_back(Vec2f(m_df.mdf_distance[index][indextmp], indextmp));
	}

	sort(candidates.begin(), candidates.end(), Svec2cmp<float>());
	for (int i = 0; i < (std::min)(m_df.getNumOfImages(), (int)candidates.size()); ++i)
		indexes.push_back((int)candidates[i][1]);


}
void Optim::addImages(Patch::Cpatch& patch) const {
	// take into account m_edge
	std::vector<int> used;
	used.resize(m_df.getNumOfImages());
	for (int index = 0; index < m_df.getNumOfImages(); ++index)
		used[index] = 0;

	std::vector<int>::const_iterator bimage = patch.m_images.begin();
	std::vector<int>::const_iterator eimage = patch.m_images.end();
	while (bimage != eimage) {
		used[*bimage] = 1;
		++bimage;
	}

	bimage = m_df.m_visdata2[patch.m_images[0]].begin();
	eimage = m_df.m_visdata2[patch.m_images[0]].end();

	const float athreshold = cos(m_df.m_angleThreshold0);
	while (bimage != eimage) {
		if (used[*bimage]) {
			++bimage;
			continue;
		}

		const Vec3f icoord = m_df.getPhoto(*bimage).project(*bimage, patch.m_coord, m_df.getmLevel());
		if (icoord[0] < 0.0f || m_df.getWidtByIndex(*bimage, m_df.getmLevel()) - 1 <= icoord[0] ||
			icoord[1] < 0.0f || m_df.getHeightByIndex(*bimage, m_df.getmLevel()) - 1 <= icoord[1]) {
			++bimage;
			continue;
		}
		//TODO:2 uraditi kad se budu ubacivali edge-evi
		/*if (m_df.m_pss.getEdge(patch.m_coord, *bimage, m_df.getmLevel()) == 0) {
			++bimage;
			continue;
		}*/

		Vec4f ray = m_df.getPhoto(*bimage).m_center - patch.m_coord;
		unitize(ray);
		const float ftmp = ray * patch.m_normal;

		if (athreshold <= ftmp)
			patch.m_images.push_back(*bimage);

		++bimage;
	}
}
//TODO:4 constrain image
//void Optim::constraintImages(Patch::Cpatch& patch, const float nccThreshold,
//	const int id) {
//	std::vector<float> inccs;
//	setINCCs(patch, inccs, patch.m_images, id, 0);
//
//	//----------------------------------------------------------------------
//	// Constraint images
//	vector<int> newimages;
//	newimages.push_back(patch.m_images[0]);
//	for (int i = 1; i < (int)patch.m_images.size(); ++i) {
//		if (inccs[i] < 1.0f - nccThreshold)
//			newimages.push_back(patch.m_images[i]);
//	}
//	patch.m_images.swap(newimages);
//}

//TODO:5 preProces
//int Optim::preProcess(Patch::Cpatch& patch, const int id, const int seed) {
//	addImages(patch);
//	// Here define reference images, and sort images.
//	// Something similar to constraintImages is done inside.
//	constraintImages(patch, m_df.m_nccThresholdBefore, id);
//
//	// Fix the reference image and sort the other  m_tau - 1 images.
//	sortImages(patch);
//
//	// Pierre Moulon (it avoid crash in some case)
//	if ((int)patch.m_images.size() > 0)
//	{
//		// setSscales should be here to avoid noisy output
//		m_df.m_pos.setScales(patch);
//	}
//
//	// Check minimum number of images
//	if ((int)patch.m_images.size() < m_df.m_minImageNumThreshold)
//		return 1;
//
//	const int flag =
//		m_df.m_pss.checkAngles(patch.m_coord, patch.m_images,
//			m_df.m_maxAngleThreshold,
//			m_df.m_angleThreshold1,
//			m_df.m_minImageNumThreshold);
//
//	if (flag) {
//		patch.m_images.clear();
//		return 1;
//	}
//
//	return 0;
//}