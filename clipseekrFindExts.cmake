include(ExternalProject)

set(HIREDIS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hiredis)
set(HIREDIS_INCLUDE_DIRS ${HIREDIS_DIR}/include)
ExternalProject_Add(hiredis
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/hiredis
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hiredis
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND echo configure
  BUILD_COMMAND make static
  INSTALL_COMMAND  echo install)

if (DL_COPY_GIFLIB)
  set(GIFLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/giflib CACHE STRING "giflib location" FORCE)
  set(giflib ${CMAKE_CURRENT_SOURCE_DIR}/giflib/lib/libgif.a)

  ExternalProject_add(gif-lib
	URL http://iweb.dl.sourceforge.net/project/giflib/giflib-5.1.4.tar.bz2
	PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/giflib
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/giflib/src/gif-lib/configure
	--prefix=${CMAKE_CURRENT_SOURCE_DIR}/giflib --with-pic --enable-static)
else()
  if (GIFLIB_DIR)
	find_library(giflib libgif.a PATHS ${GIFLIB_DIR} NO_DEFAULT_PATH)
  else()
	find_library(giflib libgif.a)
  endif()
  if (${giflib} STREQUAL giflib-NOTFOUND)
	message(FATAL_ERROR "no giflib found")
  endif()
endif()

message(STATUS "Found giflib at ${giflib}")

if (DL_COPY_BOOST)
  set(BOOST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boost CACHE STRING "boost install dir" FORCE)
  set(Boost_LIB_DIR ${BOOST_DIR}/lib)
  set(Boost_PROGRAM_OPTIONS_LIBRARY ${BOOST_DIR}/lib/libboost_program_options.a)
  set(Boost_FILESYSTEM_LIBRARY      ${BOOST_DIR}/lib/libboost_filesystem.a)
  set(Boost_SYSTEM_LIBRARY          ${BOOST_DIR}/lib/libboost_system.a)
  set(Boost_INCLUDE_DIRS            ${BOOST_DIR}/include)
  set(Boost_VERSION 1.62.0)
  ExternalProject_Add(boost-libs
	URL https://sourceforge.net/projects/boost/files/boost/1.62.0/boost_1_62_0.tar.gz/download
	PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/boost
	CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/boost/src/boost-libs/bootstrap.sh --prefix=${CMAKE_CURRENT_SOURCE_DIR}/boost --with-libraries=program_options,filesystem,system
	BUILD_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/boost/src/boost-libs/b2 variant=release link=static threading=single runtime-link=shared install
	BUILD_IN_SOURCE 1
	INSTALL_COMMAND echo skip install
	)
else()
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED OFF)
  set(Boost_USE_STATIC_RUNTIME OFF)
  set(BOOST_ROOT ${BOOST_DIR})
  set(Boost_NO_SYSTEM_PATHS ON)
  if (BOOST_DIR)
	find_package(Boost 1.62.0 REQUIRED COMPONENTS program_options filesystem system PATHS ${BOOST_DIR} NO_DEFAULT_PATH)
  else()
	set(Boost_NO_SYSTEM_PATHS OFF)
	find_package(Boost 1.62.0 REQUIRED COMPONENTS program_options filesystem system)
  endif()
  if (NOT Boost_FOUND)
	message(FATAL_ERROR "No Boost 1.62.0 package found")
  endif()
endif()

message(STATUS "Found Boost at ${Boost_LIB_DIR}")


