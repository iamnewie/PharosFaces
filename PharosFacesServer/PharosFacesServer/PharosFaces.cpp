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
#include <conio.h>
#include<winsock2.h>
#include <windows.h>
#include <winapifamily.h>

#include <mysql.h>

#define WIN32_LEAN_AND_MEAN

using namespace cv;
using namespace std;

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#define BUFFER_SIZE 512

//Function identifier
int faceRecognition(char fileName[], Ptr<FaceRecognizer> model);

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


// Do face recognition here
int faceRecognition(char fileName[], Ptr<FaceRecognizer> model) {
	Mat receivedImage = imread(fileName, CV_LOAD_IMAGE_COLOR);
	Mat receivedImageFlipped;               // dst must be a different Mat
	flip(receivedImage, receivedImageFlipped, 1);
	Mat gray;
	cvtColor(receivedImageFlipped,gray,CV_BGR2GRAY);

	int predictedLabel = model->predict(gray);
	return predictedLabel;
}

Ptr<FaceRecognizer> training(Ptr<FaceRecognizer> model) {

	vector<Mat> images;
	vector<int> labels;

	try {
		printf("\nDATASET = %s", getenv("DATASET_CSV"));
		read_csv("dataset.csv", images, labels);
	}
	catch (Exception e) {
		cerr << "Error opening file" << getenv("DATASET_CSV") << " Reason: " << e.msg << endl;
		exit(1);
	}

	if (images.size() <= 1) {
		string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
		CV_Error(CV_StsError, error_message);
	}

	int height = images[0].rows;
	model = createEigenFaceRecognizer(10, numeric_limits<double>::infinity());
	model->train(images, labels);
	return model;
}


