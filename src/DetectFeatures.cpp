#define _USE_MATH_DEFINES
#include "DetectFeatures.h"
#include <cmath>
#include <windef.h>
#define min(a, b)            (((a) < (b)) ? (a) : (b))
#define max(a, b)            (((a) > (b)) ? (a) : (b))


DetectFeatures::DetectFeatures() : m_pos(*this), m_seed(*this), m_optim(*this), m_exp(*this), m_filt(*this) {
    m_nccThreshold = 0.7;
}

//

DetectFeatures::~DetectFeatures() {
}

void DetectFeatures::run(std::string path, std::string name, const int csize) {
    Organizer readOprionFile(path, name);
    readOprionFile.init();
    std::vector<std::string> namesOfFile;
    std::string s = readOprionFile.getPathTofolder();
    std::string nm = readOprionFile.getExtension();
    readOprionFile.getFileFormDirectory(readOprionFile.getPathTofolder(),
                                        readOprionFile.getExtension(), namesOfFile);
    readOprionFile.initVisdata();

    int num = namesOfFile.size();
    std::string pathToF = readOprionFile.getPathTofolder();
    nameOfimages = namesOfFile;
    pathtoFolder = pathToF;


    /*readOprionFile.m_oimages = 0;*/

    m_visdata = readOprionFile.m_visdatatemp;
    m_visdata2 = readOprionFile.m_visdata2temp;

    m_csize = csize;
    m_wsize = 5;
    m_minimagenumthresho = 2;
    m_level = readOprionFile.getLevel();
    setMaxLevel(m_level);
    readOprionFile.m_minImageNum = 2;
    m_points.clear();
    m_points.resize(num);
    m_tnum = readOprionFile.imgIndex.size();
    m_tau = (std::min)(readOprionFile.m_minImageNum * 2, num);

    // ----------------------------------------------------------------------
    // for (int index = 0; index < num; ++index)
    // m_jobs.push_back(index);

    // vector<thrd_t> threads(m_CPU);
    // for (int i = 0; i < m_CPU; ++i)
    // thrd_create(&threads[i], &runThreadTmp, (void*)this);
    // for (int i = 0; i < m_CPU; ++i)
    // thrd_join(threads[i], NULL);
    // ----------------------------------------------------------------------

    // ----------------------- Create image level
    UtilityM ut;
    Image img;
    std::vector<unsigned char> tempImgUCa;
    for (int i = 0; i < num; i++) {
        cv::Mat tempImg = cv::imread(pathToF + namesOfFile[i]);
        cv::cvtColor(tempImg, tempImg, CV_BGR2RGB);
        cv::Mat tempMask = cv::Mat::zeros(tempImg.size(), tempImg.type());
        cv::Mat tempedge = cv::Mat::zeros(tempImg.size(), tempImg.type());

        int w = tempImg.cols;
        int h = tempImg.rows;
        cv::Mat temp = tempImg.clone();
        ut.convertImgToUcharArray(temp, w, h, tempImgUCa);
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

        img.setWidthHeightByLevel(tempImg, imgWidth, imgHeight, m_level, getMaxLevel());

        tempImg.release();
        temp.release();
    }

    std::vector<std::vector<unsigned char> > tempL;
    for (int i = 0; i < num; i++) {
        img.buildImageByLevel(0, imgWidth[i], imgHeight[i], alImgChar[i], getMaxLevel());
    }

    // ----------------------- Finish Create image level

    // ----------------------------------- calculate Camera parametras
    Photo photo(num);
    for (int i = 0; i < num; i++) {
        std::string nameOfTxtFile = readOprionFile.setTxtFileName(nameOfimages[i]);
        std::string namePath;

        namePath = readOprionFile.getPathToTxt() + nameOfTxtFile;

        photo.init(namePath, getMaxLevel());
        photos.push_back(photo);
    }
    // ----------------------------------- Finish calculate Camera parametras
    int n = getNumOfImages();
    std::cout << "numImg:" << n << std::endl;
    // ----------------------------------- Start features detection
    RunFetureDetect();
    std::cerr << "done" << std::endl;
    // ----------------------------------- Finish features detection
    photo.setDistances(n, photos);
    mdf_distance = photo.m_distances;
    // std::cout << "photos[0].m_distances[0][0]:" <<mdf_distance[0][0] << std::endl;
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

    m_maxAngleThreshold = 10 * M_PI / 180.0f;

    m_nccThresholdBefore = m_nccThreshold - 0.3f;

    m_quadThreshold = 2.0;

    m_epThreshold = 2.0f;
    m_sequenceThreshold = -1;
    m_depth = 0;
}

