#ifndef _FRAMEREADER_H
#define _FRAMEREADER_H

#include <string>
#include <stdexcept>
#include <vector>
#include <stdint.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

using namespace std;

class FrameReader {
protected:
	AVFormatContext *oc;
	AVCodecContext *c;
	struct SwsContext *s;
	AVFrame *pframe, *pframe2;
	int pixfmt;
	int64_t currentframe;
	int64_t totalframes;
	int videoStream;

public:

	FrameReader(const string &filename, int pixfmt);
	AVFrame* GetFrame(int64_t index);
	void GetSize(int &width, int &height);
	void GetNumberFrames(int64_t &nbframes);
	~FrameReader();

};

class FrameReaderException: public runtime_error {
public:
	FrameReaderException(const string &str) :
			runtime_error(str) {
	}
	;
};

#endif // _FRAMEREADER_H
