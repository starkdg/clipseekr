#include <iostream>
#include <string>
#include "FrameReader.hpp"

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

using namespace std;

int main(int argc, char **argv){
    if (argc < 2){
	cout << "not enough args" << endl;
	exit(0);
    }
    int pixfmt = PIX_FMT_YUVJ420P;
    string filename = argv[1];
    cout << "file: " << filename << endl;

    FrameReader *pReader = new FrameReader(filename, pixfmt);

    int width, height;
    pReader->GetSize(width, height);

    cout << "Size: " << width << "x" << height << endl;

    int64_t total;
    pReader->GetNumberFrames(total);

    cout << "Number frames: " << total << endl;

    int64_t current = 0;
    AVFrame *pframe;

    cvNamedWindow("clipreader", CV_WINDOW_AUTOSIZE);
    IplImage *img = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 1);

    do {
	pframe = pReader->GetFrame(current);
	cvSetData(img, pframe->data[0], pframe->linesize[0]);
	cvShowImage("clipreader", img);
	cvWaitKey(10);
	current++;
    } while (current < total);


    cout << "Done." << endl;
    pReader->~FrameReader();
    cout << "Done." << endl;
    cvDestroyAllWindows();
    return 0;
}
