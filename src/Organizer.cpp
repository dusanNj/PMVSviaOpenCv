#include "Organizer.h"



Organizer::Organizer(std::string path, std::string name)
{
	pathTofile = path;
	nameOfFile = name;
	m_oimages.push_back(0);
	m_oimages.clear();
}

Organizer::~Organizer()
{
	imgIndex.clear();
	images.clear();
}

void Organizer::init() {

	std::string file = pathTofile + nameOfFile;
	std::ifstream ifstr;
	ifstr.open(file.c_str());

	while (1)
	{
		std::string name;
		ifstr >> name;

		if (ifstr.eof()) {
			break;
		}

		if (name[0] == '#') {
			char buffer[1024];
			ifstr.putback('#');
			ifstr.getline(buffer, 1024);
			continue;
		}

		if (name == "images") {
			ifstr >> imgs;
			if (imgs == -1) {
				int firstimage, lastimage;
				ifstr >> firstimage >> lastimage;

				for (int i = firstimage; i < lastimage; ++i) {
					imgIndex.push_back(i);
				}
			}
			else if (0 < imgs) {
				for (int i = 0; i < imgs; i++)
				{
					int itmp;
					ifstr >> itmp;
					imgIndex.push_back(itmp);
				}
			}
			else {
				std::cerr << "img is no valid:" << imgs << std::endl;
			}
		}
		else if (name == "folder") {
			ifstr >> pathTofolder;
		}
		else if (name == "extension") {
			ifstr >> extension;
		}
		else if (name == "txtfile") {
			ifstr >> pathToTxtsFile;
		}
		else if (name == "level") {
			ifstr >> level;
		}
		else if (name == "m_minImageNum") {
			ifstr >> m_minImageNum;
		}
	}
}

void Organizer::readImages() {
	cv::Mat tempImg;
	for (int i = 0; i < imgIndex.size(); i++) {
		if (i < 10) {
				tempImg = cv::imread(imagesFolder + "00" + std::to_string(imgIndex[i]) + ".png");
				if (tempImg.empty()) {
					throw "Image is empty!";
				}
		}
		else if (i > 9 && i < 100) {
				tempImg = cv::imread(imagesFolder + "0" + std::to_string(imgIndex[i]) + ".png");
				if (tempImg.empty()) {
					throw "Image is empty!";
				}
		}
		else {
				tempImg = cv::imread(imagesFolder + std::to_string(imgIndex[i]) + ".png");
				if (tempImg.empty()) {
					throw "Image is empty!";
				}
		}
		images.push_back(tempImg);
		tempImg.release();
	}
}

void Organizer::getFileFormDirectory(std::string pathToFolder, std::string extension, std::vector<std::string>& returnFileNameList) {
	
	std::string names;
	std::string search_path = pathToFolder + extension;
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				returnFileNameList.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
}

void Organizer::initVisdata(void) {
	// Case classifications. Set m_visdata by using vis.dat or not.
	int m_useVisData = 0;
	if (m_useVisData == 0) {
		const int tnum = (int)imgIndex.size();
		const int onum = (int)m_oimages.size();
		const int num = tnum + onum;
		m_visdatatemp.resize(num);
		m_visdata2temp.resize(num);
		for (int y = 0; y < num; ++y) {
			m_visdatatemp[y].resize(num);
			for (int x = 0; x < num; ++x)
				if (x == y)
					m_visdatatemp[y][x] = 0;
				else {
					m_visdatatemp[y][x] = 1;
					m_visdata2temp[y].push_back(x);
				}
		}
	}
	else
		initVisdata2();
}
void Organizer::initVisdata2(void) {

}
//void Organizer::initVisdata2(void) {
//	std::string svisdata = m_prefix + string("vis.dat");
//
//	vector<int> images;
//	images.insert(images.end(), m_timages.begin(), m_timages.end());
//	images.insert(images.end(), m_oimages.begin(), m_oimages.end());
//	map<int, int> dict2;
//	for (int i = 0; i < (int)images.size(); ++i)
//		dict2[images[i]] = i;
//
//	ifstream ifstr;
//	ifstr.open(svisdata.c_str());
//	if (!ifstr.is_open()) {
//		cerr << "No vis.dat although specified to initVisdata2: " << endl
//			<< svisdata << endl;
//		exit(1);
//	}
//
//	string header;  int num2;
//	ifstr >> header >> num2;
//
//	m_visdata2.resize((int)images.size());
//	for (int c = 0; c < num2; ++c) {
//		int index0;
//		map<int, int>::iterator ite0 = dict2.find(c);
//		if (ite0 == dict2.end())
//			index0 = -1;
//		else
//			index0 = ite0->second;
//		int itmp;
//		ifstr >> itmp >> itmp;
//		for (int i = 0; i < itmp; ++i) {
//			int itmp2;
//			ifstr >> itmp2;
//			int index1;
//			map<int, int>::iterator ite1 = dict2.find(itmp2);
//			if (ite1 == dict2.end())
//				index1 = -1;
//			else
//				index1 = ite1->second;
//
//			if (index0 != -1 && index1 != -1)
//				m_visdata2[index0].push_back(index1);
//		}
//	}
//	ifstr.close();
//
//	const int num = (int)images.size();
//	m_visdata.clear();
//	m_visdata.resize(num);
//	for (int y = 0; y < num; ++y) {
//		m_visdata[y].resize(num);
//		fill(m_visdata[y].begin(), m_visdata[y].end(), 0);
//		for (int x = 0; x < (int)m_visdata2[y].size(); ++x)
//			m_visdata[y][m_visdata2[y][x]] = 1;
//	}
//
//	// check symmetry
//	for (int i = 0; i < (int)m_visdata.size(); ++i) {
//		for (int j = i + 1; j < (int)m_visdata.size(); ++j) {
//			if (m_visdata[i][j] != m_visdata[j][i]) {
//				m_visdata[i][j] = m_visdata[j][i] = 1;
//			}
//		}
//	}
//}