if (DL_COPY_FFMPEG)
  set(FFMPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg CACHE STRING "ffmpeg location" FORCE)
  set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
  set(avformatlib avformat)
  set(avcodeclib avcodec)
  set(avutillib avutil)
  set(avfilterlib avfilter)

  ExternalProject_add(ffmpeg-lib
    URL http://ffmpeg.org/releases/ffmpeg-2.8.tar.bz2
	PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg
	BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/src/ffmpeg-lib/configure
	--prefix=${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg 
	--enable-shared
	--enable-pic
	--disable-programs
	--disable-sdl
	--disable-doc
	--disable-encoders
	)
else()
  if (FFMPEG_DIR)
	set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
	find_library(avformatlib avformat PATHS ${FFMPEG_DIR}/lib NO_DEFAULT_PATH)
	find_library(avcodeclib avcodec PATHS ${FFMPEG_DIR}/lib NO_DEFAULT_PATH)
	find_library(avutillib avutil PATHS ${FFMPEG_DIR}/lib NO_DEFAULT_PATH)
	find_library(avfilterlib avfilter PATHS ${FFMPEG_DIR}/lib NO_DEFAULT_PATH)
  endif()
  find_library(avformatlib avformat)
  find_library(avutillib avutil)
  find_library(avcodeclib avcodec)
  find_library(avfilterlib avfilter)
  if (avutililib-NOTFOUND)
	message(FATAL_ERROR "no ffmpeg libav* libraries found.")
  endif()
endif()

message(STATUS "ffmpeg libav* location : ${avformatlib}")
message(STATUS "                         ${avcodeclib}")
message(STATUS "                         ${avutillib}")
message(STATUS "                         ${avfilterlib}")

if (DL_COPY_OPENCV)
  set(OPENCV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/opencv CACHE STRING "opencv location" FORCE)
  set(OpenCV_INCLUDE_DIRS ${OPENCV_DIR}/include)
  set(OpenCV_INSTALL_PATH ${OPENCV_DIR})
  set(OpenCV_MAJOR_VERSION 3)
  set(OpenCV_MINOR_VERSION 1)
  set(OpenCV_PATCH_VERSION 0)
  set(OpenCV_LIB_DIR ${OpenCV_INSTALL_PATH}/lib)
  set(OpenCV_3RDPARTY_LIB_DIR ${OpenCV_INSTALL_PATH}/share/OpenCV/3rdparty/lib)
  set(OpenCV_LIBS opencv_imgproc opencv_highgui opencv_imgcodecs opencv_video opencv_videoio opencv_core)
  ExternalProject_add(opencv-lib
	URL https://github.com/Itseez/opencv/archive/3.1.0.zip
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/opencv
	BUILD_IN_SOURCE 1
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/opencv
	-DCMAKE_POSITION_INDEPENDENT_CODE=1
	-DBUILD_DOCS=0 -DBUILD_opencv_python2=0 
	-DWITH_JAVA=0 -DBUILD_PERF_TESTS=0
	-DBUILD_SHARED_LIBS=1 -DBUILD_TESTS=0 -DBUILD_TIFF=1 
	-DBUILD_ZLIB=1 -DWITH_1394=0 
	-DWITH_CUFFT=0 -DWITH_EIGEN=0 -DWITH_GIGEAPI=0 
	-DWITH_GPHOTO2=0 -DWITH_VTK=0
	-DWITH_GTK=1 -DWITH_GSTREAMER=0 -DWITH_IPP=1
	-DWITH_JASPER=0 -DWITH_LIBV4L=0
	-DWITH_OPENCLAMDBLAS=0 -DWITH_OPENCLAMDFFT=0 
	-DWITH_OPENEXR=0 -DWITH_PVAPI=0
	-DWITH_V4L=0 -DWITH_WEBP=0 -DBUILD_opencv_java=0 
	-DWITH_FFMPEG=0 -DBUILD_JPEG=1 -DBUILD_PNG=1
	-DBUILD_opencv_flann=0 -DBUILD_opencv_apps=0 
	-DBUILD_opencv_calib3d=0 -DBUILD_opencv_features2d=0
	-DBUILD_opencv_ml=0 -DBUILD_opencv_objdetect=0 
	-DBUILD_opencv_photo=0 -DBUILD_opencv_shape=0
	-DBUILD_opencv_stitching=0 -DBUILD_opencv_superres=0 
	-DBUILD_opencv_ts=0
	-DBUILD_opencv_videostab=0 
	)
else()
  set(OpenCV_SHARED ON CACHE BOOL "use opencv shared lib" FORCE)
  if (OPENCV_DIR)
	find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs video videoio highgui
	  PATHS ${OPENCV_DIR} NO_DEFAULT_PATH)
  else()
	find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs video videoio highgui)
  endif()
  if (NOT OpenCV_FOUND)
	message(FATAL_ERROR "no OpenCV package found. Get OpenCV verion 3.1.0")
  endif()
  set(OPENCV_DIR ${OpenCV_INSTALL_PATH} CACHE STRING "opencv location" FORCE)
  set(OpenCV_LIB_DIR ${OpenCV_INSTALL_PATH}/lib)
  set(OpenCV_3RDPARTY_LIB_DIR ${OpenCV_INSTALL_PATH}/share/OpenCV/3rdparty/lib)
endif()

message(STATUS "Found OpenCV at ${OpenCV_INSTALL_PATH}")
message(STATUS "    libs at ${OpenCV_LIB_DIR}")
message(STATUS "    3rd party libs at ${OpenCV_3RDPARTY_LIB_DIR}")

