#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include<io.h>
#include<stdio.h>


using namespace cv;
using namespace std;


/*int main(int argc, char** argv) {

	vector<Mat> images;
	vector<int> labels;

	for (int i = 1; i < argc; i++) {
		char* imagename = argv[i];
		printf("image name : %s", imagename);
		Mat image = imread(imagename,1);
		Mat gray;
		cvtColor(image, gray, CV_BGR2GRAY);
		imwrite(imagename, gray);
	}
	return 0;
}*/