//
// int DetectFeatures::countImageIndex() {
// int i = 0;
//
////if (m_Image->empty()) {
////	return 0;
////}
////i++;
// return i;
// }

void DetectFeatures::updateThreshold(void) {
    m_nccThreshold -= 0.05f;
    m_nccThresholdBefore -= 0.05f;

    m_countThreshold1 = 2;
}

int DetectFeatures::isNeighborRadius(const Patch::Cpatch& lhs,
                                     const Patch::Cpatch& rhs,
                                     const float hunit,
                                     const float neighborThreshold,
                                     const float radius) {
    if (lhs.m_normal * rhs.m_normal < cos(120.0 * M_PI / 180.0)) {
        return 0;
    }
    const Vec4f diff = rhs.m_coord - lhs.m_coord;

    const float vunit = lhs.m_dscale + rhs.m_dscale;

    const float f0 = lhs.m_normal * diff;
    const float f1 = rhs.m_normal * diff;
    float ftmp = (fabs(f0) + fabs(f1)) / 2.0;
    ftmp /= vunit;

    // this may loosen the isneighbor testing. need to tighten (decrease) threshold?
    const float hsize = norm(2 * diff - lhs.m_normal * f0 - rhs.m_normal * f1) / 2.0 / hunit;

    // radius check
    if (radius / hunit < hsize) {
        return 0;
    }

    if (1.0 < hsize) {
        ftmp /= min(2.0f, hsize);
    }

    if (ftmp < neighborThreshold) {
        return 1;
    } else {
        return 0;
    }
}

void DetectFeatures::RunFetureDetect(void) {
    int cn = 0;
    do {
        // int numImg = countImageIndex();
        int image = cn + 1;
        std::cerr << image << ' ' << std::flush;

        float sigma = 4.0;
        const float firstScale = 1.0f;
        const float lastScale = 3.0  /*3.0f*/;
        int maxLevel = 3;
        Harris harris(maxLevel);
        std::multiset<Cpoint> result;
        std::vector<std::vector<unsigned char> > imgCharDog = ImagesChar;
        std::vector<std::vector<unsigned char> > maskDogchar  /*= masksChar*/;
        std::vector<std::vector<unsigned char> > edgeDogchar  /*= edgesChar*/;
        masksChar[0].clear();
        edgesChar[0].clear();
        maskDogchar.push_back(masksChar[0]);
        edgeDogchar.push_back(edgesChar[0]);
        // ------------------------------------------------------------
        int fcsize = 16;
        std::vector<cv::Point2f> cornPosition;
        harris.run(alImgChar[cn], masksChar[0], edgesChar[0],
                   imgWidth[cn][m_level], imgHeight[cn][m_level], fcsize, sigma, cornPosition, result, m_level);

        std::multiset<Cpoint>::reverse_iterator rbegin = result.rbegin();

        while (rbegin != result.rend()) {
            m_points[image - 1].push_back(*rbegin);
            rbegin++;
        }
        result.clear();


        cv::Mat drawFeaturesImg = cv::imread(pathtoFolder + nameOfimages[cn]);
        UtilityM ut;
        ut.setDrawCorner(UtilityM::drawCornerFor::harris);
        ut.drawCorners(drawFeaturesImg, cornPosition, cn);
        cornPosition.clear();

        // -----------------------------------------------------------Dog
        std::multiset<Cpoint> resultD;
        Dog dog;
        std::vector<cv::Point2f> cornPositionDog;

        dog.run(alImgChar[cn], maskDogchar[0], edgeDogchar[0],
                imgWidth[cn][m_level], imgHeight[cn][m_level], fcsize, firstScale, lastScale, resultD, cornPositionDog, m_level);

        std::multiset<Cpoint>::reverse_iterator rbeginD = resultD.rbegin();
        while (rbeginD != resultD.rend()) {
            m_points[image - 1].push_back(*rbeginD);
            rbeginD++;
        }

        cv::Mat drawFeaturesImgD = cv::imread(pathtoFolder + nameOfimages[cn]);
        UtilityM utD;
        utD.setDrawCorner(UtilityM::drawCornerFor::dog);
        utD.drawCorners(drawFeaturesImgD, cornPositionDog, cn);

        cn++;
    } while (alImgChar.size() > cn);
    //
}

// ================================================
int DetectFeatures::getMask(int index1, int x, int y, int level) {
    if (masksChar[level].empty()) {
        return 1;
    }

    if ((x < 0) || (masksChar[level][index1] <= x) || (y < 0) || (masksChar[level][index1] <= y)) {
        return 1;
    }

    const int index = y * imgWidth[level][index1] + x;
    return masksChar[level][index1];
}

