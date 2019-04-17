#include "FrameWriter.hpp"
#include <ctime>
#include <sstream>

using namespace std;


void FrameWriter::SaveFrame(const string &filename, AVFrame *pframe, int width, int height, int pixfmt){

    FILE *pfile = fopen(filename.c_str(), "w");
    if (!pfile) 
	throw FrameWriterException("unable to open file");

    int dest_pixfmt = PIX_FMT_YUVJ420P;
    int bufsize = avpicture_get_size((PixelFormat)dest_pixfmt, width, height);
    uint8_t *buf = (uint8_t*)av_malloc(bufsize);
    if (!buf)
	throw FrameWriterException("unable to allocate encode buffer");
    memset(buf, 0, bufsize);

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
    if (!pCodecCtx){
	av_free(buf);
	throw FrameWriterException("unable to allocate codec context");
    }
    pCodecCtx->bit_rate = pCodecCtx->bit_rate;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->pix_fmt = (PixelFormat)dest_pixfmt;
    pCodecCtx->codec_id = CODEC_ID_MJPEG;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 10;

    AVCodec *pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec){
	av_free(buf);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	throw FrameWriterException("unable to find encoder");
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
	av_free(buf);
	avcodec_close(pCodecCtx);

	av_free(pCodecCtx);
	throw FrameWriterException("unable to open codec");
    }

    pCodecCtx->mb_lmin = pCodecCtx->lmin = pCodecCtx->qmin*FF_QP2LAMBDA;
    pCodecCtx->mb_lmax = pCodecCtx->lmax = pCodecCtx->qmax*FF_QP2LAMBDA;
    pCodecCtx->flags = CODEC_FLAG_QSCALE;
    pCodecCtx->global_quality = pCodecCtx->qmin*FF_QP2LAMBDA;
    pframe->pts = 1;
    pframe->quality = pCodecCtx->global_quality;
    int actual_bufsize = avcodec_encode_video(pCodecCtx, buf, bufsize, pframe);
    if (actual_bufsize < 0){
	av_free(buf);
	av_free(pCodecCtx);
        throw FrameWriterException("encode error");
    }

    fwrite(buf, 1, actual_bufsize, pfile);
    fclose(pfile);
    avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
    av_free(buf);
}


void FrameWriter::OpenVideo(){
   
    avcodec_open2(c, pCodec, NULL);
    outbuf = NULL;
    outbufSize = 200000;
    outbuf = (uint8_t*)av_malloc(outbufSize);
    if (outbuf == NULL)
	throw FrameWriterException("unable to allocate encode buffer");
}

AVStream* FrameWriter::AddVideoStream(int width, int height, int pixfmt, AVCodecID codec_id, int bitrate){
    AVStream *stream;

    stream = avformat_new_stream(oc, NULL);
    if (stream == NULL) throw FrameWriterException("unable to create stream");

    c = stream->codec;
    pCodec = avcodec_find_encoder(codec_id);
    if (pCodec == NULL) throw FrameWriterException("unable to find encoder");

    avcodec_get_context_defaults3(c, pCodec);
    c->codec_id = codec_id;
    c->bit_rate = bitrate;
    c->width = width;
    c->height = height;

    int fr = (int)(1000.0*framerate);
    c->time_base = (AVRational){ 1000, fr};
    c->gop_size = 12;
    c->pix_fmt = (PixelFormat)pixfmt;
    if (oc->oformat->flags & AVFMT_GLOBALHEADER){
	c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    return stream;
}

FrameWriter::FrameWriter(const string &filename, int width, int height, int pixfmt, float framerate, int bitrate){
    this->framerate = framerate;
    this->width = width;
    this->height = height;
    this->pixfmt = pixfmt;
    this->framerate = framerate;
    this->bitrate = bitrate;
    
    AVOutputFormat *fmt = NULL;
    avcodec_register_all();
    av_register_all();

    oc = NULL;
    avformat_alloc_output_context2(&oc, NULL, "avi", NULL);
    if (!oc){
	throw FrameWriterException("unable to alloc output format");
    }

    fmt = oc->oformat;
    fmt->video_codec = CODEC_ID_MJPEG;

    videoStream = AddVideoStream(width, height, pixfmt, fmt->video_codec, bitrate);
    if (videoStream == NULL){
	throw FrameWriterException("unable to AddVideoStream");
    }

    OpenVideo();

    stringstream ss;
    ss << filename << ".avi";
    if (avio_open(&oc->pb, ss.str().c_str(), AVIO_FLAG_WRITE) < 0){
	throw FrameWriterException("unable to open file for writing");
    }

    avformat_write_header(oc, 0);
    pts = 0;
}

void FrameWriter::WriteVideoFrame(AVFrame *pframe){

    int outSize = avcodec_encode_video(c, outbuf, outbufSize, pframe);

    if (outSize > 0){
	AVPacket pkt;
	av_init_packet(&pkt);
	pframe->pts = pts;
	pkt.pts = pts++;
	pkt.flags |= AV_PKT_FLAG_KEY;
	pkt.stream_index = videoStream->index;
	pkt.data = outbuf;
	pkt.size = outSize;

	av_interleaved_write_frame(oc, &pkt);
    }
}

FrameWriter::~FrameWriter(){
    av_write_trailer(oc);
    avcodec_close(c);
    av_free(outbuf);
    outbuf = NULL;
    avio_close(oc->pb);
    av_free(oc);
}
