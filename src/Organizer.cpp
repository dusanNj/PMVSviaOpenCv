#include "Organizer.h"



Organizer::Organizer(std::string path, std::string name)
{
	pathTofile = path;
	nameOfFile = name;
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