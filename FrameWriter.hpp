#ifndef _FRAMEWRITER_H
#define _FRAMEWRITER_H

#include <string>
#include <stdint.h>
#include <stdexcept>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
};

using namespace std;

class FrameWriter {
protected:
    int width, height, pixfmt, bitrate;
    float framerate;
    AVFormatContext *oc;
    AVCodecContext *c;
    AVCodec *pCodec;
    AVStream *videoStream;
    uint8_t *outbuf;
    int outbufSize;
    double video_pts;
    uint64_t pts;

    void OpenVideo();
    AVStream* AddVideoStream(int width, int height, int pixfmt, AVCodecID codec_id, int bitrate);

public:
    static void SaveFrame(const string &filename, AVFrame *pframe, int width, int height, int pixfmt);

    FrameWriter(const string &filename, int width, int height, int pixfmt, float framerate, int bitrate);

    void WriteVideoFrame(AVFrame *pframe);

    ~FrameWriter();
};

class FrameWriterException : public runtime_error {
public:
    FrameWriterException(const string &str):runtime_error(str){};
};

#endif //_FRAMEWRITER_H
