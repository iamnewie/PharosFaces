/*
* Copyright (c) 2011. Philipp Wagner <bytefish[at]gmx[dot]de>.
* Released to public domain under terms of the BSD Simplified license.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of the organization nor the names of its contributors
*     may be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*   See <http://www.opensource.org/licenses/bsd-license>
*/

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
#include<winsock2.h>
#include <windows.h>
#define WIN32_LEAN_AND_MEAN

using namespace cv;
using namespace std;

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
	std::ifstream file(filename.c_str(), ifstream::in);
	if (!file) {
		string error_message = "No valid input file was given, please check the given filename.";
		CV_Error(CV_StsBadArg, error_message);
	}
	string line, path, classlabel;

	while (getline(file, line)) {
		stringstream liness(line);
		getline(liness, path, separator);
		getline(liness, classlabel);
		if (!path.empty() && !classlabel.empty()) {
			images.push_back(imread(path, 0));
			labels.push_back(atoi(classlabel.c_str()));
		}
	}
}

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#define BUFFER_SIZE 512

int OpenSocket(int argc, const char *argv[])
{
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char *message;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(2000);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
	}

	puts("Bind done");

	//Listen to incoming connections
	listen(s, 10);

	//Accept and incoming connection

	puts("Waiting for incoming connections...");

	c = sizeof(struct sockaddr_in);

	new_socket = accept(s, (struct sockaddr *)&client, &c);
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
	}

	puts("Connection accepted");

	//Code STREAM CODE HERE

	//	int max;
	int iResult = 0;
	int width;
	int height;


	int count;

	char responsebuff[BUFFER_SIZE];

		iResult = recv(new_socket, responsebuff, BUFFER_SIZE, 0);
		if (iResult > 0) {

			string str(responsebuff);
			printf("Response buff is %s",responsebuff);
			size_t found = str.find("SIZE");
			if (found != string::npos) {
				char * split;
				split = strtok(responsebuff, ";");
				split = strtok(NULL, ";");
				width = atoi(split);
				split = strtok(NULL, ";");
				height = atoi(split);

				printf("Request Received: %d\n", iResult);

				printf("Width = %d", width);
				printf("Height = %d", height);

				char ack[] = "ACK";
				if (send(new_socket, ack, sizeof(ack), 0) == SOCKET_ERROR)
					printf("\nSENDING ACK FAILED");
			
				printf("\nSending ACK , %d\n",sizeof(ack));
			}

		}
		if (iResult == SOCKET_ERROR) {
			printf("SOCKET ERROR");
		}


		char *recvbuf = (char*)malloc(sizeof(char) * (width * height));

	//FILE *output;
	//output = fopen("Ctest.jpg", "a");

	ofstream file;
	file.open("Test.jpg", ios::out | ios::binary);
	assert(file.is_open());

	int readbyte = 0;
	do {
		iResult = recv(new_socket, recvbuf, (width * height), 0);
		if (iResult > 0) {
			file.write(recvbuf,iResult);
			printf("Bytes received: %d\n", iResult);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());

		readbyte += iResult;
	} while (iResult > 0);
	file.close();

	//fwrite(recvbuf, sizeof(char), readbyte, output);
	//fclose(output);
	printf("%d", count);

	//Code image here

	// Check for valid command line arguments, print usage
	// if no arguments were given.

	if (argc != 3) {
		cout << "usage: " << argv[0] << " </path/to/haar_cascade> </path/to/csv.ext> </path/to/device id>" << endl;
		cout << "\t </path/to/haar_cascade> -- Path to the Haar Cascade for face detection." << endl;
		cout << "\t </path/to/csv.ext> -- Path to the CSV file with the face database." << endl;
		//cout << "\t </path/to/jpg.ext> -- Path to the JPG file for comparing." << endl;
		//cout << "\t <device id> -- The webcam device id to grab frames from." << endl;
		//exit(1);
	}
	// Get the path to your CSV:
	/*Mat image;
	string fn_haar = string(argv[1]);
	string fn_csv = string(argv[2]);
	//int deviceId = atoi(argv[3]);
	image = imread(argv[3]);
	// These vectors hold the images and corresponding labels:
	vector<Mat> images;
	vector<int> labels;
	// Read in the data (fails if no valid input filename is given, but you'll get an error message):
	try {
		read_csv(fn_csv, images, labels);
	}
	catch (cv::Exception& e) {
		cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
		// nothing more we can do
		exit(1);
	}*/
	// Get the height from the first image. We'll need this
	// later in code to reshape the images to their original
	// size AND we need to reshape incoming faces to this size:
	/*int im_width = images[0].cols;
	int im_height = images[0].rows;
	// Create a FaceRecognizer and train it on the given images:
	Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
	model->train(images, labels);
	// That's it for learning the Face Recognition model. You now
	// need to create the classifier for the task of Face Detection.
	// We are going to use the haar cascade you have specified in the
	// command line arguments:
	//
	CascadeClassifier haar_cascade;
	haar_cascade.load(fn_haar);
	// Get a handle to the Video device:
	//VideoCapture cap(deviceId);
	// Check if we can use this device at all:
	//if (!cap.isOpened()) {
	//	cerr << "Capture Device ID " << deviceId << "cannot be opened." << endl;
	//	return -1;
	//}
	// Holds the current frame from the Video device:
	//Mat frame;*/

	for (;;) {

		/*Mat original = image.clone();

		Mat gray;
		cvtColor(original, gray, CV_BGR2GRAY);

		vector< Rect_<int> > faces;
		haar_cascade.detectMultiScale(gray, faces);

		for (int i = 0; i < faces.size(); i++) {
			Rect face_i = faces[i];
			Mat face = gray(face_i);
			Mat face_resized;
			cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);
			int prediction = model->predict(face_resized);
			string name;
			if (prediction == 0) {
				name = "Atek";
			}
			else
			{
				name = "Nelson";
			}
			rectangle(original, face_i, CV_RGB(0, 255, 0), 1);
			string box_text = format("Prediction = %d", prediction);
			int pos_x = max(face_i.tl().x - 10, 0);
			int pos_y = max(face_i.tl().y - 10, 0);
			putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 2.0);
		}*/
		//Mat imagenew = Mat(360, 260, CV_8U, &recvbuf).clone();
		Mat imagenew = imread("Ctest.jpg");

		// Show the result:
		//imshow("face_recognizer", imagenew);
		// And display it:
		char key = (char)waitKey(20);
		// Exit this loop on escape:
		if (key == 27)
			break;
	}


	//End of Code image here
	//Reply to client
	message = "Hello Client , I have received your connection. But I have to go now, bye\n";
	send(new_socket, message, strlen(message), 0);

	getchar();

	closesocket(s);
	WSACleanup();

	return 0;
}


int main(int argc, const char *argv[]) {

	OpenSocket(argc, argv);

	return 0;
}