// ================================================
Vec3f DetectFeatures::getColor(const int indexIm, const float x, const float y, const int level) const {
    // Bilinear cases
    const int lx = static_cast<int>(x);
    const int ly = static_cast<int>(y);
    const int index = 3 * (ly * imgWidth[indexIm][level] + lx);

    const float dx1 = x - lx;
    const float dx0 = 1.0f - dx1;
    const float dy1 = y - ly;
    const float dy0 = 1.0f - dy1;

    const float f00 = dx0 * dy0;
    const float f01 = dx0 * dy1;
    const float f10 = dx1 * dy0;
    const float f11 = dx1 * dy1;
    const int index2 = index + 3 * imgWidth[indexIm][level];

    #ifdef IMAGE_GAMMA
        const float* fp0 = &m_dimages[level][index] - 1;
        const float* fp1 = &m_dimages[level][index2] - 1;
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        r += *(++fp0) * f00 + *(++fp1) * f01;
        g += *(++fp0) * f00 + *(++fp1) * f01;
        b += *(++fp0) * f00 + *(++fp1) * f01;
        r += *(++fp0) * f10 + *(++fp1) * f11;
        g += *(++fp0) * f10 + *(++fp1) * f11;
        b += *(++fp0) * f10 + *(++fp1) * f11;
        return Vec3f(r, g, b);
        /*
        return Vec3f(m_dimages[level][index] * f00 + m_dimages[level][index + 3] * f10 +
        m_dimages[level][index2] * f01 + m_dimages[level][index2 + 3] * f11,
        m_dimages[level][index + 1] * f00 + m_dimages[level][index + 4] * f10 +
        m_dimages[level][index2 + 1] * f01 + m_dimages[level][index2 + 4] * f11,
        m_dimages[level][index + 2] * f00 + m_dimages[level][index + 5] * f10 +
        m_dimages[level][index2 + 2] * f01 + m_dimages[level][index2 + 5] * f11);
        */
    #else
        const unsigned char* ucp0 = &alImgChar[indexIm][level][index] - 1;
        const unsigned char* ucp1 = &alImgChar[indexIm][level][index2] - 1;
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        r += *(++ucp0) * f00 + *(++ucp1) * f01;
        g += *(++ucp0) * f00 + *(++ucp1) * f01;
        b += *(++ucp0) * f00 + *(++ucp1) * f01;
        r += *(++ucp0) * f10 + *(++ucp1) * f11;
        g += *(++ucp0) * f10 + *(++ucp1) * f11;
        b += *(++ucp0) * f10 + *(++ucp1) * f11;
        return Vec3f(r, g, b);
    #endif
}

int DetectFeatures::checkAngles(const Vec4f& coord,
                                const std::vector<int>& indexes,
                                const float minAngle,
                                const float maxAngle,
                                const int num) const {
    int count = 0;

    std::vector<Vec4f> rays;
    rays.resize((int)indexes.size());
    for (int i = 0; i < (int)indexes.size(); ++i) {
        const int index = indexes[i];
        rays[i] = photos[index].m_center - coord;
        unitize(rays[i]);
    }

    for (int i = 0; i < (int)indexes.size(); ++i) {
        for (int j = i + 1; j < (int)indexes.size(); ++j) {
            const float dot = (std::max)(-1.0f, (std::min)(1.0f, rays[i] * rays[j]));
            const float angle = acos(dot);
            if ((minAngle < angle) && (angle < maxAngle)) {
                ++count;
            }
        }
    }

    // if (count < num * (num - 1) / 2)
    if (count < 1) {
        return 1;
    } else {
        return 0;
    }
}

int DetectFeatures::insideBimages(const Vec4f& coord) {
    for (int i = 0; i < (int)m_bindexes.size(); ++i) {
        int index = m_bindexes[i];
        Vec3f icoord = getPhoto(index).project(index, coord, m_level);
        if ((icoord[0] < 0.0) || (getWidtByIndex(index, m_level) - 1 < icoord[0]) ||
            (icoord[1] < 0.0) || (getHeightByIndex(index, m_level) - 1 < icoord[1])) {
            return 0;
        }
    }
    return 1;
}

void DetectFeatures::write(const std::string prefix, bool bExportPLY, bool bExportPatch, bool bExportPSet) {
    m_pos.writePatches2(prefix, bExportPLY, bExportPatch, bExportPSet);
}

void DetectFeatures::runMatching() {
    m_seed.run();
    m_pos.writePatchesAndImageProjections("", this->getNumOfImages());
    this->write("Test1", true, false, false);
    m_exp.run();
    m_pos.writePatchesAndImageProjections("", this->getNumOfImages());
    this->write("Test_exp", true, false, false);
    // m_seed.clear();
}
