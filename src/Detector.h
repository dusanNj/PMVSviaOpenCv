#pragma once
#ifndef _DETECTOR_
#define _DETECTOR_
#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"vec3.h"
#include<set>
#include"Cpoint.h"

class Detector
{
public:
	Detector();
	~Detector();

	static void setGaussD(const float sigmaD, std::vector<float>& gaussD);
	static void setGaussI(const float sigmaI, std::vector<float>& gaussI);
	static float setThreshold(std::multiset<Cpoint>& grid);
	int isCloseBoundary(const int x, const int y, const int margin) const;




protected:
	int m_width;
	int m_height;
	std::vector<std::vector<Vec3f> > m_image;
	std::vector<std::vector<unsigned char> > m_mask;

public:
	template <class T>
	void convolveX(std::vector<std::vector<T> >& image,
		const std::vector<std::vector<unsigned char> >& mask,
		const std::vector<float>& filter,
		std::vector<std::vector<T> >& buffer) {
		const int width = image[0].size();
		const int height = image.size();
		const int margin = ((int)filter.size()) / 2;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				buffer[y][x] *= 0.0;

				if (!mask.empty() && mask[y][x] == 0)
					continue;

				for (int j = 0; j < (int)filter.size(); ++j) {
					int xtmp = x + j - margin;
					if (xtmp < 0)
						xtmp = 0;
					else if (width <= xtmp)
						xtmp = width - 1;

					const int ytmp = y;

					if (!mask.empty() && mask[ytmp][xtmp] == 0)
						continue;
					

					buffer[y][x] += filter[j] * image[ytmp][xtmp];


			/*		if (image[ytmp][xtmp] != 0.0) {
						std::cout << " xtmp:" << xtmp << std::endl;
						std::cout << " ytmp:" << ytmp << std::endl;
						std::cout << " image:" << image[ytmp][xtmp] << std::endl;
						std::cout << " buffer:" << buffer[y][x] << std::endl;
					}*/

				}
			}
		}

		buffer.swap(image);
	}


	template <class T>
	void convolveY(std::vector<std::vector<T> >& image,
		const std::vector<std::vector<unsigned char> >& mask,
		const std::vector<float>& filter,
		std::vector<std::vector<T> >& buffer) {
		const int width = image[0].size();
		const int height = image.size();
		const int margin = ((int)filter.size()) / 2;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				buffer[y][x] *= 0.0;

				if (!mask.empty() && mask[y][x] == 0)
					continue;

				for (int j = 0; j < (int)filter.size(); ++j) {
					const int xtmp = x;
					int ytmp = y + j - margin;
					if (ytmp < 0)
						ytmp = 0;
					else if (height <= ytmp)
						ytmp = height - 1;

					if (!mask.empty() && mask[ytmp][xtmp] == 0)
						continue;

					buffer[y][x] += filter[j] * image[ytmp][xtmp];
				}
			}
		}

		buffer.swap(image);
	}

	template <class T>
	void convolveX(std::vector<std::vector<T> >& image,
		const std::vector<float>& filter,
		std::vector<std::vector<T> >& buffer) {
		const int width = image[0].size();
		const int height = image.size();
		const int margin = ((int)filter.size()) / 2;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				buffer[y][x] *= 0.0;

				for (int j = 0; j < (int)filter.size(); ++j) {
					const int xtmp = x + j - margin;
					const int ytmp = y;
					if (xtmp < 0 || width <= xtmp)
						continue;

					buffer[y][x] += filter[j] * image[ytmp][xtmp];
				}
			}
		}

		buffer.swap(image);
	}


	template <class T>
	void convolveY(std::vector<std::vector<T> >& image,
		const std::vector<float>& filter,
		std::vector<std::vector<T> >& buffer) {
		const int width = image[0].size();
		const int height = image.size();
		const int margin = ((int)filter.size()) / 2;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				buffer[y][x] *= 0.0;

				for (int j = 0; j < (int)filter.size(); ++j) {
					const int xtmp = x;
					const int ytmp = y + j - margin;
					if (ytmp < 0 || height <= ytmp)
						continue;

					buffer[y][x] += filter[j] * image[ytmp][xtmp];
				}
			}
		}

		buffer.swap(image);
	}

};



#endif // !_DETECTOR_



