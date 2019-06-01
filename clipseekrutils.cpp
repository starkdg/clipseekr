#include <iostream>
#include <cstring>
#include <cmath>
#include "clipseekrutils.h"
#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>

using namespace std;

bool isblackframe(AVFrame *pframe, int pixel_threshold, int black_threshold){
	int num_black = 0; // no. black pixels
	int numpixels = pframe->width * pframe->height;
	uint8_t *p = pframe->data[0];
	for (int i=0;i<pframe->height;i++){
		for (int j=0;j<pframe->width;j++){
			num_black += p[j] < pixel_threshold;
		}
		p += pframe->linesize[0];
	}

	if (num_black * 100 /numpixels >= black_threshold){
		return true;
	}
	return false;
}

int64_t process_timestamp(int64_t ts, AVRational tb){
	return av_rescale(ts, tb.num, tb.den);
}


void process_timestamp(int64_t ts, AVRational tb, int &hrs, int &mins, int &secs){
	int64_t seconds = av_rescale(ts, tb.num, tb.den);
	hrs  =  seconds/3600;
	int64_t s1 = seconds%3600;
	mins = s1/60;
	secs = s1%60;
}


void print_metadata(ph::MetaData &mdata){
	cout << "--------info-------------" << endl;
	cout << "title: " << mdata.title_str << endl;
	cout << "artist: " << mdata.artist_str << endl;
	cout << "album: " << mdata.album_str << endl;
	cout << "genre: " << mdata.genre_str << endl;
	cout << "composer: " << mdata.composer_str << endl;
	cout << "performer: " << mdata.performer_str << endl;
	cout << "album artist: " << mdata.album_artist_str << endl;
	cout << "copyright: " << mdata.copyright_str << endl;
	cout << "date: " << mdata.date_str << endl;
	cout << "track: " << mdata.track_str << endl;
	cout << "disc: " << mdata.disc_str << endl;
	cout << "--------------------------" << endl;
}


void frame_hist(AVFrame *pframe, int hist[], int N){
	int shiftby = 8 - log2(N);
	memset((void*)hist, 0, N*sizeof(int));
	uint8_t* p = pframe->data[0];
	for (int i=0;i<pframe->height;i++){
		for (int j=0;j<pframe->width;j++){
			int pixel = p[j];
			int hindex = pixel >> shiftby;
			hist[hindex] += 1;
		}
		p += pframe->linesize[0];
	}
}

double hist_diff(int hist[], int prevhist[], int N){
	double acc = 0;
	double sum = 0;
	for (int i=0;i<N;i++){
		sum += hist[i];
		acc = acc + fabs(hist[i] - prevhist[i]);
	}
	return acc/sum;
}

static int _framehash(void *iplimg, float rho, uint64_t *hash, int blk_idx=1) {
	if (!iplimg || !hash) return -1;

	*hash = 0;
	IplImage *img = (IplImage*)iplimg;
	IplImage *imgplane   = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);
	IplImage *imgblurred = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);
	IplImage *imgresized = cvCreateImage(cvSize(32,32), img->depth, 1);
	IplImage *imgprime   = cvCreateImage(cvSize(32,32), IPL_DEPTH_32F, 1);
	IplImage *dctimg     = cvCreateImage(cvSize(32,32), IPL_DEPTH_32F, 1);

	cvSplit(img, imgplane, NULL, NULL, NULL);
	cvSmooth(imgplane, imgblurred, CV_GAUSSIAN, 3, 0, rho, rho);
	cvResize(imgblurred, imgresized, CV_INTER_AREA);

	cvConvertScale(imgresized, imgprime, 1, 0);
	cvDCT(imgprime, dctimg, 0);

	double minval, maxval, medianval;
	cvSetImageROI(dctimg, cvRect(blk_idx,blk_idx,blk_idx+8,blk_idx+8));
	cvMinMaxLoc(dctimg, &minval, &maxval, NULL, NULL, NULL);
	medianval = (maxval+minval)/2;
	
	uint64_t hashval = 0;
	for (int i=1;i<=8;i++){
	    const float *row = (const float*)(dctimg->imageData + i*dctimg->widthStep);
	    for (int j=1;j<=8;j++, hashval <<= 1){
			if (row[j] > medianval){
				hashval |= 1;
			}
	    }
	}
	*hash = hashval;
				 
	cvReleaseImage(&imgplane);
	cvReleaseImage(&imgblurred);
	cvReleaseImage(&imgresized);
	cvReleaseImage(&imgprime);
	cvReleaseImage(&dctimg);
	return 0;
}

int framehash(void *data, int width, int height, int widthstep, float rho, uint64_t *hash , int blk_idx){
	if (data == NULL || hash == NULL) return -1;
	IplImage *img = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 1);
	if (img == NULL) return -1;
	cvSetData(img, data, widthstep);
	int rc = _framehash(img, rho, hash);
	cvReleaseImageHeader(&img);
	return rc;
}


