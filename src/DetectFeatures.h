#pragma once

#ifndef _DETECT_FEATURES
#define _DETECT_FEATURES
# define M_PI           3.14159265358979323846
#include<iostream>
#include<stdio.h>
#include<math.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"Cpoint.h"
#include"Harris.h"
#include"UtilityM.h"
#include<set>
#include"Dog.h"
#include"Organizer.h"
//#include"Detector.h"
#include"PatchOrganizer.h"
#include"Photo.h"
#include"Filter.h"
#include"Expand.h"
#include"Optim.h"
#include"Seed.h"

#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <queue>
#include <boost/shared_ptr.hpp>


class DetectFeatures
{

public:

	DetectFeatures();
	virtual ~DetectFeatures();

	void run(std::string path,std::string name,
			 const int csize);

	void RunFetureDetect(void);
	//int countImageIndex(void);


	std::vector<std::vector<Cpoint> > m_points;

	int getNumOfImages() {
		return (int)alImgChar.size();
	}

	//getMetode
	int getmCsize() {
		return m_csize;
	}
	int getmLevel() {
		return m_level;
	}

	int getWidtByIndex(int ind, int level) {
		return imgWidth[ind][level];
	}

	int getHeightByIndex(int ind, int level) {
		return imgHeight[ind][level];
	}

	Photo getPhoto(int index) {
		return photos[index];
	}


	//std::vector<int> getm_width(int index) {
	//	return imgWidth[index];
	//}

	/*Koristimo klasu Detectfeatures slicno kao sto je u PMVS_U findMatch*/
	//Kreirani objekti drugih klasa
	PatchOrganizer m_pos;
	Seed m_seed;
	Expand m_exp;
	Filter m_filt;
	Optim m_optim;
	
	/*Ovde cemo ucitati kamere koje kreiramo da bi mogli da im pristupimo kasnije*/
	std::vector<Photo> photos;
	std::vector<std::vector<int>> imgWidth;
	std::vector<std::vector<int>> imgHeight;
	int m_count;
	//jobs
	std::list<int> m_jobs;

	void runMatching();
	int getMask(int index,int x, int y, int level);
	// For first feature matching. Images within this angle are used in
	// matching.
	float m_nccThreshold = 0.7;
	float m_quadThreshold;

	float m_angleThreshold0;
	// tigher angle
	float m_angleThreshold1;

	// Number of success generation from each seed point
	int m_countThreshold0;
	// Number of counts, expansion can be tried
	int m_countThreshold1;

	// Number of trials for each cell in seed
	int m_countThreshold2;

	// Parameter for isNeighbor in findemptyblocks
	float m_neighborThreshold;
	// Parameter for isNeighbor in filterOutside
	float m_neighborThreshold1;
	// Parameter for filterNeighbor
	float m_neighborThreshold2;

	// ncc threshold before optim
	float m_nccThresholdBefore;
	// Maximum angle of images must be at least as large as this
	float m_maxAngleThreshold;

	// visibility consistency threshold
	float m_visibleThreshold;
	float m_visibleThresholdLoose;

	// Epipolar distance in seed generation
	float m_epThreshold;
	// visdata from SfM. m_num x m_num matrix
	std::vector<std::vector<int> > m_visdata;
	// an array of relavant images
	std::vector<std::vector<int> > m_visdata2;
	// sequence Threshold
	int m_sequenceThreshold;
	// Maximum number of images used in the optimization
	int m_tau;
	int m_wsize;
	int m_minimagenumthresho;
	//TODO:6a getMask za canADdd
	//int getMask(int index, int x, int y, int level);
	//TODO1
	//int getMask2(const Vec4f& coord, const int level) const {
	//	for (int index = 0; index < m_num; ++index)
	//		if (getMask(coord, index, level) == 0)
	//			return 0;
	//	return 1;
	//};

	std::vector<std::string> nameOfimages;
	std::string pathtoFolder;

	std::vector < std::vector<float> >mdf_distance;//distanca koja se dobije za photos
	Vec3f getColor(const int index, const float fx, const float fy,
		const int level) const;
	int checkAngles(const Vec4f& coord,
		const std::vector<int>& indexes,
		const float minAngle, const float maxAngle,
		const int num) const;
protected:
	//const cv::Mat *m_Image;
	std::vector<cv::Mat> *m_Images;
	cv::Mat *m_Img;
	int m_csize;
	int m_level;

	std::vector<std::vector<unsigned char>> ImagesChar;
	std::vector<std::vector<std::vector<unsigned char>>> alImgChar;
	std::vector<std::vector<unsigned char>> masksChar;
	std::vector<std::vector<unsigned char>> edgesChar;


};




#endif // _DETECT_FEATURES




