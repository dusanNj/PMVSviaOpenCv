#define _USE_MATH_DEFINES
#include <cmath>

#include <algorithm>
#include <numeric>
#include <cstdio>
#include "Optim.h"
#include"DetectFeatures.h"
#include"nlopt.hpp"
Optim* Optim::m_one = NULL;

Optim::Optim(DetectFeatures& detectFeatures) : m_df(detectFeatures)
{
	m_one = this;
	m_status.resize(35);
	fill(m_status.begin(), m_status.end(), 0);

}

void Optim::init(void) {
	m_texsT.resize(2);
	for (int i = 0; i < m_texsT.size(); i++)
	{
		m_texsT[i].resize(m_df.getNumOfImages());
	}

	for (int i = 0; i < m_texsT.size(); i++)
	{
		for (int j = 0; j < m_texsT[i].size(); j++)
		{
			m_texsT[i][j].resize(3*m_df.m_wsize*m_df.m_wsize);
		}
	}
	m_weightsT.resize(2);
	m_weightsT[0].resize(2);
	m_weightsT[1].resize(2);


	m_vect0T.resize(2);
	m_centersT.resize(2);
	m_raysT.resize(2);
	m_indexesT.resize(2);
	m_dscalesT.resize(2);
	m_ascalesT.resize(2);

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
	//std::cout << "ray:" << ray0 << std::endl;
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
	//std::cout << "ray1:" << ray1 << std::endl;
	if (ray0 * ray1 < cos(m_df.m_angleThreshold0))
		continue;
	//std::cout << "m_distances:" << m_df.mdf_distance[index][indextmp] << std::endl;
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

void Optim::sortImages(Patch::Cpatch& patch) const {

	const int newm = 1;
	if (newm == 1) {
		const float threshold = 1.0f - cos(10.0 * M_PI / 180.0);
		std::vector<int> indexes, indexes2;
		std::vector<float> units, units2;
		std::vector<Vec4f> rays, rays2;

		computeUnits(patch, indexes, units, rays);

		patch.m_images.clear();
		if (indexes.size() < 2)
			return;

		units[0] = 0.0f;

		while (!indexes.empty()) {
			std::vector<float>::iterator ite = min_element(units.begin(), units.end());
			const int index = ite - units.begin();

			patch.m_images.push_back(indexes[index]);

			// Remove other images within 5 degrees
			indexes2.clear();    units2.clear();
			rays2.clear();
			for (int j = 0; j < (int)rays.size(); ++j) {
				if (j == index)
					continue;

				indexes2.push_back(indexes[j]);
				rays2.push_back(rays[j]);
				const float ftmp = std::min(threshold,
					std::max(threshold / 2.0f,
						1.0f - rays[index] * rays[j]));

				units2.push_back(units[j] * (threshold / ftmp));
			}
			indexes2.swap(indexes);
			units2.swap(units);
			rays2.swap(rays);
		}
	}
	else {
		//----------------------------------------------------------------------
		//Sort and grab the best m_tau images. All the other images don't
		//matter.  First image is the reference and fixed
		const float threshold = cos(5.0 * M_PI / 180.0);
		std::vector<int> indexes, indexes2;
		std::vector<float> units, units2;
		std::vector<Vec4f> rays, rays2;

		computeUnits(patch, indexes, units, rays);

		patch.m_images.clear();
		if (indexes.size() < 2)
			return;

		units[0] = 0.0f;

		while (!indexes.empty()) {
			//for (int i = 0; i < size; ++i) {
			std::vector<float>::iterator ite = min_element(units.begin(), units.end());
			const int index = ite - units.begin();

			patch.m_images.push_back(indexes[index]);

			// Remove other images within 5 degrees
			indexes2.clear();    units2.clear();
			rays2.clear();
			for (int j = 0; j < (int)rays.size(); ++j) {
				if (rays[index] * rays[j] < threshold) {
					indexes2.push_back(indexes[j]);
					units2.push_back(units[j]);
					rays2.push_back(rays[j]);
				}
			}
			indexes2.swap(indexes);
			units2.swap(units);
			rays2.swap(rays);
		}
	}

}

void Optim::computeUnits(const Patch::Cpatch& patch,
	std::vector<int>& indexes,
	std::vector<float>& units,
	std::vector<Vec4f>& rays) const {

	std::vector<int>::const_iterator bimage = patch.m_images.begin();
	std::vector<int>::const_iterator eimage = patch.m_images.end();

	while (bimage != eimage) {
		Vec4f ray = m_df.getPhoto(*bimage).m_center - patch.m_coord;
		unitize(ray);
		const float dot = ray * patch.m_normal;
		if (dot <= 0.0f) {
			++bimage;
			continue;
		}

		const float scale = getUnit(*bimage, patch.m_coord);
		const float fine = scale / dot;

		indexes.push_back(*bimage);
		units.push_back(fine);
		rays.push_back(ray);
		++bimage;
	}

}

int Optim::preProcess(Patch::Cpatch& patch, const int id, const int seed) {
	addImages(patch);
	// here define reference images, and sort images.
	// something similar to constraintimages is done inside.
	constraintImages(patch, m_df.m_nccThresholdBefore, id);

	// fix the reference image and sort the other  m_tau - 1 images.
	sortImages(patch);

	// pierre moulon (it avoid crash in some case)
	if ((int)patch.m_images.size() > 0)
	{
		// setsscales should be here to avoid noisy output
		m_df.m_pos.setScales(patch);
	}

	// check minimum number of images
	if ((int)patch.m_images.size() < m_df.m_minimagenumthresho)
		return 1;

	const int flag =
		m_df.checkAngles(patch.m_coord, patch.m_images,
			m_df.m_maxAngleThreshold,
			m_df.m_angleThreshold1,
			m_df.m_minimagenumthresho);

	if (flag) {
		patch.m_images.clear();
		return 1;
	}

	return 0;
}

void Optim::constraintImages(Patch::Cpatch& patch, const float nccThreshold,
	const int id) {

	std::vector<float> inccs;
	setINCCs(patch, inccs, patch.m_images, id, 0);

	//----------------------------------------------------------------------
	// Constraint images
	std::vector<int> newimages;
	newimages.push_back(patch.m_images[0]);
	for (int i = 1; i < (int)patch.m_images.size(); ++i) {
		if (inccs[i] < 1.0f - nccThreshold)
			newimages.push_back(patch.m_images[i]);
	}
	patch.m_images.swap(newimages);

}
void Optim::normalize(std::vector<float>& tex) {

	const int size = (int)tex.size();
	const int size3 = size / 3;
	Vec3f ave;

	float* texp = &tex[0] - 1;
	for (int i = 0; i < size3; ++i) {
		ave[0] += *(++texp);
		ave[1] += *(++texp);
		ave[2] += *(++texp);
	}

	ave /= size3;

	float ave2 = 0.0;
	texp = &tex[0] - 1;
	for (int i = 0; i < size3; ++i) {
		const float f0 = ave[0] - *(++texp);
		const float f1 = ave[1] - *(++texp);
		const float f2 = ave[2] - *(++texp);

		ave2 += f0 * f0 + f1 * f1 + f2 * f2;
	}

	ave2 = sqrt(ave2 / size);

	if (ave2 == 0.0f)
		ave2 = 1.0f;

	texp = &tex[0] - 1;
	for (int i = 0; i < size3; ++i) {
		*(++texp) -= ave[0];    *texp /= ave2;
		*(++texp) -= ave[1];    *texp /= ave2;
		*(++texp) -= ave[2];    *texp /= ave2;
	}
}

float Optim::dot(const std::vector<float>& tex0,
	const std::vector<float>& tex1) const {
	#ifndef PMVS_WNCC
  // Pierre Moulon (use classic access to array, windows STL do not like begin()-1)
  const int size = (int)tex0.size();
  float ans = 0.0f;
  for (int i = 0; i < size; ++i) {
    ans += tex0[i] * tex1[i];
  }  
  return ans / size;
#else
  const int size = (int)tex0.size();
  vector<float>::const_iterator i0 = tex0.begin();
  vector<float>::const_iterator i1 = tex1.begin();
  float ans = 0.0f;
  for (int i = 0; i < size; ++i, ++i0, ++i1) {
    ans += (*i0) * (*i1) * m_template[i];
  }
  return ans;
#endif
}

void Optim::setINCCs(const Patch::Cpatch& patch,
	std::vector<float> & inccs,
	const std::vector<int>& indexes,
	const int id, const int robust) {
	const int index = indexes[0];
	Vec4f pxaxis, pyaxis;
	getPAxes(index, patch.m_coord, patch.m_normal, pxaxis, pyaxis);

	std::vector<std::vector<float> >& texs = m_texsT[id];

	const int size = (int)indexes.size();
	for (int i = 0; i < size; ++i) {
		const int flag = grabTex(patch.m_coord, pxaxis, pyaxis, patch.m_normal,
			indexes[i], m_df.m_wsize, texs[i]);
		if (flag == 0) {
			normalize(texs[i]);
		}
	}

	inccs.resize(size);
	if (texs[0].empty()) {
		fill(inccs.begin(), inccs.end(), 2.0f);
		return;
	}

	for (int i = 0; i < size; ++i) {
		if (i == 0)
			inccs[i] = 0.0f;
		else if (!texs[i].empty()) {
			if (robust == 0)
				inccs[i] = 1.0f - dot(texs[0], texs[i]);
			else
				inccs[i] = robustincc(1.0f - dot(texs[0], texs[i]));
		}
		else
			inccs[i] = 2.0f;
	}
}

float Optim::getUnit(const int index, const Vec4f& coord) const {
	const float fz = norm(coord - m_df.getPhoto(index).m_center);
	const float ftmp = m_ipscales[index];
	if (ftmp == 0.0)
		return 1.0;

	return 2.0 * fz * (0x0001 << m_df.getmLevel()) / ftmp;
}

void Optim::getPAxes(const int index, const Vec4f& coord, const Vec4f& normal,
	Vec4f& pxaxis, Vec4f& pyaxis) const {
	const float pscale = getUnit(index, coord);

	Vec3f normal3(normal[0], normal[1], normal[2]);
	Vec3f yaxis3 = cross(normal3, m_xaxes[index]);
	unitize(yaxis3);
	Vec3f xaxis3 = cross(yaxis3, normal3);
	pxaxis[0] = xaxis3[0];  pxaxis[1] = xaxis3[1];  pxaxis[2] = xaxis3[2];  pxaxis[3] = 0.0;
	pyaxis[0] = yaxis3[0];  pyaxis[1] = yaxis3[1];  pyaxis[2] = yaxis3[2];  pyaxis[3] = 0.0;

	pxaxis *= pscale;
	pyaxis *= pscale;
	const float xdis = norm(m_df.getPhoto(index).project(index, coord + pxaxis, m_df.getmLevel()) -
		m_df.getPhoto(index).project(index, coord, m_df.getmLevel()));
	const float ydis = norm(m_df.getPhoto(index).project(index, coord + pyaxis, m_df.getmLevel()) -
		m_df.getPhoto(index).project(index, coord, m_df.getmLevel()));
	pxaxis /= xdis;
	pyaxis /= ydis;
}
static float Log2 = log(2.0f);
// My own optimisaton
float MyPow2(int x)
{
	const float answers[] = { 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };

	return answers[x + 4];
}
int Optim::grabSafe(const int index, const int size, const Vec3f& center,
	const Vec3f& dx, const Vec3f& dy, const int level) const {
	const int margin = size / 2;

	const Vec3f tl = center - dx * margin - dy * margin;
	const Vec3f tr = center + dx * margin - dy * margin;

	const Vec3f bl = center - dx * margin + dy * margin;
	const Vec3f br = center + dx * margin + dy * margin;

	const float minx = std::min(tl[0], std::min(tr[0], std::min(bl[0], br[0])));
	const float maxx = std::max(tl[0], std::max(tr[0], std::max(bl[0], br[0])));
	const float miny = std::min(tl[1], std::min(tr[1], std::min(bl[1], br[1])));
	const float maxy = std::max(tl[1], std::max(tr[1], std::max(bl[1], br[1])));

	// 1 should be enough
	const int margin2 = 3;
	// ??? may need to change if we change interpolation method
	if (minx < margin2 ||
		m_df.getWidtByIndex(index, level) - 1 - margin2 <= maxx ||
		miny < margin2 ||
		m_df.getHeightByIndex(index, level) - 1 - margin2 <= maxy)
		return 0;
	return 1;
}


int Optim::grabTex(const Vec4f& coord, const Vec4f& pxaxis, const Vec4f& pyaxis,
	const Vec4f& pzaxis, const int index, const int size,
	std::vector<float>& tex) const {

	tex.clear();

	Vec4f ray = m_df.getPhoto(index).m_center - coord;
	unitize(ray);
	const float weight = std::max(0.0f, ray * pzaxis);

	//???????
	//if (weight < cos(m_df.m_angleThreshold0))
	if (weight < cos(m_df.m_angleThreshold1))
		return 1;

	const int margin = size / 2;

	Vec3f center = m_df.getPhoto(index).project(index, coord, m_df.getmLevel());
	Vec3f dx = m_df.getPhoto(index).project(index, coord + pxaxis, m_df.getmLevel()) - center;
	Vec3f dy = m_df.getPhoto(index).project(index, coord + pyaxis, m_df.getmLevel()) - center;

	const float ratio = (norm(dx) + norm(dy)) / 2.0f;
	//int leveldif = (int)floor(log(ratio) / log(2.0f) + 0.5f);
	int leveldif = (int)floor(log(ratio) / Log2 + 0.5f);

	// Upper limit is 2
	leveldif = std::max(-m_df.getmLevel(), std::min(2, leveldif));

	//const float scale = pow(2.0f, (float)leveldif);

	const float scale = MyPow2(leveldif);
	const int newlevel = m_df.getmLevel() + leveldif;

	center /= scale;  dx /= scale;  dy /= scale;

	if (grabSafe(index, size, center, dx, dy, newlevel) == 0)
		return 1;

	Vec3f left = center - dx * margin - dy * margin;

	tex.resize(3 * size * size);
	float* texp = &tex[0] - 1;
	for (int y = 0; y < size; ++y) {
		Vec3f vftmp = left;
		left += dy;
		for (int x = 0; x < size; ++x) {
			Vec3f color = m_df.getColor(index, vftmp[0], vftmp[1], newlevel);
			*(++texp) = color[0];
			*(++texp) = color[1];
			*(++texp) = color[2];
			vftmp += dx;
		}
	}
	return 0;
}

double Optim::my_f(unsigned n, const double *x, double *grad, void *my_func_data){
	  double xs[3] = {x[0], x[1], x[2]};
  const int id = *((int*)my_func_data);

  const float angle1 = xs[1] * m_one->m_ascalesT[id];
  const float angle2 = xs[2] * m_one->m_ascalesT[id];

  double ret = 0.0;

  //?????
  const double bias = 0.0f;//2.0 - exp(- angle1 * angle1 / sigma2) - exp(- angle2 * angle2 / sigma2);
  
  Vec4f coord, normal;
  m_one->decode(coord, normal, xs, id);
  
  const int index = m_one->m_indexesT[id][0];
  Vec4f pxaxis, pyaxis;
  m_one->getPAxes(index, coord, normal, pxaxis, pyaxis);
  
  const int size = (std::min)(m_one->m_df.m_tau, (int)m_one->m_indexesT[id].size());
  const int mininum = std::min(m_one->m_df.m_minimagenumthresho, size);

  for (int i = 0; i < size; ++i) {
    int flag;
    flag = m_one->grabTex(coord, pxaxis, pyaxis, normal, m_one->m_indexesT[id][i],
                          m_one->m_df.m_wsize, m_one->m_texsT[id][i]);

    if (flag == 0)
      m_one->normalize(m_one->m_texsT[id][i]);
  }

  const int pairwise = 0;
  if (pairwise) {
    double ans = 0.0f;
    int denom = 0;
    for (int i = 0; i < size; ++i) {
      for (int j = i+1; j < size; ++j) {
        if (m_one->m_texsT[id][i].empty() || m_one->m_texsT[id][j].empty())
          continue;
        
        ans += robustincc(1.0 - m_one->dot(m_one->m_texsT[id][i], m_one->m_texsT[id][j]));
        denom++;
      }
    }
    if (denom <
        //m_one->m_fm.m_minImageNumThreshold *
        //(m_one->m_fm.m_minImageNumThreshold - 1) / 2)
        mininum * (mininum - 1) / 2)
      ret = 2.0f;
    else
      ret = ans / denom + bias;
  }
  else {
    if (m_one->m_texsT[id][0].empty())
      return 2.0;
      
    double ans = 0.0f;
    int denom = 0;
    for (int i = 1; i < size; ++i) {
      if (m_one->m_texsT[id][i].empty())
        continue;
      ans +=
        robustincc(1.0 - m_one->dot(m_one->m_texsT[id][0], m_one->m_texsT[id][i]));
      denom++;
    }
    //if (denom < m_one->m_fm.m_minImageNumThreshold - 1)
    if (denom < mininum - 1)
      ret = 2.0f;
    else
      ret = ans / denom + bias;
  }

  return ret;
}

bool Optim::refinePatchBFGS(Patch::Cpatch& patch, const int id,
	const int time, const int ncc) {

	int idtmp = id;

	m_centersT[id] = patch.m_coord;
	m_raysT[id] = patch.m_coord - m_df.getPhoto(patch.m_images[0]).m_center;
	unitize(m_raysT[id]);
	m_indexesT[id] = patch.m_images;

	m_dscalesT[id] = patch.m_dscale;
	m_ascalesT[id] = M_PI / 48.0f; //patch.m_ascale;

	computeUnits(patch, m_weightsT[id]);

	for (int i = 1; i < (int)m_weightsT[id].size(); ++i)
		m_weightsT[id][i] = (std::min)(1.0f, m_weightsT[id][0] / m_weightsT[id][i]);
	m_weightsT[id][0] = 1.0f;

	double p[3];
	encode(patch.m_coord, patch.m_normal, p, id);

	double min_angle = -23.99999;
	double max_angle = 23.99999;

	std::vector<double> lower_bounds(3);
	lower_bounds[0] = -HUGE_VAL;		// Not bound
	lower_bounds[1] = min_angle;
	lower_bounds[2] = min_angle;
	std::vector<double> upper_bounds(3);
	upper_bounds[0] = HUGE_VAL;		// Not bound
	upper_bounds[1] = max_angle;
	upper_bounds[2] = max_angle;

	bool success = false;

	try
	{
		// LN_NELDERMEAD: Corresponds to the N-Simplex-Algorithm of GSL, that was used originally here
		// LN_SBPLX
		// LN_COBYLA
		// LN_BOBYQA
		// LN_PRAXIS
		nlopt::opt opt(nlopt::LN_BOBYQA, 3);
		opt.set_min_objective(my_f, &idtmp);
		opt.set_xtol_rel(1.e-7);
		opt.set_maxeval(time);

		opt.set_lower_bounds(lower_bounds);
		opt.set_upper_bounds(upper_bounds);

		std::vector<double> x(3);
		for (int i = 0; i < 3; i++)
		{
			// NLOPT returns an error if x is not within the bounds
			x[i] = std::max(std::min(p[i], upper_bounds[i]), lower_bounds[i]);
		}

		double minf;
		nlopt::result result = opt.optimize(x, minf);

		p[0] = x[0];
		p[1] = x[1];
		p[2] = x[2];

		success = (result == nlopt::SUCCESS
			|| result == nlopt::STOPVAL_REACHED
			|| result == nlopt::FTOL_REACHED
			|| result == nlopt::XTOL_REACHED);
	}
	catch (std::exception &e)
	{
		success = false;
	}

	if (success) {
		decode(patch.m_coord, patch.m_normal, p, id);

		patch.m_ncc = 1.0 -
			unrobustincc(computeINCC(patch.m_coord,
				patch.m_normal, patch.m_images, id, 1));

	}
	else {
		return false;
	}

	return true;

	 
}
void Optim::encode(const Vec4f& coord,
	double* const vect, const int id) const {
	vect[0] = (coord - m_centersT[id]) * m_raysT[id] / m_dscalesT[id];
}

void Optim::encode(const Vec4f& coord, const Vec4f& normal,
	double* const vect, const int id) const {

	encode(coord, vect, id);

	const int image = m_indexesT[id][0];
	const float fx = m_xaxes[image] * proj(normal); // projects from 4D to 3D, divide by last value
	const float fy = m_yaxes[image] * proj(normal);
	const float fz = m_zaxes[image] * proj(normal);

	vect[2] = asin(std::max(-1.0f, std::min(1.0f, fy)));
	const float cosb = cos(vect[2]);

	if (cosb == 0.0)
		vect[1] = 0.0;
	else {
		const float sina = fx / cosb;
		const float cosa = -fz / cosb;
		vect[1] = acos(std::max(-1.0f, std::min(1.0f, cosa)));
		if (sina < 0.0)
			vect[1] = -vect[1];
	}

	vect[1] = vect[1] / m_ascalesT[id];
	vect[2] = vect[2] / m_ascalesT[id];
}
void Optim::decode(Vec4f& coord, Vec4f& normal,
	const double* const vect, const int id) const {

	decode(coord, vect, id);
	const int image = m_indexesT[id][0];

	const float angle1 = vect[1] * m_ascalesT[id];
	const float angle2 = vect[2] * m_ascalesT[id];

	const float fx = sin(angle1) * cos(angle2);
	const float fy = sin(angle2);
	const float fz = -cos(angle1) * cos(angle2);

	Vec3f ftmp = m_xaxes[image] * fx + m_yaxes[image] * fy + m_zaxes[image] * fz;
	normal = Vec4f(ftmp[0], ftmp[1], ftmp[2], 0.0f);

}
void Optim::decode(Vec4f& coord, const double* const vect, const int id) const {
	coord = m_centersT[id] + m_dscalesT[id] * vect[0] * m_raysT[id];
}
void Optim::computeUnits(const Patch::Cpatch& patch,
	std::vector<float>& units) const {

	const int size = (int)patch.m_images.size();
	units.resize(size);

	std::vector<int>::const_iterator bimage = patch.m_images.begin();
	std::vector<int>::const_iterator eimage = patch.m_images.end();

	std::vector<float>::iterator bfine = units.begin();

	while (bimage != eimage) {
		*bfine = INT_MAX / 2;

		*bfine = getUnit(*bimage, patch.m_coord);
		Vec4f ray = m_df.getPhoto(*bimage).m_center - patch.m_coord;
		unitize(ray);
		const float denom = ray * patch.m_normal;
		if (0.0 < denom)
			*bfine /= denom;
		else
			*bfine = INT_MAX / 2;

		++bimage;
		++bfine;
	}

}


void Optim::refinePatch(Patch::Cpatch& patch, const int id,
	const int time) {
	if (!refinePatchBFGS(patch, id, 1000, 1))
		std::cout << "refinePatchBFGS failed!" << std::endl;

	if (patch.m_images.empty())
		return;
}

double Optim::computeINCC(const Vec4f& coord, const Vec4f& normal,
	const std::vector<int>& indexes, const int id,
	const int robust) {

	if ((int)indexes.size() < 2)
		return 2.0;

	const int index = indexes[0];
	Vec4f pxaxis, pyaxis;
	getPAxes(index, coord, normal, pxaxis, pyaxis);

	return computeINCC(coord, normal, indexes, pxaxis, pyaxis, id, robust);

}
double Optim::computeINCC(const Vec4f& coord, const Vec4f& normal,
	const std::vector<int>& indexes, const Vec4f& pxaxis,
	const Vec4f& pyaxis, const int id,
	const int robust) {
	if ((int)indexes.size() < 2)
		return 2.0;

	const int size = (std::min)(m_df.m_tau, (int)indexes.size());
	std::vector<std::vector<float> >& texs = m_texsT[id];

	for (int i = 0; i < size; ++i) {
		int flag;
		flag = grabTex(coord, pxaxis, pyaxis, normal,
			indexes[i], m_df.m_wsize, texs[i]);

		if (flag == 0)
			normalize(texs[i]);
	}

	if (texs[0].empty())
		return 2.0;

	double score = 0.0;

	// pure pairwise of reference based
#ifdef PMVS_PAIRNCC
	float totalweight = 0.0;
	for (int i = 0; i < size; ++i) {
		for (int j = i + 1; j < size; ++j) {
			if (!texs[i].empty() && !texs[j].empty()) {
				const float ftmp = m_weightsT[id][i] * m_weightsT[id][j];
				totalweight += ftmp;
				if (robust)
					score += robustincc(1.0 - dot(texs[i], texs[j])) * ftmp;
				else
					score += (1.0 - dot(texs[i], texs[j])) * ftmp;
			}
		}
	}

	if (totalweight == 0.0)
		score = 2.0;
	else
		score /= totalweight;
#else
	float totalweight = 0.0;
	for (int i = 1; i < size; ++i) {
		if (!texs[i].empty()) {
			totalweight += m_weightsT[id][i];
			if (robust)
				score += robustincc(1.0 - dot(texs[0], texs[i])) * m_weightsT[id][i];
			else
				score += (1.0 - dot(texs[0], texs[i])) * m_weightsT[id][i];
		}
	}
	if (totalweight == 0.0)
		score = 2.0;
	else
		score /= totalweight;
#endif  

	return score;
}
void Optim::filterImagesByAngle(Patch::Cpatch& patch) {
	std::vector<int> newindexes;

	std::vector<int>::iterator bimage = patch.m_images.begin();
	std::vector<int>::iterator eimage = patch.m_images.end();

	while (bimage != eimage) {
		const int index = *bimage;
		Vec4f ray = m_df.getPhoto(index).m_center - patch.m_coord;
		unitize(ray);
		if (ray * patch.m_normal < cos(m_df.m_angleThreshold1)) {
			// if reference image is deleted, over
			if (bimage == patch.m_images.begin()) {
				patch.m_images.clear();
				return;
			}
		}
		else
			newindexes.push_back(index);
		++bimage;
	}

	patch.m_images.swap(newindexes);
}

void Optim::setRefImage(Patch::Cpatch& patch, const int id) {
#ifdef DEBUG
	if (patch.m_images.empty()) {
		cerr << "empty images" << endl;    exit(1);
	}
#endif
	//----------------------------------------------------------------------
	// Set the reference image
	// Only for target images
	std::vector<int> indexes;
	std::vector<int>::const_iterator begin = patch.m_images.begin();
	std::vector<int>::const_iterator end = patch.m_images.end();
	while (begin != end) {
		if (*begin < m_df.m_tnum)
			indexes.push_back(*begin);
		++begin;
	}
	// To avoid segmentation error on alley dataset. (this code is necessary because of the use of filterExact)
	if (indexes.empty()) {
		patch.m_images.clear();
		return;
	}

	std::vector<std::vector<float> > inccs;
	setINCCs(patch, inccs, indexes, id, 1);

	int refindex = -1;
	float refncc = INT_MAX / 2;
	for (int i = 0; i < (int)indexes.size(); ++i) {
		const float sum = accumulate(inccs[i].begin(), inccs[i].end(), 0.0f);
		if (sum < refncc) {
			refncc = sum;
			refindex = i;
		}
	}
	const int refIndex = indexes[refindex];
	for (int i = 0; i < (int)patch.m_images.size(); ++i) {
		if (patch.m_images[i] == refIndex) {
			const int itmp = patch.m_images[0];
			patch.m_images[0] = refIndex;
			patch.m_images[i] = itmp;
			break;
		}
	}
}
void Optim::setINCCs(const Patch::Cpatch& patch,
	std::vector<std::vector<float> >& inccs,
	const std::vector<int>& indexes,
	const int id, const int robust) {

	const int index = indexes[0];
	Vec4f pxaxis, pyaxis;
	getPAxes(index, patch.m_coord, patch.m_normal, pxaxis, pyaxis);

	std::vector<std::vector<float> >& texs = m_texsT[id];

	const int size = (int)indexes.size();
	for (int i = 0; i < size; ++i) {
		const int flag = grabTex(patch.m_coord, pxaxis, pyaxis, patch.m_normal,
			indexes[i], m_df.m_wsize, texs[i]);

		if (flag == 0)
			normalize(texs[i]);
	}

	inccs.resize(size);
	for (int i = 0; i < size; ++i)
		inccs[i].resize(size);

	for (int i = 0; i < size; ++i) {
		inccs[i][i] = 0.0f;
		for (int j = i + 1; j < size; ++j) {
			if (!texs[i].empty() && !texs[j].empty()) {
				if (robust == 0)
					inccs[j][i] = inccs[i][j] = 1.0f - dot(texs[i], texs[j]);
				else
					inccs[j][i] = inccs[i][j] = robustincc(1.0f - dot(texs[i], texs[j]));
			}
			else
				inccs[j][i] = inccs[i][j] = 2.0f;
		}
	}

}

int Optim::postProcess(Patch::Cpatch& patch, const int id, const int seed) {
	if ((int)patch.m_images.size() < m_df.m_minimagenumthresho)
		return 1;

	if (/*m_fm.m_pss.getMask(patch.m_coord, m_fm.m_level) == 0 ||*/
		m_df.insideBimages(patch.m_coord) == 0)
		return 1;

	addImages(patch);
	constraintImages(patch,m_df.m_nccThreshold,id);
	filterImagesByAngle(patch);

	if ((int)patch.m_images.size() < m_df.m_minimagenumthresho)
		return 1;

	m_df.m_pos.setGrids(patch);
	setRefImage(patch,id);
	constraintImages(patch, m_df.m_nccThreshold, id);

	if ((int)patch.m_images.size() < m_df.m_minimagenumthresho)
		return 1;

	m_df.m_pos.setGrids(patch);

	// set m_timages
	patch.m_timages = 0;
	std::vector<int>::const_iterator begin = patch.m_images.begin();
	std::vector<int>::const_iterator end = patch.m_images.end();
	while (begin != end) {
		if (*begin < m_df.m_tnum)
			++patch.m_timages;
		++begin;
	}

	patch.m_tmp = patch.score2(m_df.m_nccThreshold);
	// Set vimages vgrids.

	//TODO 3: Ovaj deo uraditi kada dodje do inicijalizacije deptha
	/*if (m_df.m_depth) {
		m_df.m_pos.setVImagesVGrids(patch);

		if (2 <= m_fm.m_depth && check(patch))
			return 1;
	}*/
	return 0;

}