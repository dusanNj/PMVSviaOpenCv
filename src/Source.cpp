#include<iostream>
#include<stdio.h>
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"Detector.h"
#include"DetectFeatures.h"
#include"Organizer.h"



int main(){
	std::string path = "D:\\DUSAN\\3Lateral\\PMVSviaOpenCV\\vc++\\vc++\\";
	std::string name = "option0.txt";
	DetectFeatures df;
	df.run(path, name,2);
	df.runMatching();

	return 0;
}