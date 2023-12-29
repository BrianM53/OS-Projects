#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"

// ecgno to use for datamsgs
#define EGCNO 1

using namespace std;

void patient_thread_function(int n, int p, BoundedBuffer *reqBuf)
{
    // functionality of the patient threads
    datamsg data(p, 0.0, 1);

    for (int i = 0; i < n; i++)
    {
        reqBuf->push((char *)&data, sizeof(data));
        data.seconds += 0.004;
    }
}

void file_thread_function(int m, string filename, FIFORequestChannel *chan, BoundedBuffer *reqBuf)
{
    // functionality of the file thread
    filemsg fm(0, 0);
    string fname = filename;

    long long len = sizeof(filemsg) + (sizeof(fname) + 1);
    char buf2[1024];
    memcpy(buf2, &fm, sizeof(filemsg));

    strcpy(buf2 + sizeof(filemsg), fname.c_str());

    chan->cwrite(buf2, len); // I want the file length;

    int64_t filesize = 0;
    chan->cread(&filesize, sizeof(int64_t));

    // loop over the segments in the file filesize / buff capcity(m)

    string filepath = "received/" + filename;

    FILE *filePointer = fopen(filepath.c_str(), "wb");
    fseek(filePointer, filesize, SEEK_SET);
    fclose(filePointer);

    int remaining = filesize;

    filemsg *newFile = (filemsg *)buf2;
    newFile->mtype = FILE_MSG;

    while (remaining > 0)
    {
        if (remaining < m)
        {
            newFile->length = remaining; // set the length be careful of the last seg
        }
        else
        {
            newFile->length = m;
        }
        // send the request (buf2)
        // recieve the response

        reqBuf->push((char *)newFile, len);
        newFile->offset += newFile->length;
        remaining -= newFile->length;
        // memcpy(buf2, &fm, sizeof(filemsg));
    }
}

void worker_thread_function(int m, int p, string filename, FIFORequestChannel *chan, BoundedBuffer *reqBuf, BoundedBuffer *popBuf)
{

    while (true)
    {
        char *popMsg = new char[m];

        int serverMsg = reqBuf->pop(popMsg, m);

        MESSAGE_TYPE msgType = *(MESSAGE_TYPE *)popMsg;

        if (msgType == QUIT_MSG)
        {
            chan->cwrite((char *)&msgType, serverMsg);
            delete chan;
            delete[] popMsg;
            break;
        }
        else if (msgType == DATA_MSG)
        {
            // functionality of the worker threads
            char buf[MAX_MESSAGE];          // 256
            datamsg x = *(datamsg *)popMsg; // change from hardcoding to users values

            memcpy(buf, &x, sizeof(datamsg));
            chan->cwrite(buf, sizeof(datamsg)); // question
            double reply;
            chan->cread(&reply, sizeof(double)); // answer
            pair<int, double> pData;
            pData.second = reply;
            pData.first = x.person;
            popBuf->push((char *)&pData, sizeof(pData));
        }
        else if (msgType == FILE_MSG)
        {

            filemsg *file = (filemsg *)popMsg;

            int len = sizeof(filemsg) + (filename.size() + 1);
            char *buf3 = new char[file->length];

            chan->cwrite(popMsg, len);
            chan->cread(buf3, file->length);
            // write buf3 into file recieved/filename
            string filepath = "received/" + filename;
            FILE *filePointer = fopen(filepath.c_str(), "rb+");
            fseek(filePointer, file->offset, SEEK_SET);
            fwrite(buf3, 1, file->length, filePointer);
            fclose(filePointer);

            delete[] buf3;
        }
        delete[] popMsg;
    }
    p++;
}

void histogram_thread_function(HistogramCollection *hc, BoundedBuffer *popBuf)
{
    // functionality of the histogram threads
    /*
    -pop from response buffer (you poped the pair)
    -call hist update w/ obv args
    */

    while (true)
    {
        pair<int, double> pData;
        popBuf->pop((char *)&pData, sizeof(pData));

        if (pData.first < 0)
        {
            break;
        }

        hc->update(pData.first, pData.second);
    }
}

