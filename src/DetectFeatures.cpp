#include "DetectFeatures.h"



DetectFeatures::DetectFeatures()
{
}


DetectFeatures::~DetectFeatures()
{
	
}

void DetectFeatures::run(std::string path, std::string name,
						 const int csize, const int maxLevel) {
	Organizer readOprionFile(path,name);
	readOprionFile.init();
	std::vector<std::string> namesOfFile;
	readOprionFile.getFileFormDirectory(readOprionFile.getPathTofolder(),
										readOprionFile.getExtension(),namesOfFile);

	int num = namesOfFile.size();


	m_Images = &images;

	m_csize = csize;
	m_level = maxLevel;

	m_points.clear();
	m_points.resize(num);

	//----------------------------------------------------------------------
	//for (int index = 0; index < num; ++index)
	//	m_jobs.push_back(index);

	//vector<thrd_t> threads(m_CPU);
	//for (int i = 0; i < m_CPU; ++i)
	//	thrd_create(&threads[i], &runThreadTmp, (void*)this);
	//for (int i = 0; i < m_CPU; ++i)
	//	thrd_join(threads[i], NULL);
	//----------------------------------------------------------------------


	UtilityM ut;
	std::vector<unsigned char> tempImgUCa;
	for (int i = 0; i < m_Images->size();i++) {
		cv::Mat tempMask = cv::Mat::zeros(m_Images->at(i).size(), m_Images->at(i).type());
		cv::Mat tempedge = cv::Mat::zeros(m_Images->at(i).size(), m_Images->at(i).type());

		int w = m_Images->at(i).cols;
		int h = m_Images->at(i).rows;
		cv::Mat temp = m_Images->at(i).clone();
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
	}
	std::vector<cv::Mat> imgtemp = *m_Images;
	Image img;
	img.setWidthHeightByLevel(imgtemp,imgWidth,imgHeight);

	std::vector<std::vector<unsigned char>> tempL;
	for (int i = 0; i < m_Images->size();i++) {
		img.buildImageByLevel(0, imgWidth[i], imgHeight[i], alImgChar[i]);
	}
	

	RunFetureDetect();
	std::cerr << "done" << std::endl;


}

int DetectFeatures::countImageIndex() {
	int i = 0;

	//if (m_Image->empty()) {
	//	return 0;
	//}
	//i++;
	return i;
}


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
		std::vector<std::vector<unsigned char>> tempImg = alImgChar[cn];
		harris.run(alImgChar[cn], masksChar[0], edgesChar[0],
			m_Images->at(cn).cols, m_Images->at(cn).rows, m_csize, sigma, cornPosition, result);

		std::multiset<Cpoint>::reverse_iterator rbegin = result.rbegin();

	/*	std::multiset<Cpoint>::iterator bg = result.begin();
		std::multiset<Cpoint>::iterator end = result.end();*/
		

		while (rbegin != result.rend()) {
			m_points[image - 1].push_back(*rbegin);
			rbegin++;
		}
		result.clear();
		cv::Mat drawFeaturesImg = m_Images->at(cn).clone();
		UtilityM ut;
		ut.setDrawCorner(UtilityM::drawCornerFor::harris);
		ut.drawCorners(drawFeaturesImg, cornPosition,cn);
		cornPosition.clear();

		//-----------------------------------------------------------Dog
		std::multiset<Cpoint> resultD;
		Dog dog;
		std::vector<cv::Point2f> cornPositionDog;

		dog.run(alImgChar[cn], maskDogchar[0], edgeDogchar[0],
			m_Images->at(cn).cols, m_Images->at(cn).rows, m_csize, firstScale, lastScale, resultD, cornPositionDog);

		std::multiset<Cpoint>::reverse_iterator rbeginD = resultD.rbegin();
		while (rbeginD != resultD.rend()) {
			m_points[image - 1].push_back(*rbeginD);
			rbeginD++;
		}

		cv::Mat drawFeaturesImgD = m_Images->at(cn).clone();
		UtilityM utD;
		utD.setDrawCorner(UtilityM::drawCornerFor::dog);
		utD.drawCorners(drawFeaturesImgD, cornPositionDog, cn);

		cn++;

	} while (m_Images->size() > cn);
//
}