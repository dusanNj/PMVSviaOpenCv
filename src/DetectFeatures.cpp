#define _USE_MATH_DEFINES
#include <cmath>
#include "DetectFeatures.h"
#include<windef.h>
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define max(a,b)            (((a) > (b)) ? (a) : (b))



DetectFeatures::DetectFeatures() :m_pos(*this),  m_seed(*this), m_optim(*this), m_exp(*this), m_filt(*this)
{
}
//

DetectFeatures::~DetectFeatures()
{
	
}

void DetectFeatures::run(std::string path, std::string name,
						 const int csize) {
	Organizer readOprionFile(path,name);
	readOprionFile.init();
	std::vector<std::string> namesOfFile;
	readOprionFile.getFileFormDirectory(readOprionFile.getPathTofolder(),
										readOprionFile.getExtension(),namesOfFile);
	readOprionFile.initVisdata();

	int num = namesOfFile.size();
	std::string pathToF = readOprionFile.getPathTofolder();

	/*readOprionFile.m_oimages = 0;*/

	m_visdata = readOprionFile.m_visdatatemp;
	m_visdata2 = readOprionFile.m_visdata2temp;

	m_csize = csize;
	m_level = readOprionFile.getLevel();

	m_points.clear();
	m_points.resize(num);

	m_tau = (std::min)(readOprionFile.m_minImageNum * 2, num);

	//----------------------------------------------------------------------
	//for (int index = 0; index < num; ++index)
	//	m_jobs.push_back(index);

	//vector<thrd_t> threads(m_CPU);
	//for (int i = 0; i < m_CPU; ++i)
	//	thrd_create(&threads[i], &runThreadTmp, (void*)this);
	//for (int i = 0; i < m_CPU; ++i)
	//	thrd_join(threads[i], NULL);
	//----------------------------------------------------------------------

	//----------------------- Create image level
	UtilityM ut;
	Image img;
	std::vector<unsigned char> tempImgUCa;
	for (int i = 0; i < num;i++) {
		cv::Mat tempImg = cv::imread(pathToF+namesOfFile[i]);
		cv::Mat tempMask = cv::Mat::zeros(tempImg.size(), tempImg.type());
		cv::Mat tempedge = cv::Mat::zeros(tempImg.size(), tempImg.type());
		
		int w = tempImg.cols;
		int h = tempImg.rows;
		cv::Mat temp = tempImg.clone();
		ut.convertImgToUcharArray(temp, w,h,tempImgUCa);
		ImagesChar.push_back(tempImgUCa);
		alImgChar.push_back(ImagesChar);
		ImagesChar.clear();
		tempImgUCa.clear();

		ut.convertImgToUcharArray(tempMask, w, h, tempImgUCa);
		masksChar.push_back(tempImgUCa);
		tempImgUCa.clear();

		ut.convertImgToUcharArray(tempedge, w, h, tempImgUCa);
		edgesChar.push_back(tempImgUCa);
		tempImgUCa.clear();

		tempMask.release();
		tempedge.release();

		img.setWidthHeightByLevel(tempImg, imgWidth, imgHeight);

		tempImg.release();
		temp.release();
	}

	std::vector<std::vector<unsigned char>> tempL;
	for (int i = 0; i < num;i++) {
		img.buildImageByLevel(0, imgWidth[i], imgHeight[i], alImgChar[i]);
	}

	//----------------------- Finish Create image level
	
	//----------------------------------- calculate Camera parametras 
	//for (int i = 0; i < num;i++) {
	//	std::string namePath;
	//	std::string tempN;
	//	if (i<10) {
	//		tempN = "00" + std::to_string(i) + ".txt";
	//	}
	//	else if (i>=10 && i<100) {
	//		tempN = "0" + std::to_string(i) + ".txt";
	//	}
	//	else
	//	{
	//		tempN = std::to_string(i) + ".txt";
	//	}

	//	namePath = readOprionFile.getPathToTxt() + tempN;

	//	Photo photo(num);
	//	photo.init(namePath,3);
	//	photos.push_back(photo);
	//}
	//----------------------------------- Finish calculate Camera parametras 
	int n = getNumOfImages();
	std::cout << "numImg:" << n << std::endl;
	//----------------------------------- Start features detection
	RunFetureDetect();
	std::cerr << "done" << std::endl; 
	//----------------------------------- Finish features detection
	m_pos.init();
	m_seed.init(m_points);
	m_exp.init();
	m_filt.init();
	m_optim.init();
	
	m_angleThreshold0 = 60.0f * M_PI / 180.0f;
	m_angleThreshold1 = 60.0f * M_PI / 180.0f;

	m_countThreshold0 = 2;
	m_countThreshold1 = 4;
	m_countThreshold2 = 2;

	m_neighborThreshold = 0.5f;
	m_neighborThreshold1 = 1.0f;

	m_neighborThreshold2 = 1.0f;

	m_maxAngleThreshold = 10;

	m_nccThresholdBefore = m_nccThreshold - 0.3f;

	m_quadThreshold = 2.0;

	m_epThreshold = 2.0f;
	m_sequenceThreshold = -1;


}
//
//int DetectFeatures::countImageIndex() {
//	int i = 0;
//
//	//if (m_Image->empty()) {
//	//	return 0;
//	//}
//	//i++;
//	return i;
//}


