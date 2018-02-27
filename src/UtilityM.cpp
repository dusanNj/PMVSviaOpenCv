#include "UtilityM.h"


UtilityM::UtilityM() {
}

UtilityM::~UtilityM() {
}

void UtilityM::drawCorners(cv::Mat& img, std::vector<cv::Point2f>& corn, int n, int level) {
    cv::Point2f center;

    // cv::resize(img, img, cv::Size(3000, 2000));
    for (int i = 0; i < corn.size(); i++) {
        center.x = corn[i].x * pow(2, (level));
        center.y = corn[i].y * pow(2, (level));

        cv::circle(img, center, 2, cv::Scalar(0, 0, 255), -1, 8);
    }

    switch (getDrawC()) {
        case UtilityM::dog:
            cv::imwrite("dog" + std::to_string(n) + ".png", img);
            break;
        case UtilityM::harris:
            cv::imwrite("harris" + std::to_string(n) + ".png", img);
            break;
        default:
            break;
    }
}

void UtilityM::convertImgToUcharArray(cv::Mat in, int width, int height, std::vector<unsigned char>& out) {
    for (int j = 0; j < height; j++) {
        for (int k = 0; k < width; k++) {
            for (int l = 0; l < 3; l++) {
                unsigned char* u = &in.at<cv::Vec3b>(j, k).val[l];
                out.push_back(*u);
            }
        }
    }
}

void UtilityM::transformImgFromUcharToMat(std::vector<unsigned char> v, cv::Mat& out, int width, int height) {
    int count = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < 3; k++) {
                out.at<cv::Vec3b>(i, j).val[k] = v[count++];
                std::cout << "r:" << count++ << std::endl;
            }
        }
    }
}

void UtilityM::WritePlySimple(std::string name, std::vector<std::vector<Ppoint> > point) {
    std::ofstream file;
    file.open(name.c_str());
    int n = 0;
    std::vector<Ppoint> tempPoint;
    for (int i = 0; i < point.size(); i++) {
        for (int j = 0; j < point[i].size(); j++) {
            tempPoint.push_back(point[i][j]);
            n = n + j;
        }
    }
    file << "ply" << std::endl;
    file << "format ascii 1.0" << std::endl;
    file << "element vertex " << tempPoint.size() << std::endl;
    file << "property float x" << std::endl;
    file << "property float y" << std::endl;
    file << "property float z" << std::endl;
    // file << "property uchar diffuse_red" << std::endl;
    // file << "property uchar diffuse_green" << std::endl;
    // file << "property uchar diffuse_blue" << std::endl;
    file << "end_header" << std::endl;

    for (int i = 0; i < tempPoint.size(); i++) {
        file << tempPoint[i]->m_coord[0] << " " << tempPoint[i]->m_coord[1] << " " << tempPoint[i]->m_coord[2] << std::endl;
    }
    file.close();
}
