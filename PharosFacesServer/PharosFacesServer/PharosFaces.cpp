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
#include <winapifamily.h>
#define WIN32_LEAN_AND_MEAN

using namespace cv;
using namespace std;

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#define BUFFER_SIZE 512

//Function identifier
void faceRecognition(char fileName[]);
static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';');
int OpenSocket(int argc, const char *argv[]);

int argcTemp;
char argvTemp[];

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
		return 0;
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
		return 0;
	}

	puts("Connection accepted");

	//Code STREAM CODE HERE

	//Listen for response code for Initiation
	int iResult = 0;
	int width;
	int height;
	int count;
	char responsebuff[BUFFER_SIZE];

	iResult = recv(new_socket, responsebuff, BUFFER_SIZE, 0);
	if (iResult > 0) {

		string str(responsebuff);
		printf("Response buff is %s", responsebuff);
		size_t found = str.find("SIZE");
		if (found != string::npos) {
			char * split;
			split = strtok(responsebuff, ";");
			split = strtok(NULL, ";");

			// Get the width and height of the image
			width = atoi(split);
			split = strtok(NULL, ";");
			height = atoi(split);

			printf("Request Received: %d\n", iResult);

			printf("Width = %d", width);
			printf("Height = %d", height);

			char ack[] = "ACK";
			if (send(new_socket, ack, sizeof(ack), 0) == SOCKET_ERROR)
				printf("\nSENDING ACK FAILED");

			printf("\nSending ACK , %d\n", sizeof(ack));
		}

	}
	if (iResult == SOCKET_ERROR) {
		printf("SOCKET ERROR");
	}

	//Start receiving bytes stream
	char* recvbuf = (char*)malloc(sizeof(char) * (width * height));

	FILE* tempfile;
	char tempFileName[L_tmpnam]; // Create temporary file name
	tmpnam(tempFileName);
	tempfile = fopen(tempFileName, "wb");

	int readbyte = 0;
	do {
		iResult = recv(new_socket, recvbuf, (width * height), 0);

		if (iResult > 0) {
			fwrite(recvbuf,sizeof(char),iResult,tempfile);
			printf("Bytes received: %d\n", iResult);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());

		readbyte += iResult;
	} while (iResult > 0);

	printf("Total Bytes Received = %d", readbyte);

	printf("%s", tempFileName);
	faceRecognition(tempFileName);
	fclose(tempfile);
	closesocket(s);
	WSACleanup();

	return 0;
}

// Do face recognition here
void faceRecognition(char fileName[], String csvPath) {

	vector<Mat> images;
	vector<int> labels;
	try {
		read_csv(getenv("DATASET_CSV"), images, labels);
	}
	catch (Exception e) {
		cerr << "Error opening file" << getenv("DATASET_CSV") << " Reason: " << e.msg << endl;
		exit(1);
	}

	if (images.size() <= 1) {
		string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
		CV_Error(CV_StsError, error_message);
	
	}

	int height = images[0].row;

	Mat receivedImage = imread(fileName, CV_LOAD_IMAGE_COLOR);
	Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
	model->train(images, labels);



	
}


int main(int argc, const char *argv[]) {

	if (argc != 3) {
		cout << "usage: " << argv[0] << " </path/to/haar_cascade> </path/to/csv.ext> </path/to/device id>" << endl;
		cout << "\t </path/to/haar_cascade> -- Path to the Haar Cascade for face detection." << endl;
		cout << "\t </path/to/csv.ext> -- Path to the CSV file with the face database." << endl;
		cout << "\t <device id> -- The webcam device id to grab frames from." << endl;
		exit(1);
	}

	OpenSocket(argc, argv);

	return 0;
}