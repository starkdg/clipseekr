#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>

#include <ncurses.h>
#include "FrameWriter.hpp"
#include "FrameReader.hpp"

extern "C" {
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
};

#define KEY_SPACE 32
#define KEY_ESC   27

using namespace std;

void InitCursors(){
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    /* init colors pairs */
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
}

void CloseCursors(){
    endwin();
}

void WriteTitle(){
    int i, nrows, ncols;
    getmaxyx(stdscr, nrows, ncols);

    move(nrows/8-1, ncols/2-10);
    for (i=0;i<18;i++) addch(ACS_DIAMOND | A_BOLD | A_ALTCHARSET);

    move(nrows/8, ncols/2-10);
    addch(ACS_DIAMOND | A_BOLD | A_ALTCHARSET);
    printw("  ClipReader v0.1 ");
    printw("  by Aetilius, Inc. 2012 ");
    addch(ACS_DIAMOND | A_BOLD | A_ALTCHARSET);

    move(nrows/8+1, ncols/2-10);
    for (i=0;i<18;i++) addch(ACS_DIAMOND | A_BOLD | A_ALTCHARSET);

    refresh();
    move(nrows-1, ncols-1);
}

void WriteInfo(int64_t current, int64_t start, int64_t step, int64_t end, int64_t total){
    int i, nrows, ncols;
    getmaxyx(stdscr, nrows, ncols);

    move(2*nrows/8, 0);
    for (i=0;i<ncols;i++) addch(ACS_HLINE | A_BOLD | A_ALTCHARSET);
    mvprintw(2*nrows/8+1 , 5           , "CURRENT: %5lld", current); 
    mvprintw(2*nrows/8+2, 5           , "START:   %5lld" , start);
    mvprintw(2*nrows/8+3, 5           , "STEP:    %5lld" , step);
    mvprintw(2*nrows/8+4, 5           , "END:     %5lld" , end);
    mvprintw(2*nrows/8+5, 5           , "TOTAL:   %5lld" , total);
    move(2*nrows/8+6, 0);
    for (i=0;i<ncols;i++) addch(ACS_HLINE | A_BOLD | A_ALTCHARSET);

    move(nrows-1, ncols-1);
}

void WriteInstructions(){
    int i, nrows, ncols;
    getmaxyx(stdscr, nrows, ncols);

    move(3*nrows/4, 0);
    for (i=0;i<ncols;i++) addch(ACS_HLINE | A_BOLD | A_ALTCHARSET);
   
    /* left column */
    attron(A_BOLD);
    mvprintw(3*nrows/4+1, 5, "ESC   - quit");
    mvprintw(3*nrows/4+2, 5, "q     - quit");
    mvprintw(3*nrows/4+3, 5, "F2    - save");
    mvprintw(3*nrows/4+4, 5, "s     - mark start");
    mvprintw(3*nrows/4+5, 5, "e     - mark end");
    attron(A_BOLD);

    /* right column */
    mvaddch(3*nrows/4+1, ncols/2+5, ACS_LARROW | A_BOLD | A_ALTCHARSET);
    printw(" - previous frame");
    mvaddch(3*nrows/4+2, ncols/2+5, ACS_RARROW | A_BOLD | A_ALTCHARSET);
    printw(" - next frame");

    move(nrows-1, 0);
    for (i=0;i<ncols;i++) addch(ACS_HLINE | A_BOLD | A_ALTCHARSET);

    refresh();
}

void GetDateStamp(string &str){
    stringstream ss(stringstream::in | stringstream::out);

    time_t current;
    struct tm local;
    time(&current);
    localtime_r(&current, &local);

    ss << "D" << local.tm_mon << "-" << local.tm_mday << "_T" << local.tm_hour << ":" 
                                         << local.tm_min << ":" << local.tm_sec ;

    str = ss.str();
}

void GetName(string &nameStr){
    const char *msg = "Enter description:";
    int nrows, ncols;
    getmaxyx(stdscr, nrows, ncols);

    attron(A_BOLD);
    mvprintw(5*nrows/8+1, 10,"                                                      ");
    mvprintw(5*nrows/8+1, 10, msg);
    mvprintw(5*nrows/8+2, 10, "                                                     ");
    move(5*nrows/8+1, 10+strlen(msg));
    attron(A_BOLD);
    refresh();

    curs_set(2); /* make cursor visible */

    string dateString;
    GetDateStamp(dateString);

    stringstream ss(stringstream::in | stringstream::out);
    int ch, i = 0;
    do {
	ch = getch();
	if (ch == KEY_BACKSPACE && i > 0){
	    int x,y;
	    getyx(stdscr, y,x);
	    mvaddch(y,x-1,' ');
	    move(y,x-1);
	    i--;
	} else if (ch != '\n' && ch != KEY_BACKSPACE){
	    i++;
	    ss << (char)ch;
	    addch(ch | A_BOLD);
	} 
    } while (ch != '\n' && i < 64);

    //ss << dateString;
    nameStr = ss.str();

    curs_set(0); /* make cursor invisible */
}

