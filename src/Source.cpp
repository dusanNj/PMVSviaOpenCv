#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"Detector.h"
#include"DetectFeatures.h"
#include"Organizer.h"



int main(){
	std::string path = "D:\\DUSAN\\3Lateral\\W33\\PMVS-OurSolution\\PmvsOurSolution\\PmvsOurSolution\\";
	std::string name = "option0.txt";
	Organizer redOptionFile(path, name);
	redOptionFile.init();
	redOptionFile.readImages();
	std::vector<cv::Mat> imgs;
	imgs = redOptionFile.getImages();
	int n = redOptionFile.getImagesCount();
	//cv::Mat inimg = cv::imread("D:\\DUSAN\\3Lateral\\W33\\PMVS-OurSolution\\PmvsOurSolution\\PmvsOurSolution\\00000000.jpg");
	DetectFeatures df;
	df.run(imgs,n,16,3);
	

	return 0;
}