void DetectFeatures::RunFetureDetect(void) {
	int cn = 0;
	do {
		//int numImg = countImageIndex();
		int image = cn+1;
		std::cerr << image << ' ' << std::flush;

		float sigma = 4.0;
		const float firstScale = 1.0f;    const float lastScale = 3.0/*3.0f*/;
		int maxLevel = 3;
		Harris harris(maxLevel);
		std::multiset<Cpoint> result;
		std::vector<std::vector<unsigned char>> imgCharDog = ImagesChar;
		std::vector<std::vector<unsigned char>> maskDogchar /*= masksChar*/;
		std::vector<std::vector<unsigned char>> edgeDogchar /*= edgesChar*/;
		masksChar[0].clear();
		edgesChar[0].clear();
		maskDogchar.push_back(masksChar[0]);
		edgeDogchar.push_back(edgesChar[0]);
		//------------------------------------------------------------

		std::vector<cv::Point2f> cornPosition;
		harris.run(alImgChar[cn], masksChar[0], edgesChar[0],
			imgWidth[cn][0], imgHeight[cn][0], m_csize, sigma, cornPosition, result);

		std::multiset<Cpoint>::reverse_iterator rbegin = result.rbegin();

		while (rbegin != result.rend()) {
			m_points[image - 1].push_back(*rbegin);
			rbegin++;
		}
		result.clear();

		

		cv::Mat drawFeaturesImg = cv::imread("D:\\DUSAN\\3Lateral\\PMVSviaOpenCV\\vc++\\vc++\\images\\00"+
												std::to_string(cn)+".png");
		UtilityM ut;
		ut.setDrawCorner(UtilityM::drawCornerFor::harris);
		ut.drawCorners(drawFeaturesImg, cornPosition,cn);
		cornPosition.clear();

		//-----------------------------------------------------------Dog
		std::multiset<Cpoint> resultD;
		Dog dog;
		std::vector<cv::Point2f> cornPositionDog;

		dog.run(alImgChar[cn], maskDogchar[0], edgeDogchar[0],
			imgWidth[cn][0], imgHeight[cn][0], m_csize, firstScale, lastScale, resultD, cornPositionDog);

		std::multiset<Cpoint>::reverse_iterator rbeginD = resultD.rbegin();
		while (rbeginD != resultD.rend()) {
			m_points[image - 1].push_back(*rbeginD);
			rbeginD++;
		}

		cv::Mat drawFeaturesImgD = cv::imread("D:\\DUSAN\\3Lateral\\PMVSviaOpenCV\\vc++\\vc++\\images\\00" +
			std::to_string(cn) + ".png");;
		UtilityM utD;
		utD.setDrawCorner(UtilityM::drawCornerFor::dog);
		utD.drawCorners(drawFeaturesImgD, cornPositionDog, cn);

		cn++;

	} while (alImgChar.size() > cn);
//
}

//================================================
int DetectFeatures::getMask(int index1,int x, int y, int level) {
	if (masksChar[level].empty())
		return 1;

	if (x < 0 || masksChar[level][index1] <= x || y < 0 || masksChar[level][index1] <= y)
		return 1;
	
	const int index = y * imgWidth[level][index1] + x;
	return masksChar[level][index1];
}
//================================================

void DetectFeatures::runMatching() {

	//m_seed.run();
	//m_seed.clear();

}