void WriteDone(string name){
    int nrows, ncols;
    getmaxyx(stdscr, nrows, ncols);
    mvprintw(5*nrows/8+3, 10, "  saved: %s                                       ", name.c_str());
    curs_set(0);
}

void WriteFail(string name){
    int nrows, ncols;
    getmaxyx(stdscr, nrows, ncols);
    mvprintw(5*nrows/8+3, 10, "  not saved: %s                                   ", name.c_str());
    curs_set(0);
}

bool WriteFrames(FrameReader *pReader,string &name, int width, int height, int pixfmt, float fr, int br,
		 int64_t start, int64_t end){
    bool rc = true;
    FrameWriter *pWriter = new FrameWriter(name, width, height, pixfmt, fr, br);
    if (pWriter == NULL) return false;

    for (int64_t index=start;index<=end;index++){
	AVFrame *pframe = pReader->GetFrame(index);
	if (pframe == NULL) {
	    rc = false;
	    break;
	}
	pframe->pts = index;
	pWriter->WriteVideoFrame(pframe);
    }
    delete pWriter;
    return rc;
}

void MainLoop(const string &filename){
    int nrows, ncols;
    getmaxyx(stdscr, nrows, ncols);

    int ch;
    int pixfmt = PIX_FMT_YUVJ420P;
    float fr = 29.97f;
    int br = 200000;
    int64_t current = 0, i, start = 0, step = 15, end = 0, total = 0;
    string outfilename;

    FrameReader *pReader;
    try {
	pReader = new FrameReader(filename, pixfmt);
    } catch (FrameReaderException ex){
	cout << "no such file:" << ex.what() << endl;
	return;
    }

    int width, height;
    pReader->GetSize(width, height);

    int64_t totalframes;
    pReader->GetNumberFrames(totalframes);
    total = totalframes;
    end   = total - 1;

    IplImage *img = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 1);
    AVFrame *pframe = pReader->GetFrame(current);
    cvSetData(img, pframe->data[0], pframe->linesize[0]);

    cvShowImage("ClipReader", img);
    cvWaitKey(10);

    curs_set(0);
    do {
	ch = getch(); 
	switch (ch){
	case KEY_UP:
	    step = (step + 1 < total) ? step + 1 : step;
	    WriteInfo(current, start, step, end, total);
	    break;
	case KEY_DOWN:
	    step = (step - 1  > 1) ? step - 1 : 0;
	    WriteInfo(current, start, step, end, total);
	    break;
	case KEY_LEFT:
	    current = (current - step > 0) ? current - step : 0;
	    WriteInfo(current, start, step, end, total);
	    pframe = pReader->GetFrame(current);
	    cvShowImage("ClipReader", img);
	    cvWaitKey(10);
	    break;
	case KEY_RIGHT:
	    current = (current + step < total) ? current + step : current;
	    WriteInfo(current, start, step, end, total);
	    pframe = pReader->GetFrame(current);
	    cvShowImage("ClipReader", img);
	    cvWaitKey(10);
	    break;
	case 's':
	    start = current;
	    end = (end <= start) ? total : end;
	    WriteInfo(current, start, step, end, total);
	    break;
	case 'e':
	    end = current;
	    start = (start >= end) ? 0 : start;
	    WriteInfo(current, start, step, end, total);
	    break;
	case KEY_F(2):
	    start = (start >= end) ? 0: start;
	    end   = (end <= start) ? totalframes-1 : end;
	    GetName(outfilename);
	    if (WriteFrames(pReader, outfilename, width, height, pixfmt, fr, br, start, end)){
		WriteDone(outfilename);
	    } else {
		WriteFail(outfilename);
	    }
	    break;
	case 'q':
	    ch = KEY_ESC;
	}

    } while (ch != KEY_ESC);

    delete pReader;
    return;
}

int main(int argc, char **argv){
    if (argc < 2) {
	printf("not enough input args\n");
	exit(1);
    }
    const string filename = argv[1];
    cvNamedWindow("ClipReader", CV_WINDOW_AUTOSIZE);
    InitCursors();
    WriteTitle();
    WriteInfo(0, 0, 1, 0, 0);
    WriteInstructions();
    MainLoop(filename);
    CloseCursors();
    cvDestroyAllWindows();
    return 0;
}
