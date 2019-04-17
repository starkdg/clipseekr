ClipSeekr v1.5

Background
----------------------------------------------------------------
ClipSeekr is a set of programs with the coordinated intent to detect
known video sequences (e.g. commercial spots) from a live video stream.  
These spots can be directly provided to the system through user cut
and edited video files.

New Features
----------------------------------------------------------------
   Redis support for hash storage. Must run at least one redis server.

1. A program (clipster) which is able to index video clips to the redis
   server.  (Video clips can be in any format, but should contain de-interlaced
   frames, since clipseekr applies a deinterlace filter when searching streams.)

2. Deinterlace filtering added via libavfilter for clipseekr.

3. Cropping ability added in through libavfilter.

4. code written in c++ 


Usage
----------------------------------------------------------------
The trial version has just the file interface and can index a limited
number of clips for which to search.  

Steps:

1. Use clipster program to index the clips into running redis db server.
   The directory is the directory in which the clips reside.
   
   e.g. ./clipster -d <directory>

2. Run clipseekr on unknown video file to scan for the clips.

   e.g. ./clipseekr --mode monitor -i <file>


Clipseekr-utils
---------------------------------------------------------------

A shared library that encompasses video capturing and  frame processing
capability.  Single VideoCapture interface.


FrameReader - class to read each frame of a video file.
			   (deprecated)
FrameWriter - class to write a series of frames to a motion jpeg video file.
              (deprecated)
			  
RedisHashStore - class to insert/lookup frame hashes into redis db.



ClipSeekr
---------------------------------------------------------------

A program to monitor a video stream.  It
operates in two modes: 'info' and 'monitor', either of which can be
specified in the first command line argument. Use the -i argument to
indicate the file or device node from which to stream video.

run ./clipseekr --help

Clipster
--------------------------------------------------------------

A program to process a directory of video files, ideally in mjpeg format
which stores the hashes in redis.

	./clipster -d <directory> 


Other Software
-----------------------------------------------------------------

libphvideocapture 1.0.0
libpHashPro 1.3.0

	

Trial Version dependencies
-----------------------------
	OpenCV 2.4.11
	ffmpeg 2.6.5
	libavformat 56.25.101
	libavcodec  56.26.100
	libavutil   54.20.100
	libavfilter 5.11.102
	libswscale  3.1.101
	