int main(int argc, char *argv[])
{
    int n = 1000;        // default number of requests per "patient"
    int p = 10;          // number of patients [1,15]
    int w = 100;         // default number of worker threads
    int h = 20;          // default number of histogram threads
    int b = 20;          // default capacity of the request buffer (should be changed)
    int m = MAX_MESSAGE; // default capacity of the message buffer
    string f = "";       // name of file to be transferred

    // read arguments
    int opt;
    while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            n = atoi(optarg);
            break;
        case 'p':
            p = atoi(optarg);
            break;
        case 'w':
            w = atoi(optarg);
            break;
        case 'h':
            h = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 'm':
            m = atoi(optarg);
            break;
        case 'f':
            f = optarg;
            break;
        }
    }

    // fork and exec the server
    int pid = fork();
    if (pid == 0)
    {
        execl("./server", "./server", "-m", (char *)to_string(m).c_str(), nullptr);
    }

    // initialize overhead (including the control channel)
    FIFORequestChannel *chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
    HistogramCollection hc;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++)
    {
        Histogram *h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }

    /*create all threads here*/
    /*
    if data:
        -create p patient threads
        -create w worker threads
            ->create channel
        -create h histogram treads

    if file:

    */

    vector<FIFORequestChannel *> channels;

    thread *wHandler = new thread[w];
    thread *pHandler = new thread[p];
    thread *hHandler = new thread[h];
    thread fileHandler;

    for (int i = 0; i < w; i++)
    {
        // worker_thread_function(int m, int p, string filename, FIFORequestChannel chan, BoundedBuffer reqBuf, BoundedBuffer popBuf)
        MESSAGE_TYPE nc = NEWCHANNEL_MSG;

        chan->cwrite(&nc, sizeof(MESSAGE_TYPE));
        // create a variable to hold name
        char name[MAX_MESSAGE];
        // cread the response from server
        chan->cread(name, sizeof(name));
        // call FIFORequest Channel constructor with name from server
        // recomend doing this dynamically
        FIFORequestChannel *new_channel = new FIFORequestChannel(name, FIFORequestChannel::CLIENT_SIDE);
        // push the new channel into the vector
        channels.push_back(new_channel);
    }

    if (f.empty())
    {
        for (int i = 0; i < p; i++)
        {
            // patient_thread_function(int n, int p, BoundedBuffer *reqBuf)
            pHandler[i] = thread(patient_thread_function, n, i + 1, &request_buffer);
        }

        for (int i = 0; i < h; i++)
        {
            // histogram_thread_function(HistogramCollection hc, BoundedBuffer popBuf)
            hHandler[i] = thread(histogram_thread_function, &hc, &response_buffer);
        }
    }
    else if (!f.empty())
    {
        // void file_thread_function(int m, string filename, FIFORequestChannel *chan, BoundedBuffer *reqBuf)
        fileHandler = thread(file_thread_function, m, f, chan, &request_buffer);
    }
    for (int i = 0; i < w; i++)
    {
        wHandler[i] = thread(worker_thread_function, m, p, f, channels[i], &request_buffer, &response_buffer);
    }

    // record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /* create all threads here */

    /* join all threads here */

    if (!f.empty()) {
        fileHandler.join();
    }

    if (f.empty())
    {
        for (int i = 0; i < p; i++)
        {
            pHandler[i].join();
        }
    }

    /* Join patient threads */

    for (int i = 0; i < w; i++)
    {
        MESSAGE_TYPE q = QUIT_MSG;
        request_buffer.push((char *)&q, sizeof(MESSAGE_TYPE));
    }

    /* Join worker threads */
    for (int i = 0; i < w; i++)
    {
        wHandler[i].join();
    }

    /* Join histogram threads */
    for (int i = 0; i < h; i++)
    {
        pair<int, double> pData;
        pData.first = -1;
        pData.second = -1.0;
        response_buffer.push((char *)&pData, sizeof(pData));
    }

    if (f.empty())
    {

        /* Join histogram threads */
        for (int i = 0; i < h; i++)
        {
            hHandler[i].join();
        }

    }

    // record end time
    gettimeofday(&end, 0);

    // print the results
    if (f == "")
    {
        hc.print();
    }
    int secs = ((1e6 * end.tv_sec - 1e6 * start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int)1e6);
    int usecs = (int)((1e6 * end.tv_sec - 1e6 * start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int)1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

    // quit and close control channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite((char *)&q, sizeof(MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

    /* for (int i = 0; i < w; i++)
    {
        delete channels[i];
    } */

    delete[] wHandler;
    delete[] hHandler;
    delete[] pHandler;

    // wait for server to exit
    wait(nullptr);
}
