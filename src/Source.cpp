#include "DetectFeatures.h"
#include "Detector.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Organizer.h"
#include <iostream>
#include <stdio.h>


int main() {
    std::string path = "";
    std::string name = "option0.txt";
    DetectFeatures df;
    df.m_wsize = 13;
    df.run(path, name, 1);
    df.num_of_expand = 1;
    df.runMatching();

    return 0;
}
