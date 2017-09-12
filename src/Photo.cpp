#include "Photo.h"
# define M_PI           3.14159265358979323846


Photo::Photo(int numOfimages)
{
	m_num = numOfimages;
}


Photo::~Photo()
{
}

void Photo::init(/*const std::string name, const std::string mname,
	const std::string ename,*/
	const std::string cname, const int maxLevel) {
	Camera::init(cname, maxLevel);
}
/*Racunaanje rastojanja izmedju ptickih centara kamere i racunanje ugla izedju dve kamere, 
postoji angle treshold koji mora biti zadovoljen zatim se postavljaju dobijene distance*/

void Photo::setDistances(void) {
	m_num;
	m_distances.resize(m_num);
	float avedis = 0.0f;
	int denom = 0;
	for (int i = 0; i < m_num; ++i) {
		m_distances[i].resize(m_num);
		for (int j = 0; j < m_num; ++j) {
			if (i == j)
				m_distances[i][j] = 0.0f;
			else {
				const float ftmp = norm(cam_m_center[i]/*.m_center*/ - cam_m_center[j]/*.m_center*/);
				m_distances[i][j] = ftmp;
				avedis += ftmp;
				denom++;
			}
		}
	}
	if (denom == 0)
		return;

	avedis /= denom;
	if (avedis == 0.0f) {
		std::cerr << "All the optical centers are identical..?" << std::endl;
		exit(1);
	}

	// plus angle difference
	for (int i = 0; i < m_num; ++i) {
		Vec4f ray0 = cam_m_oaxis[i]/*m_photos[i].m_oaxis*/;
		ray0[3] = 0.0f;
		for (int j = 0; j < m_num; ++j) {
			Vec4f ray1 = cam_m_oaxis[i]/*m_photos[j].m_oaxis*/;
			ray1[3] = 0.0f;

			m_distances[i][j] /= avedis;
			const float margin = cos(10.0f * M_PI / 180.0f);
			const float dis = std::max(0.0f, 1.0f - ray0 * ray1 - margin);
			m_distances[i][j] += dis;
		}
	}
}
