#ifndef CLIPSEEKR_UTILS_H
#define CLIPSEEKR_UTILS_H

#include <cstdint>
#include <VideoCapture.hpp>

using namespace std;


bool isblackframe(AVFrame *pframe, int pixel_threshold = 25, int black_threshhold = 95);

int64_t process_timestamp(int64_t ts, AVRational tb);

void process_timestamp(int64_t ts, AVRational tb, int &hrs, int &mins, int &secs);

void print_metadata(ph::MetaData &mdata);

void frame_hist(AVFrame *pframe, int hist[], int N);

double hist_diff(int hist[], int prevhist[], int N);


#endif /* CLIPSEEKR_UTILS_H */
