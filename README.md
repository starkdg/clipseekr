# ClipSeekr

Clipseekr manages a reverse index of video sequences for the purpose of
sequence detection. It encompasses two programs: (1) clipster, for the 
purpose of indexing a set of short video clips, and (2) clipseekr, a program
to monitor a given stream for the occurence of previously indexed clips.

## Install

cmake .
make all
sudo make install

Set installation locations of ffmpeg and opencv by setting cmake variables
FFMPEG_DIR and OPENCV_DIR. You can do this with `cmake-gui .`

Make sure opencv is compiled with gtk2.0 support.

## Usage

Activate redis-server:

    `redis-server /etc/redis/redis_6379.conf`
	
or alternatively, you can run the init script:
		
    `sudo /etc/init.d/redis_6379 start`
	
	
Learn more about setting up redis server: [redis](https://redis.io/topics/quickstart)	

Run clipster program to index movie clips. See options with 

	`./clipster --help`

Run clipseekr program to monitor a stream.  Like so: 

	`./clipseekr --mode monitor -i <video_file.mpg>`
	
Use the -P (or --preview=1) flag for a video display.
	
The file name can be replaced with a network stream url or a device node (e.g. /dev/video0)
for real time monitoring.
	

See all options for above programs with `./clipster --help` and `./clipseekr --help`


## Dependencies

Be sure to compile OpenCV with gtk2.0 support!

[OpenCV 3.4.6](https://github.com/opencv/opencv/tree/3.4.6)
[ffmpeg 2.6.9](https://ffmpeg.org/releases/ffmpeg-2.6.9.tar.xz)

These will be auto-downloaded by cmake script:

[Boost 1.62.0](URL https://sourceforge.net/projects/boost/files/boost/1.62.0/boost_1_62_0.tar.gz)
[phvideocapture](https://github.com/starkdg/phvideocapture)
