include(ExternalProject)

set(HIREDIS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hiredis)
set(HIREDIS_INCLUDE_DIRS ${HIREDIS_DIR})
ExternalProject_Add(hiredis
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/hiredis
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hiredis
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND echo configure
  BUILD_COMMAND make static
  INSTALL_COMMAND  echo install)


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

set(FFMPEG_DIR "/usr/local" CACHE STRING "ffmpeg location")
set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
find_library(avformatlib avformat PATHS ${FFMPEG_DIR}/lib)
find_library(avcodeclib avcodec PATHS ${FFMPEG_DIR}/lib)
find_library(avutillib avutil PATHS ${FFMPEG_DIR}/lib)
find_library(avfilterlib avfilter PATHS ${FFMPEG_DIR}/lib)
if (avutililib-NOTFOUND)
  message(FATAL_ERROR "no ffmpeg libav* libraries found.")
endif()

message(STATUS "ffmpeg libav* location : ${avformatlib}")
message(STATUS "                         ${avcodeclib}")
message(STATUS "                         ${avutillib}")
message(STATUS "                         ${avfilterlib}")

set(OPENCV_DIR "/usr/local" CACHE STRING "opencv location")
set(OpenCV_SHARED ON CACHE BOOL "use opencv shared lib" FORCE)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs video videoio highgui PATHS ${OPENCV_DIR})
set(OpenCV_LIB_DIR ${OpenCV_INSTALL_PATH}/lib)
set(OpenCV_3RDPARTY_LIB_DIR ${OpenCV_INSTALL_PATH}/share/OpenCV/3rdparty/lib)

message(STATUS "Found OpenCV at ${OpenCV_INSTALL_PATH}")
message(STATUS "    libs at ${OpenCV_LIB_DIR}")
message(STATUS "    3rd party libs at ${OpenCV_3RDPARTY_LIB_DIR}")

set(CMAKE_VCLIB_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vclib -DWITH_ASNDLIB=OFF)
set(CMAKE_VCLIB_ARGS ${CMAKE_VCLIB_ARGS} -DFFMPEG_DIR=${FFMPEG_DIR})
set(CMAKE_VCLIB_ARGS ${CMAKE_VCLIB_ARGS} -DOPENCV_DIR=${OPENCV_DIR})
message(STATUS "configure vclib with cmake args: ${CMAKE_VCLIB_ARGS}")
set(VCLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vclib CACHE STRING "phvideocapture lib location" FORCE)
set(phvideocapturelib ${CMAKE_CURRENT_SOURCE_DIR}/vclib/lib/libphvideocapture-static.a)

ExternalProject_Add(vclib
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/vclib
  GIT_REPOSITORY https://github.com/starkdg/phvideocapture.git
  HTTP_USERNAME
  HTTP_PASSWORD
  CMAKE_ARGS ${CMAKE_VCLIB_ARGS})

message(STATUS "phvideocapture lib ${phvideocapturelib}")