int main(int argc, const char *argv[]) {

	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	Ptr<FaceRecognizer> model;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	const char *server_mysql = "localhost";
	const char *user = "root";
	const char *password = "";
	const char *database = "pharos";
	const int port = 3306;

	
	//Start training
	model = training(model);

	//Start loop to accept connection
	while (true)
	{
		printf("\nInitialising Winsock...");
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			printf("Failed. Error Code : %d", WSAGetLastError());
			return 0;
		}

		printf("Initialised.\n");

		//Create a socket
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			printf("Could not create socket : %d", WSAGetLastError());
		}

		printf("Socket created.\n");

		//Prepare the sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(2000);

		//Bind
		if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
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
		if (new_socket == INVALID_SOCKET) {
			printf("accept failed with error code : %d", WSAGetLastError());
			return 0;
		}

		puts("Connection accepted");

		//Code STREAM CODE HERE

		//Listen for response code for Initiation
		int iResult = 0;
		int width;
		int height;
		int totalByte = 0;

		char responsebuff[BUFFER_SIZE];
		size_t found = NULL;
		char * split;
		string userid;

		iResult = recv(new_socket, responsebuff, BUFFER_SIZE, 0);

		if (iResult > 0) {

			responsebuff[iResult] = '\0';
			string str(responsebuff);

			cout << "Response buff is " << str << endl;

			if (str.find("SIZE") != string::npos) {
				
				split = strtok(responsebuff, ";");
				split = strtok(nullptr, ";");

				// Get the width and height of the image
				width = atoi(split);
				split = strtok(nullptr, ";");
				height = atoi(split);
				split = strtok(nullptr, ";");
				totalByte = atoi(split);

				printf("Request Received: %d\n", iResult);

				printf("Width = %d", width);
				printf("Height = %d", height);
				printf("Total byte about to be received = %d", totalByte);

				if (send(new_socket, "ACK", sizeof("ACK"), 0) == SOCKET_ERROR) {
					printf("\nSENDING ACK FAILED");

				}
				else{
					printf("\nSending ACK , %d\n", sizeof("ACK"));
					
				}
			}
			
			if (str.find("LOGOUT") != string::npos) {
				
				conn = mysql_init(nullptr);
				if (!mysql_real_connect(conn, server_mysql, user, password, database, port, NULL, 0)) {
					printf("koneksi gagal");
					return 0;
				}
				split = strtok(responsebuff, ";"); 
				split = strtok(nullptr, ";");
				userid = split;

				string query = "insert into log(id,status) values('";
				query += userid + "','0')";
				mysql_query(conn,query.c_str());
				mysql_close(conn);

				closesocket(s);
				WSACleanup();
				continue;
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
			recvbuf[iResult] = '/0';
			if (iResult > 0) {
				fwrite(recvbuf, sizeof(char), iResult, tempfile);
				printf("Bytes received: %d\n", iResult);
			}
			else if (iResult == 0)
				printf("Connection closed\n");
			else
				printf("recv failed: %d\n", WSAGetLastError());
			readbyte += iResult;
			//break loop sampai semua byte keterima
			if (readbyte == totalByte) {
				break;
			}
		} while (iResult > 0);

		printf("Total Bytes Received = %d\n", readbyte);

		printf("%s", tempFileName);
		int karyawanID = faceRecognition(tempFileName,model);
		printf("karyawan ID : %d\n", karyawanID);


		// Membuka koneksi ke mysql server
		conn = mysql_init(nullptr);
		if (!mysql_real_connect(conn, server_mysql, user, password, database, port, nullptr, 0)) {
			printf("koneksi gagal");
			return 0;
		}

		// Query string
		String query = "select name from staff where id=";
		query += to_string(karyawanID);
		mysql_query(conn, query.c_str());
		res = mysql_store_result(conn);

		int num_fields = mysql_num_fields(res);

		// Fetch all rows from the result

		String username;

		while ((row = mysql_fetch_row(res))) {
			for (int i = 0; i < num_fields; i++)
			{
				if (row[i] != nullptr) {
					cout << row[i] << endl;
					username = row[i];
				}
			}
		}

		String message = to_string(karyawanID) + ";" + username;
		//Send username to client

		if (send(new_socket, message.c_str(), sizeof(message), 0) == SOCKET_ERROR)
			printf("\nSENDING USERNAME FAILED");

		char confirmBuff[BUFFER_SIZE];

		iResult = recv(new_socket, confirmBuff, BUFFER_SIZE, 0);


		if (iResult > 0) {
			confirmBuff[iResult] = '\0';

			if (strcmp(confirmBuff, "yes") == 0) {

				printf("Confirm buff is %s \n", confirmBuff);
				query = "select count(*) from log where DATE(NOW()) = DATE(date) and id='" + to_string(karyawanID) + "' and status='1'";

				mysql_query(conn, query.c_str());

				res = mysql_store_result(conn);

				num_fields = mysql_num_fields(res);
				String dateCount;

				while ((row = mysql_fetch_row(res))) {
					for (int i = 0; i < num_fields; i++)
					{
						if (row[i] != nullptr) {
							cout << row[i] << endl;
							dateCount = row[i];
						}
					}
				}

				if (dateCount.compare("1") == 0) {

					if (send(new_socket, "FAIL", sizeof("FAIL"), 0) == SOCKET_ERROR) {
						printf("\nSENDING FAIL MESSAGE FAILED");
					}
					else
					printf("\nSending FAIL Message , %d\n", sizeof("FAIL"));
				}
				else{
					query = "insert into log(id, status) values('" + to_string(karyawanID) + " ',' " + "1')";
					mysql_query(conn, query.c_str()); 
					if (send(new_socket, "SUCCESS", sizeof("SUCCESS"), 0) == SOCKET_ERROR) {
						printf("\nSENDING SUCCESS MESSAGE FAILED");
					}
					else
					printf("\nSending SUCCESS Message , %d\n", sizeof("SUCCESS"));
				}
			}

			if (strcmp(confirmBuff, "no") == 0) {
				printf("Confirm buff is %s \n", confirmBuff);
			}
		}
		mysql_close(conn);
		fclose(tempfile);
		closesocket(s);
		WSACleanup();
	}
}