# ClipSeekr

[![xscode](https://img.shields.io/badge/Available%20on-xs%3Acode-blue?style=?style=plastic&logo=appveyor&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAMAAACdt4HsAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAAZQTFRF////////VXz1bAAAAAJ0Uk5T/wDltzBKAAAAlUlEQVR42uzXSwqAMAwE0Mn9L+3Ggtgkk35QwcnSJo9S+yGwM9DCooCbgn4YrJ4CIPUcQF7/XSBbx2TEz4sAZ2q1RAECBAiYBlCtvwN+KiYAlG7UDGj59MViT9hOwEqAhYCtAsUZvL6I6W8c2wcbd+LIWSCHSTeSAAECngN4xxIDSK9f4B9t377Wd7H5Nt7/Xz8eAgwAvesLRjYYPuUAAAAASUVORK5CYII=)](https://xscode.com/starkdg/clipseekr)

Clipseekr manages a reverse index of video sequences for the purpose of
sequence detection. It encompasses two programs: (1) clipster, for the 
purpose of indexing a set of short video clips, and (2) clipseekr, a program
to monitor a given stream for the occurence of previously indexed clips.

Read the blog post about it here: [blog](http://blog.phash.org/posts/ClipSeekr-VideoClipDetection)

## Install

```
cmake .
make all
sudo make install
```

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

[OpenCV 3.4.6](https://github.com/opencv/opencv/tree/3.4.6)\

[ffmpeg 2.6.9](https://ffmpeg.org/releases/ffmpeg-2.6.9.tar.xz)

These will be auto-downloaded by cmake script:

[Boost libs v1.62.0](URL https://sourceforge.net/projects/boost/files/boost/1.62.0/boost_1_62_0.tar.gz)\

[phvideocapture v0.0.1](https://github.com/starkdg/phvideocapture)

[hiredis v0.9.0](https://github.com/redis/hiredis/archive/v0.9.0.tar.gz)
