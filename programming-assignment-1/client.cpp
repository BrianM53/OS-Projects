/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Brian McKeown
	UIN: 431005648
	Date: 
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include "iostream"
#include "math.h"
#include <sys/wait.h>

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	int m = MAX_MESSAGE;
	int new_chan = false;
	vector<FIFORequestChannel*> channels; 
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'c':
				new_chan = true;
				break;
		}
	}

	//give args for server
	//server needs './server', '-m', '<val for  - m args>', 'NULL'
	//fork
	//In the child, run execvp using server args
	string mStr = to_string(m);
	pid_t child = fork();
	if (child == 0) {
		char *server_args[] = {(char*) "./server", (char*) "-m",(char*) mStr.c_str(), nullptr}; 
		int ret = -1;
        ret = execvp(server_args[0], server_args);
		if(ret < 0) {
			cout << "Error";
		}
	}
	else {
		FIFORequestChannel con_chan("control", FIFORequestChannel::CLIENT_SIDE);
		channels.push_back(&con_chan);
		if (new_chan){
			MESSAGE_TYPE nc = NEWCHANNEL_MSG;
			
			con_chan.cwrite(&nc, sizeof(MESSAGE_TYPE));
			//create a variable to hold name
			char name[100];
			//cread the response from server
			con_chan.cread(name, sizeof(name));
			//call FIFORequest Channel constructor with name from server	
			//recomend doing this dynamically
			FIFORequestChannel* new_channel = new FIFORequestChannel(name, FIFORequestChannel::CLIENT_SIDE);
			//push the new channel into the vector
			channels.push_back(new_channel);
		}
		
		FIFORequestChannel chan = *(channels.back());
		//single data point p,t,e != -1
		if (p != -1 and t != -1 and e != -1){
			// example data point request
			char buf[MAX_MESSAGE]; // 256
			datamsg x(p, t, e); //change from hardcoding to users values

			memcpy(buf, &x, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double reply;
			chan.cread(&reply, sizeof(double)); //answer
			cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		}
		else if (p != -1) {
			//loop over first 1000 lines
			char buf[MAX_MESSAGE];
			double curTime;
			string filepath = "received/x1.csv";
			ofstream file(filepath);
			for (int i = 0; i < 1000; i++){
				curTime = i * 0.004;
				//send request for ecg1 and ecg2
				datamsg x1(p, curTime, 1);
				memcpy(buf, &x1, sizeof(datamsg));
				chan.cwrite(buf, sizeof(datamsg)); // question
				double reply1;
				chan.cread(&reply1, sizeof(double)); //answer
				datamsg x2(p, curTime, 2);
				memcpy(buf, &x2, sizeof(datamsg));
				chan.cwrite(buf, sizeof(datamsg)); // question
				double reply2;
				chan.cread(&reply2, sizeof(double)); //answer

				//write line to recieved/x1.csv

				if (!file.is_open()){
					cerr << "Failed to open file: " << filepath << endl;
				}
				else {
					file << curTime << "," << reply1 << "," << reply2 << endl;
				}
			}
			file.close();
		}
		else if(filename != "") {

			//sending a nonsense message change this
			filemsg fm(0, 0);
			string fname = filename;
			
			int len = sizeof(filemsg) + (fname.size() + 1);
			char* buf2 = new char[len];
			memcpy(buf2, &fm, sizeof(filemsg));
			strcpy(buf2 + sizeof(filemsg), fname.c_str());
			chan.cwrite(buf2, len);  // I want the file length;

			int64_t filesize = 0;
			chan.cread(&filesize, sizeof(int64_t));

			char* buf3 = new char[m]; //create buffer of size buff capacity m

			//loop over the segments in the file filesize / buff capcity(m)

			string filepath = "received/" + filename;
			ofstream file(filepath, ios::binary);
			int remaining = filesize;
			while (remaining > 0) {
				//create file msg inst
				filemsg* file_req = (filemsg*) buf2;
				file_req->offset = filesize - remaining;//set offset in the file

				if (remaining < m){
					file_req->length = remaining; //set the length be careful of the last seg
				}
				else {
					file_req->length = m;
				}
				//send the request (buf2)
				chan.cwrite(buf2, len);
				//recieve the response		
				//cread into buf3 length file_req ->len
				chan.cread(buf3, file_req->length);
				//write buf3 into file recieved/filename
				file.write(buf3, file_req->length);

				remaining -= m;
			}

			file.close();

			delete[] buf2;
			delete[] buf3;
		}
		if (new_chan) {
			MESSAGE_TYPE nc = QUIT_MSG;
			channels[1]->cwrite(&nc, sizeof(MESSAGE_TYPE));
			delete channels[1];
		}

		MESSAGE_TYPE nm = QUIT_MSG;
		con_chan.cwrite(&nm, sizeof(MESSAGE_TYPE));

		for (size_t i = 2; i < channels.size(); i++) {
			MESSAGE_TYPE nd = QUIT_MSG;
			channels[i]->cwrite(&nd, sizeof(MESSAGE_TYPE));
			delete channels[i];
		}
	}
	return 0;
}
