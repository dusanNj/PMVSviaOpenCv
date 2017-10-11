#include "Image.h"



Image::Image()
{
}


Image::~Image()
{
}

/*Ova metoda je zamenjena metodom buildImageByLevel*/
void Image::buildImage(const int filter, int w, int h, std::vector<unsigned char>& img) {
	Mat4 mask;
	mask[0] = Vec4(1.0, 3.0, 3.0, 1.0);  mask[1] = Vec4(3.0, 9.0, 9.0, 3.0);
	mask[2] = Vec4(3.0, 9.0, 9.0, 3.0);  mask[3] = Vec4(1.0, 3.0, 3.0, 1.0);
	
	float total = 64.0f;
	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 4; ++x)
			mask[y][x] /= total;

	//for (int level = 1; level < m_maxLevel; ++level) {
		const int size = w * h * 3;

		img.resize(size);

		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {

				Vec3 color;

				float denom = 0.0;

				for (int j = -1; j < 3; ++j) {
					const int ytmp = 2 * y + j;
					if (ytmp < 0 || h - 1 < ytmp)
						continue;

					for (int i = -1; i < 3; ++i) {
						const int xtmp = 2 * x + i;
						if (xtmp < 0 || w - 1 < xtmp)
							continue;

						const int index = (ytmp *w + xtmp) * 3;

						if (filter == 0) {
							color[0] += mask[j + 1][i + 1] * (double)img[index];
							color[1] += mask[j + 1][i + 1] * (double)img[index + 1];
							color[2] += mask[j + 1][i + 1] * (double)img[index + 2];
							denom += mask[j + 1][i + 1];
						}
						//else if (filter == 1) {
						//	color[0] = max(color[0], (double)m_images[level - 1][index]);
						//	color[1] = max(color[1], (double)m_images[level - 1][index + 1]);
						//	color[2] = max(color[2], (double)m_images[level - 1][index + 2]);
						//}
						//else {
						//	color[0] = min(color[0], (double)m_images[level - 1][index]);
						//	color[1] = min(color[1], (double)m_images[level - 1][index + 1]);
						//	color[2] = min(color[2], (double)m_images[level - 1][index + 2]);
						//}

					}
				}
				if (filter == 0)
					color /= denom;
				const int index = (y * w + x) * 3;
//#ifdef FURUKAWA_IMAGE_GAMMA
//				m_dimages[level][index] = color[0];
//				m_dimages[level][index + 1] = color[1];
//				m_dimages[level][index + 2] = color[2];
//#else
				img[index] = (unsigned char)((int)floor(color[0] + 0.5f));
				img[index + 1] = (unsigned char)((int)floor(color[1] + 0.5f));
				img[index + 2] = (unsigned char)((int)floor(color[2] + 0.5f));
//#endif
			}
		}
	//}

}

void Image::buildImageByLevel(const int filter, std::vector<int> w, std::vector<int> h, std::vector<std::vector<unsigned char>>& img) {
	Mat4 mask;
	mask[0] = Vec4(1.0, 3.0, 3.0, 1.0);  mask[1] = Vec4(3.0, 9.0, 9.0, 3.0);
	mask[2] = Vec4(3.0, 9.0, 9.0, 3.0);  mask[3] = Vec4(1.0, 3.0, 3.0, 1.0);

	
	std::vector<unsigned char> tempSize;
	float total = 64.0f;
	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 4; ++x)
			mask[y][x] /= total;
	int m_maxLevel = 3; 
	//----------------------------------------------------------------------
	// image
	for (int level = 1; level < m_maxLevel; ++level) {
		const int size = w[level] * h[level] * 3;
#ifdef FURUKAWA_IMAGE_GAMMA
		m_dimages[level].resize(size);
#else
		tempSize.resize(size);
		img.push_back(tempSize);
		img[level].resize(size);
#endif
		/*for (int i = 0; i < img[1].size(); i++) {
			if (img[1][i] != '\0') {
				std::cout << "pos:" << i << std::endl;
			}
		}*/
		for (int y = 0; y < h[level]; ++y) {
			for (int x = 0; x < w[level]; ++x) {

				Vec3 color;
				if (filter == 2)
					color[0] = color[1] = color[2] = 255.0;

				float denom = 0.0;

				for (int j = -1; j < 3; ++j) {
					const int ytmp = 2 * y + j;
					if (ytmp < 0 || h[level - 1] - 1 < ytmp)
						continue;

					for (int i = -1; i < 3; ++i) {
						const int xtmp = 2 * x + i;
						if (xtmp < 0 || w[level - 1] - 1 < xtmp)
							continue;

						const int index = (ytmp * w[level - 1] + xtmp) * 3;
#ifdef FURUKAWA_IMAGE_GAMMA
						if (filter == 0) {
							color[0] += mask[j + 1][i + 1] * (double)m_dimages[level - 1][index];
							color[1] += mask[j + 1][i + 1] * (double)m_dimages[level - 1][index + 1];
							color[2] += mask[j + 1][i + 1] * (double)m_dimages[level - 1][index + 2];
							denom += mask[j + 1][i + 1];
						}
						else if (filter == 1) {
							color[0] = max(color[0], (double)m_dimages[level - 1][index]);
							color[1] = max(color[1], (double)m_dimages[level - 1][index + 1]);
							color[2] = max(color[2], (double)m_dimages[level - 1][index + 2]);
						}
						else {
							color[0] = min(color[0], (double)m_dimages[level - 1][index]);
							color[1] = min(color[1], (double)m_dimages[level - 1][index + 1]);
							color[2] = min(color[2], (double)m_dimages[level - 1][index + 2]);
						}
#else
						if (filter == 0) {
							color[0] += mask[j + 1][i + 1] * (double)img[level - 1][index];
							color[1] += mask[j + 1][i + 1] * (double)img[level - 1][index + 1];
							color[2] += mask[j + 1][i + 1] * (double)img[level - 1][index + 2];
							denom += mask[j + 1][i + 1];
						}
						else if (filter == 1) {
							color[0] = std::max(color[0], (double)img[level - 1][index]);
							color[1] = std::max(color[1], (double)img[level - 1][index + 1]);
							color[2] = std::max(color[2], (double)img[level - 1][index + 2]);
						}
						else {
							color[0] = std::min(color[0], (double)img[level - 1][index]);
							color[1] = std::min(color[1], (double)img[level - 1][index + 1]);
							color[2] = std::min(color[2], (double)img[level - 1][index + 2]);
						}
#endif
					}
				}
				if (filter == 0)
					color /= denom;
				const int index = (y * w[level] + x) * 3;
#ifdef FURUKAWA_IMAGE_GAMMA
				m_dimages[level][index] = color[0];
				m_dimages[level][index + 1] = color[1];
				m_dimages[level][index + 2] = color[2];
#else
				img[level][index] = (unsigned char)((int)floor(color[0] + 0.5f));
				img[level][index + 1] = (unsigned char)((int)floor(color[1] + 0.5f));
				img[level][index + 2] = (unsigned char)((int)floor(color[2] + 0.5f));
#endif
			}
		}
	}
}

void Image::gray2rgb(const float gray, float& r, float& g, float& b) {
	if (gray < 0.5) {
		r = 0.0f;
		g = 2.0f * gray;
		b = 1.0f - g;
	}
	else {
		r = (gray - 0.5f) * 2.0f;
		g = 1.0f - r;
		b = 0.0f;
	}
}