#ifndef _CAPFRAMES_H
#define _CAPFRAMES_H

#include <stdint.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <opencv2/core/core_c.h>
};

struct CapFrames {
    AVFrame **pConvertedFrames;
    IplImage **imgs;   //buffer of images, nbframes length 
    int64_t ts;        //current time relative to fps
    float fps; 
    int position;      //position in buffer marking current frame
    int nbframes;      //nb frames in buffer 
    int pixfmt;
 };

#endif //_CAPFRAMES_H
