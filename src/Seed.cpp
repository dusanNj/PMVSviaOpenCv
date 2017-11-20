#include "Seed.h"
#include"DetectFeatures.h"
#include"PatchOrganizer.h"
#include"mat3.h"
#include<numeric>
#include"vec4.h"

Seed::Seed(DetectFeatures& detectFeatures) : m_df(detectFeatures)
{

}

void Seed::init(const std::vector<std::vector<Cpoint> >& points) {

	m_ppoints.clear();
	m_ppoints.resize(m_df.getNumOfImages());
	
	for (int index = 0; index < m_df.getNumOfImages(); ++index) {
		const int gheight = m_df.m_pos.getmgHeights(index);
		//std::cout << "w:" << gheight << std::endl;
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

void Seed::run() {
	m_df.m_count = 0;
	m_df.m_jobs.clear();
	m_scounts.resize(1);
	m_fcounts0.resize(1);
	m_fcounts1.resize(1);
	m_pcounts.resize(1);
	fill(m_scounts.begin(), m_scounts.end(), 0);
	fill(m_fcounts0.begin(), m_fcounts0.end(), 0);
	fill(m_fcounts1.begin(), m_fcounts1.end(), 0);
	fill(m_pcounts.begin(), m_pcounts.end(), 0);

	std::vector<int> vitmp;
	for (int i = 0; i <  m_df.getNumOfImages(); ++i)
		vitmp.push_back(i);

	random_shuffle(vitmp.begin(), vitmp.end());
	m_df.m_jobs.insert(m_df.m_jobs.end(), vitmp.begin(), vitmp.end());

	std::cerr << "adding seeds " << std::endl;

	m_df.m_pos.clearCounts();

	// If there already exists a patch, don't use
	for (int index = 0; index < (int)m_df.getNumOfImages(); ++index) {
		for (int j = 0; j < (int)m_df.m_pos.m_pgrids[index].size(); ++j) {
			if (!m_df.m_pos.m_pgrids[index][j].empty())
				m_df.m_pos.m_counts[index][j] = m_df.m_countThreshold2;
		}
	}
	initialMatchT();

	//----------------------------------------------------------------------
	std::cerr << "done" << std::endl;
	//cerr << "---- Initial: " << (tv - curtime) / CLOCKS_PER_SEC << " secs ----" << endl;

	const int trial = accumulate(m_scounts.begin(), m_scounts.end(), 0);
	const int fail0 = accumulate(m_fcounts0.begin(), m_fcounts0.end(), 0);
	const int fail1 = accumulate(m_fcounts1.begin(), m_fcounts1.end(), 0);
	const int pass = accumulate(m_pcounts.begin(), m_pcounts.end(), 0);
	std::cerr << "Total pass fail0 fail1 refinepatch: "
		<< trial << ' ' << pass << ' '
		<< fail0 << ' ' << fail1 << ' ' << pass + fail1 << std::endl;
	std::cerr << "Total pass fail0 fail1 refinepatch: "
		<< 100 * trial / (float)trial << ' '
		<< 100 * pass / (float)trial << ' '
		<< 100 * fail0 / (float)trial << ' '
		<< 100 * fail1 / (float)trial << ' '
		<< 100 * (pass + fail1) / (float)trial << std::endl;
}

void Seed::initialMatchT(void) {

	int id = 0;//m_df.m_count++;
	//std::list<int> tempL;
	//for (int i=0; i<=1;i++)
	//{
	//	int id2 = i;
		while (1)
		{
			//const int id = m_df.m_count++;
			int index = -1;
			if (!m_df.m_jobs.empty()) {
				index = m_df.m_jobs.front();
				m_df.m_jobs.pop_front();
				//tempL.push_front(index);
			}
			if (index == -1) {
				//m_df.m_jobs = tempL;
				break;
			}
			initialMatch(index, id);
		}
	//}
	UtilityM ut;
	ut.WritePlySimple("D:/DUSAN/3Lateral/PMVSviaOpenCV/vc++/vc++/test.ply",temp3Dpoint);
}
/*Ova metoda vraca celije koje ustvari predstavljaju tacke koje konstruisu liniju kada smo presikali tacku
sa jedne slike na drugu sliku uz pomoc fundamentalne matrice.*/
void Seed::collectCells(const int index0, const int index1,
	const Cpoint& p0, std::vector<Vec2i>& cells) {
	Vec3 point(p0.m_icoord[0], p0.m_icoord[1], p0.m_icoord[2]);
	const int gwidth = m_df.m_pos.getmgWidths(index1);
	const int gheight = m_df.m_pos.getmgHeights(index1);

	Mat3 F;
	UtilityM ut;
	ut.setF(m_df.getPhoto(index0), m_df.getPhoto(index1),F,m_df.getmLevel());

	Vec3 line = transpose(F) * point;
	if (line[0] == 0.0 && line[1] == 0.0) {
		std::cerr << "Point right on top of the epipole?"
			<< index0 << ' ' << index1 << std::endl;
		return;
	}
	// vertical
	if (fabs(line[0]) > fabs(line[1])) {
		for (int y = 0; y < gheight; ++y) {
			const float fy = (y + 0.5) * m_df.getmCsize() - 0.5f;
			float fx = (-line[1] * fy - line[2]) / line[0];
			fx = std::max((float)(INT_MIN + 3.0f), std::min((float)(INT_MAX - 3.0f), fx));

			const int ix = ((int)floor(fx + 0.5f)) / m_df.getmCsize();
			if (0 <= ix && ix < gwidth)
				cells.push_back(TVec2<int>(ix, y));
			if (0 <= ix - 1 && ix - 1 < gwidth)
				cells.push_back(TVec2<int>(ix - 1, y));
			if (0 <= ix + 1 && ix + 1 < gwidth)
				cells.push_back(TVec2<int>(ix + 1, y));
		}
	}
	else {
		for (int x = 0; x < gwidth; ++x) {
			const float fx = (x + 0.5) * m_df.getmCsize() - 0.5f;
			float fy = (-line[0] * fx - line[2]) / line[1];
			fy = std::max((float)(INT_MIN + 3.0f), std::min((float)(INT_MAX - 3.0f), fy));

			const int iy = ((int)floor(fy + 0.5f)) / m_df.getmCsize();
			if (0 <= iy && iy < gheight)
				cells.push_back(TVec2<int>(x, iy));
			if (0 <= iy - 1 && iy - 1 < gheight)
				cells.push_back(TVec2<int>(x, iy - 1));
			if (0 <= iy + 1 && iy + 1 < gheight)
				cells.push_back(TVec2<int>(x, iy + 1));
		}
	}

}
/*Skuplja ficere koji zadovoljavaju epipolarnu geometriju*/
void Seed::collectCandidates(const int index, const std::vector<int>& indexes,
	const Cpoint& point, std::vector<Ppoint>& vcp) {
	//u ovom delu nkon sto skupi celije koje je nasao usporedjuje prosledjenu tacku sa
	//potencijalnom tackom koja je njen par preko tipa. Tacke mogu piti uparene samo ako su detektovane istim detektorom(harris, dog)
	const Vec3 p0(point.m_icoord[0], point.m_icoord[1], 1.0);
	for (int i = 0; i < (int)indexes.size(); ++i) {
		const int indexid = indexes[i];

		std::vector<TVec2<int> > cells;
		collectCells(index, indexid, point, cells);
		Mat3 F;
		UtilityM ut;
		ut.setF(m_df.getPhoto(index), m_df.getPhoto(indexid), F, m_df.getmLevel());

		for (int i = 0; i < (int)cells.size(); ++i) {
			const int x = cells[i][0];      const int y = cells[i][1];
			if (!canAdd(indexid, x, y))
				continue;
			const int index2 = y * m_df.m_pos.getmgWidths(indexid) + x;

			std::vector<Ppoint>::iterator begin = m_ppoints[indexid][index2].begin();
			std::vector<Ppoint>::iterator end = m_ppoints[indexid][index2].end();
			while (begin != end) {
				Cpoint& rhs = **begin;
				// ? use type to reject candidates?
				if (point.m_type != rhs.m_type) {
					++begin;
					continue;
				}

				const Vec3 p1(rhs.m_icoord[0], rhs.m_icoord[1], 1.0);
				if (m_df.m_epThreshold <= ut.computeEPD(F, p0, p1)) {
					++begin;
					continue;
				}
				vcp.push_back(*begin);
				++begin;
			}
		}
	}

	// set distances to m_response

	std::vector<Ppoint> vcptmp;
	for (int i = 0; i < (int)vcp.size(); ++i) {
		unproject(index, vcp[i]->m_itmp, point, *vcp[i], vcp[i]->m_coord);

		if (m_df.getPhoto(index).m_projection[m_df.getmLevel()][2] *
			vcp[i]->m_coord <= 0.0)
			continue;
		//TODO 1: ovaj deo uraditi kada budemo ubacivali maske
		if (/*m_df.getMask(vcp[i]->m_coord, m_df.getmLevel()) == 0 ||*/
			m_df.insideBimages(vcp[i]->m_coord) == 0)
			continue;

		//??? from the closest
		vcp[i]->m_response =
			fabs(norm(vcp[i]->m_coord - m_df.getPhoto(index).m_center) -
				norm(vcp[i]->m_coord - m_df.getPhoto(vcp[i]->m_itmp).m_center));

		vcptmp.push_back(vcp[i]);
	}
	vcptmp.swap(vcp);
	sort(vcp.begin(), vcp.end());
}

void Seed::unproject(const int index0, const int index1,
	const Cpoint& p0, const Cpoint& p1,
	Vec4f& coord) const {
	Mat4 A;
	A[0][0] =
		m_df.getPhoto(index0).m_projection[m_df.getmLevel()][0][0] -
		p0.m_icoord[0] * m_df.getPhoto(index0).m_projection[m_df.getmLevel()][2][0];
	A[0][1] =
		m_df.getPhoto(index0).m_projection[m_df.getmLevel()][0][1] -
		p0.m_icoord[0] * m_df.getPhoto(index0).m_projection[m_df.getmLevel()][2][1];
	A[0][2] =
		m_df.getPhoto(index0).m_projection[m_df.getmLevel()][0][2] -
		p0.m_icoord[0] * m_df.getPhoto(index0).m_projection[m_df.getmLevel()][2][2];
	A[1][0] =
		m_df.getPhoto(index0).m_projection[m_df.getmLevel()][1][0] -
		p0.m_icoord[1] * m_df.getPhoto(index0).m_projection[m_df.getmLevel()][2][0];
	A[1][1] =
		m_df.getPhoto(index0).m_projection[m_df.getmLevel()][1][1] -
		p0.m_icoord[1] * m_df.getPhoto(index0).m_projection[m_df.getmLevel()][2][1];
	A[1][2] =
		m_df.getPhoto(index0).m_projection[m_df.getmLevel()][1][2] -
		p0.m_icoord[1] * m_df.getPhoto(index0).m_projection[m_df.getmLevel()][2][2];
	A[2][0] =
		m_df.getPhoto(index1).m_projection[m_df.getmLevel()][0][0] -
		p1.m_icoord[0] * m_df.getPhoto(index1).m_projection[m_df.getmLevel()][2][0];
	A[2][1] =
		m_df.getPhoto(index1).m_projection[m_df.getmLevel()][0][1] -
		p1.m_icoord[0] * m_df.getPhoto(index1).m_projection[m_df.getmLevel()][2][1];
	A[2][2] =
		m_df.getPhoto(index1).m_projection[m_df.getmLevel()][0][2] -
		p1.m_icoord[0] * m_df.getPhoto(index1).m_projection[m_df.getmLevel()][2][2];
	A[3][0] =
		m_df.getPhoto(index1).m_projection[m_df.getmLevel()][1][0] -
		p1.m_icoord[1] * m_df.getPhoto(index1).m_projection[m_df.getmLevel()][2][0];
	A[3][1] =
		m_df.getPhoto(index1).m_projection[m_df.getmLevel()][1][1] -
		p1.m_icoord[1] * m_df.getPhoto(index1).m_projection[m_df.getmLevel()][2][1];
	A[3][2] =
		m_df.getPhoto(index1).m_projection[m_df.getmLevel()][1][2] -
		p1.m_icoord[1] * m_df.getPhoto(index1).m_projection[m_df.getmLevel()][2][2];

	Vec4 b;
	b[0] =
		p0.m_icoord[0] * m_df.getPhoto(index0).m_projection[m_df.getmLevel()][2][3] -
		m_df.getPhoto(index0).m_projection[m_df.getmLevel()][0][3];
	b[1] =
		p0.m_icoord[1] * m_df.getPhoto(index0).m_projection[m_df.getmLevel()][2][3] -
		m_df.getPhoto(index0).m_projection[m_df.getmLevel()][1][3];
	b[2] =
		p1.m_icoord[0] * m_df.getPhoto(index1).m_projection[m_df.getmLevel()][2][3] -
		m_df.getPhoto(index1).m_projection[m_df.getmLevel()][0][3];
	b[3] =
		p1.m_icoord[1] * m_df.getPhoto(index1).m_projection[m_df.getmLevel()][2][3] -
		m_df.getPhoto(index1).m_projection[m_df.getmLevel()][1][3];

	Mat4 AT = transpose(A);
	Mat4 ATA = AT * A;
	Vec4 ATb = AT * b;

	Mat3 ATA3;
	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 3; ++x)
			ATA3[y][x] = ATA[y][x];
	Vec3 ATb3;
	for (int y = 0; y < 3; ++y)
		ATb3[y] = ATb[y];

	Mat3 iATA3;
	invert(iATA3, ATA3);
	Vec3 ans = iATA3 * ATb3;
	for (int y = 0; y < 3; ++y)
		coord[y] = ans[y];
	coord[3] = 1.0f;
}
using namespace Patch;
void Seed::initialMatch(const int index, const int id) {
	std::vector<int> indexes;
	m_df.m_optim.collectImages(index, indexes);

	if (m_df.m_tau < (int)indexes.size())
		indexes.resize(m_df.getNumOfImages());

	if (indexes.empty())
		return;

	int totalcount = 0;

	//======================================================================
	// for each feature point, starting from the optical center, keep on
	// matching until we find candidateThreshold patches
	const int gheight = m_df.m_pos.getmgHeights(index);
	const int gwidth = m_df.m_pos.getmgWidths(index);

	int index2 = -1;
	for (int y = 0; y < gheight; ++y) {
		for (int x = 0; x < gwidth; ++x) {
			++index2;
			if (!canAdd(index, x, y))
				continue;
			std::vector<Ppoint> vcptemp1;
			for (int p = 0; p < (int)m_ppoints[index][index2].size(); ++p) {
				// collect features that satisfies epipolar geometry
				// constraints and sort them according to the differences of
				// distances between two cameras.
				std::vector<Ppoint> vcp;
				collectCandidates(index, indexes,
					*m_ppoints[index][index2][p], vcp);
				vcptemp1 = vcp;
						int count = 0;
						Patch::Cpatch bestpatch;
				//		//======================================================================
						for (int i = 0; i < (int)vcp.size(); ++i) {
							Patch::Cpatch patch;
							patch.m_coord = vcp[i]->m_coord;
							patch.m_normal =
								m_df.getPhoto(index).m_center - patch.m_coord;

							unitize(patch.m_normal);
							patch.m_normal[3] = 0.0;
							patch.m_flag = 0;

							++m_df.m_pos.m_counts[index][index2];
							const int ix = ((int)floor(vcp[i]->m_icoord[0] + 0.5f)) / m_df.getmCsize();
							const int iy = ((int)floor(vcp[i]->m_icoord[1] + 0.5f)) / m_df.getmCsize();
							const int index3 = iy * m_df.m_pos.getmgWidths(vcp[i]->m_itmp) + ix;
							if (vcp[i]->m_itmp < m_df.getNumOfImages())
								++m_df.m_pos.m_counts[vcp[i]->m_itmp][index3];

							const int flag = initialMatchSub(index, vcp[i]->m_itmp, id, patch);
							if (flag == 0) {
								++count;
								if (bestpatch.score(m_df.m_nccThreshold) <
									patch.score(m_df.m_nccThreshold))
									bestpatch = patch;
								if (m_df.m_countThreshold0 <= count)
									break;
							}
						}
						if (count != 0) {
							Ppatch ppatch(new Cpatch(bestpatch));
							m_df.m_pos.addPatch(ppatch);
							++totalcount;
							break;
						}
					}
				}
			//if (!vcptemp1.empty()) {
			//	temp3Dpoint.push_back(vcptemp1);
			//	//temp3DpointB.insert(temp3DpointB.end(), vcptemp1[0]);
			//	vcptemp1.clear();
			//}
	}
	std::cerr << '(' << index << ',' << totalcount << ')' << std::flush;
}

int Seed::initialMatchSub(const int index0, const int index1,
	const int id, Patch::Cpatch& patch) {

	//----------------------------------------------------------------------
	patch.m_images.clear();
	patch.m_images.push_back(index0);
	patch.m_images.push_back(index1);
	//if (id > 1) {
	//	std::cerr << "id veci od 1" << std::endl;
	//}
	++m_scounts[id];

	//----------------------------------------------------------------------
	// We know that patch.m_coord is inside bimages and inside mask
	if (m_df.m_optim.preProcess(patch, id, 1)) {
		++m_fcounts0[id];
		return 1;
	}

	//----------------------------------------------------------------------  
	m_df.m_optim.refinePatch(patch, id, 100);

	////----------------------------------------------------------------------
	if (m_df.m_optim.postProcess(patch, id, 1)) {
		++m_fcounts1[id];
		return 1;
	}

	++m_pcounts[id];
	//----------------------------------------------------------------------
	return 0;
}


int Seed::canAdd(const int index, const int x, const int y) {
	/*if (!m_df.getMask(index, m_df.getmCsize() * x, m_df.getmCsize() * y, m_df.getmLevel()))
		return 0;*/

	const int index2 = y * m_df.m_pos.getmgWidths(index) + x;

	if (m_df.getNumOfImages() <= index)
		return 1;

	// Check if m_pgrids already contains something
	if (!m_df.m_pos.m_pgrids[index][index2].empty())
		return 0;

	//??? critical
	if (m_df.m_countThreshold2 <= m_df.m_pos.m_counts[index][index2])
		return 0;

	return 1;
}