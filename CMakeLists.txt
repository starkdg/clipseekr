cmake_minimum_required(VERSION 3.0)

project(ClipSeekr)
set(ClipSeekr_MAJOR_VERSION 0)
set(ClipSeekr_MINOR_VERSION 1)
set(ClipSeekr_PATCH_VERSION 0)
include(ExternalProject)

configure_file (
  "${PROJECT_SOURCE_DIR}/ClipSeekrConfig.h.in"
  "${PROJECT_BINARY_DIR}/ClipSeekrConfig.h"
)

# use following vars to indicate location of libraries
set(OPENCV_DIR "" CACHE STRING "opencv install dir to use")
set(FFMPEG_DIR "" CACHE STRING "ffmpeg lib install dir to use")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(clipseekrFindExts)

set(CLIPLIB_SRCS RedisHashStore.cpp clipseekrutils.cpp)
add_library(clipseekr-utils STATIC ${CLIPLIB_SRCS})
add_dependencies(clipseekr-utils hiredis boost-libs)

set_property(TARGET clipseekr-utils APPEND PROPERTY INCLUDE_DIRECTORIES ${HIREDIS_INCLUDE_DIRS})
set_property(TARGET clipseekr-utils APPEND PROPERTY INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
set_property(TARGET clipseekr-utils APPEND PROPERTY INCLUDE_DIRECTORIES ${OpenCV_INCLUDE_DIRS})
set_property(TARGET clipseekr-utils APPEND PROPERTY COMPILE_FLAGS "-g -Wall -std=c++11")
set_property(TARGET clipseekr-utils APPEND PROPERTY COMPILE_DEFINITIONS __STDC_CONSTANT_MACROS)
set_property(TARGET clipseekr-utils APPEND PROPERTY COMPILE_DEFINITIONS USE_CV_HIGHGUI=1)
set_property(TARGET clipseekr-utils PROPERTY   VERSION "0.2")
set_property(TARGET clipseekr-utils PROPERTY SOVERSION "0.2")
set_property(TARGET clipseekr-utils PROPERTY PRIVATE_HEADER  HashStore.hpp TableEntry.hpp RedisHashStore.hpp) 

target_link_libraries (clipseekr-utils ${avutillib} ${CMAKE_CURRENT_SOURCE_DIR}/hiredis/libhiredis.a ${OpenCV_LIBS})


add_executable(clipster clipster_main.cpp)
set_property(TARGET clipster APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/hiredis)
set_property(TARGET clipster APPEND PROPERTY INCLUDE_DIRECTORIES ${VCLIB_DIR}/include)
set_property(TARGET clipster APPEND PROPERTY INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
set_property(TARGET clipster APPEND PROPERTY INCLUDE_DIRECTORIES ${PHASHPRO_DIR}/include)
set_property(TARGET clipster APPEND PROPERTY INCLUDE_DIRECTORIES ${OpenCV_INCLUDE_DIRS})
set_property(TARGET clipster APPEND PROPERTY INCLUDE_DIRECTORIES ${FFMPEG_DIR}/include)
set_property(TARGET clipster PROPERTY COMPILE_FLAGS "-g -std=c++11 -Wall")
set_property(TARGET clipster APPEND PROPERTY COMPILE_DEFINITIONS CLIPSEEKR_VERSION="${ClipSeekr_MAJOR_VERSION}.${ClipSeekr_MINOR_VERSION}.${ClipSeekr_PATCH_VERSION}")


add_executable(clipseekr clipseekr_main.cpp)
set_property(TARGET clipseekr PROPERTY COMPILE_FLAGS "-g -Wall -std=c++11")
set_property(TARGET clipseekr APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/hiredis)
set_property(TARGET clipseekr APPEND PROPERTY INCLUDE_DIRECTORIES ${VCLIB_DIR}/include)
set_property(TARGET clipseekr APPEND PROPERTY INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
set_property(TARGET clipseekr APPEND PROPERTY INCLUDE_DIRECTORIES ${PHASHPRO_DIR}/include)
set_property(TARGET clipseekr APPEND PROPERTY INCLUDE_DIRECTORIES ${OpenCV_INCLUDE_DIRS})
set_property(TARGET clipseekr APPEND PROPERTY INCLUDE_DIRECTORIES ${FFMPEG_DIR}/include)
set_property(TARGET clipseekr APPEND PROPERTY COMPILE_DEFINITIONS CLIPSEEKR_VERSION="${ClipSeekr_MAJOR_VERSION}.${ClipSeekr_MINOR_VERSION}.${clipSeekr_PATCH_VERSION}")

add_dependencies(clipster boost-libs vclib)
add_dependencies(clipseekr boost-libs vclib)


target_link_libraries(clipster clipseekr-utils pthread dl rt
  ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY}
  ${phvideocapturelib}
  ${phashprolib}
  ${OpenCV_LIBS}
  ${giflib}
  ${avformatlib} ${avcodeclib} ${avutillib} ${avfilterlib})


target_link_libraries(clipseekr clipseekr-utils pthread dl rt
  ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY}
  ${phvideocapturelib} ${phashprolib} ${giflib}
  ${OpenCV_LIBS}
  ${avformatlib} ${avcodeclib} ${avutillib} ${avfilterlib})


#test program
add_executable(testredis TestRedisHashStore.cpp)
set_property(TARGET testredis PROPERTY COMPILE_FLAGS "-Wall -std=c++11")
set_property(TARGET testredis APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/hiredis)
target_link_libraries(testredis clipseekr-utils)

#test rules
enable_testing()
add_test(testhashstore testredis)


#install rules
install (TARGETS clipseekr RUNTIME DESTINATION bin)
install (TARGETS clipster RUNTIME DESTINATION bin)

#build cpack installer package
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "ClipSeekr")
set(CPACK_VENDOR "Aetilius")
set(CPACK_GENERATOR "DEB")
set (CPACK_RESOURCE_FILE_LICENSE
        "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${ClipSeekr_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${ClipSeekr_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${ClipSeekr_VERSION_PATCH}")
set (CPACK_PACKAGE_CONTACT "starkd88@gmail.com")
include (CPack)
