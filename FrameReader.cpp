#include "FrameReader.hpp"
#include <iostream>
using namespace std;


FrameReader::FrameReader(const string &filename, int pixfmt){                      
    this->pixfmt = pixfmt;
    this->oc = 0;
    this->c  = 0;
    this->s  = 0;

//    av_log_set_level(AV_LOG_QUIET);
    avcodec_register_all();
    av_register_all();

    if (avformat_open_input(&oc, filename.c_str(), 0, 0) < 0)
	throw FrameReaderException("unable to open file");

    if (avformat_find_stream_info(oc, 0) < 0)
	throw FrameReaderException("no stream info found");

	AVCodec *dec;
	videoStream = av_find_best_stream(oc, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (videoStream == -1)
	throw FrameReaderException("no video stream found");

    c = oc->streams[videoStream]->codec;
    if (c == NULL)
	throw FrameReaderException("no codec context");

    if (avcodec_open2(c, dec, 0) < 0)
	throw FrameReaderException("no codec opened");

    pframe = av_frame_alloc();
    if (pframe == NULL)
	throw FrameReaderException("unable to allocate frame");

    pframe2 = av_frame_alloc();
    if (pframe2 == NULL)
	throw FrameReaderException("unable to allocate frame2");

    if (av_image_alloc(pframe2->data, pframe2->linesize, c->width, c->height, (PixelFormat)pixfmt, 8) < 0)
	throw FrameReaderException("unable to allocate image");

    currentframe = 0;
    do {
	if (av_seek_frame(oc, videoStream, currentframe, AVSEEK_FLAG_ANY | AVSEEK_FLAG_FRAME) < 0){
	    break;
	}
	currentframe++;
    } while (true);
    totalframes = currentframe;

    s = sws_getContext(c->width, c->height, c->pix_fmt,
                       pframe2->linesize[0], c->height, (PixelFormat)pixfmt,
		       SWS_AREA, 0, 0, 0);
    if (s == NULL)
	throw FrameReaderException("no sws context");
}

AVFrame* FrameReader::GetFrame(int64_t index){
    int rc, done = 0;
    AVPacket pkt;
    if (index < 0 || index > totalframes - 1)
	return NULL;
    int delta = index - currentframe;
    int flags = AVSEEK_FLAG_ANY | AVSEEK_FLAG_FRAME;
    if (delta < 0) flags |= AVSEEK_FLAG_BACKWARD;

    if (av_seek_frame(oc, videoStream, index, flags) < 0)
	throw FrameReaderException("unable to seek in frame");

    currentframe += delta;
    do {
		if ((rc = av_read_frame(oc, &pkt)) < 0)
			throw FrameReaderException("unable to read frame:" + rc);
		if (pkt.stream_index == videoStream){
			avcodec_decode_video2(c, pframe, &done, &pkt);
		}
		currentframe++;
		av_free_packet(&pkt);
    } while (done == 0);
	
    sws_scale(s, pframe->data, pframe->linesize, 0, c->height, pframe2->data, pframe2->linesize);

    return pframe2;
}

void FrameReader::GetSize(int &width, int &height){
    width = c->width;
    height = c->height;
}

void FrameReader::GetNumberFrames(int64_t &nbframes){
    nbframes = totalframes;
}

FrameReader::~FrameReader(){
    av_frame_free(&pframe);
    av_free(pframe2->data[0]);
    av_frame_free(&pframe2);
    sws_freeContext(s);
    avcodec_close(c);
    avformat_close_input(&oc);
}