if (DL_COPY_VCLIB)
  set(CMAKE_VCLIB_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vclib -DWITH_ASNDLIB=OFF)
  if (FFMPEG_DIR)
	set(CMAKE_VCLIB_ARGS ${CMAKE_VCLIB_ARGS} -DFFMPEG_DIR=${FFMPEG_DIR})
  endif()
  if (OPENCV_DIR)
	set(CMAKE_VCLIB_ARGS ${CMAKE_VCLIB_ARGS} -DOPENCV_DIR=${OPENCV_DIR})
  endif()
  message(STATUS "configure vclib with cmake args: ${CMAKE_VCLIB_ARGS}")
  
  set(VCLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vclib CACHE STRING "phvideocapture lib location" FORCE)
  set(phvideocapturelib ${CMAKE_CURRENT_SOURCE_DIR}/vclib/lib/libphvideocapture-static.a)
  ExternalProject_Add(vclib
	PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/vclib
	SVN_REPOSITORY http://svn.aetilius.com/aetilius/phvideocapture/trunk
	SVN_USERNAME dstarkweather
	SVN_PASSWORD sd88g93
	SVN_TRUST_CERT 1
	CMAKE_ARGS ${CMAKE_VCLIB_ARGS})

else()

  if (VCLIB_DIR)
	find_library(phvideocapturelib libphvideocapture-static.a PATHS ${VCLIB_DIR}/lib NO_DEFAULT_PATH)
  else()
	find_library(phvideocapturelib libphvideocapture-static.a)
  endif()
  if (phvideocapturelib-NOTFOUND)
	message(FATAL_ERROR "no phvideocapture lib found.")
  endif()
endif()

if (DL_COPY_FFMPEG)
  add_dependencies(vclib ffmpeg-lib)
endif()
if (DL_COPY_OPENCV)
  add_dependencies(vclib opencv-lib)
endif()

message(STATUS "phvideocapture lib ${phvideocapturelib}")

if (DL_COPY_PHASHPRO)
  set(CMAKE_PHASHPRO_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/phashpro
	-DWITH_IMAGE_HASH=ON -DWITH_AUDIO_HASH=OFF -DWITH_VIDEO_HASH=OFF -DWITH_JAVA=OFF)

  if (FFMPEG_DIR)
	set(CMAKE_PHASHPRO_ARGS ${CMAKE_PHASHPRO_ARGS} -DFFMPEG_DIR=${FFMPEG_DIR})
  endif()
  if (OPENCV_DIR)
	set(CMAKE_PHASHPRO_ARGS ${CMAKE_PHASHPRO_ARGS} -DOPENCV_DIR=${OPENCV_DIR})
  endif()
  if (VCLIB_DIR)
	set(CMAKE_PHASHPRO_ARGS ${CMAKE_PHASHPRO_ARGS} -DVCLIB_DIR=${VCLIB_DIR})
  endif()
  set(PHASHPRO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/phashpro CACHE STRING "phashpro install" FORCE)
  set(phashprolib ${CMAKE_CURRENT_SOURCE_DIR}/phashpro/lib/libpHashPro-static.a)

  ExternalProject_add(phashpro
	PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/phashpro
	BUILD_IN_SOURCE 1
	SVN_REPOSITORY http://svn.aetilius.com/aetilius/phashpro/trunk
	SVN_USERNAME dstarkweather
	SVN_PASSWORD sd88g93
	SVN_TRUST_CERT 1
	CMAKE_ARGS ${CMAKE_PHASHPRO_ARGS}
	)

else()
  if (PHASHPRO_DIR)
	find_library(phashprolib libpHashPro-static.a PATHS ${PHASHPRO_DIR}/lib NO_DEFAULT_PATH)
  else()
	find_library(phashprolib libpHashPro-static.a)
  endif()
  if (phashprolib-NOTFOUND)
	message(FATAL_ERROR "no libpHashPro found.")
  endif()
endif()


if (DL_COPY_GIFLIB)
  add_dependencies(phashpro gif-lib)
endif()
if (DL_COPY_OPENCV)
  add_dependencies(phashpro opencv-lib)
endif()
if (DL_COPY_VCLIB)
  add_dependencies(phashpro vclib)
endif()
if (DL_COPY_FFMPEG)
  add_dependencies(phashpro ffmpeg-lib)
endif()

message(STATUS "phashpro lib found at ${phashprolib}")
