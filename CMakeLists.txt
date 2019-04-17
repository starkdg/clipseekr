cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0046 OLD) # policy for non-existing dependencies
cmake_policy(SET CMP0015 NEW) # link_directories command policy (paths interpreted as relative to current source dir)
cmake_policy(SET CMP0048 NEW) # set VERSION policy

project(ClipSeekr VERSION 1.6.0.1)
include(ExternalProject)

configure_file (
  "${PROJECT_SOURCE_DIR}/ClipSeekrConfig.h.in"
  "${PROJECT_BINARY_DIR}/ClipSeekrConfig.h"
)

set(CLIPSEEKR_TRIAL_VERSION OFF CACHE BOOL "build trial version")
if (CLIPSEEKR_TRIAL_VERSION)
  message(STATUS "creating trial ClipSeekr trial binaries")
  add_definitions(-DCLIPSEEKR_TRIAL_VERSION)
endif()

macro (TODAY RESULT)
    if (WIN32)
        execute_process(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" ${RESULT} ${${RESULT}})
    elseif(UNIX)
        execute_process(COMMAND "date" "+%s" OUTPUT_VARIABLE ${RESULT})
    else (WIN32)
        message(SEND_ERROR "date not implemented")
        set(${RESULT} 0)
    endif (WIN32)
endmacro (TODAY) 


if (DL_COPY_ALL)
set(DL_COPY_BOOST    ON CACHE BOOL "use dl copy of boost libs")
set(DL_COPY_OPENCV   ON CACHE BOOL "use new copy of opencv in build")
set(DL_COPY_VCLIB    ON CACHE BOOL "use new copy of phvideocapture lib in build")
set(DL_COPY_FFMPEG   ON CACHE BOOL "use new copy of ffmpeg in build") 
set(DL_COPY_PHASHPRO ON CACHE BOOL "use new copy of phashpro lib in build")
set(DL_COPY_GIFLIB   ON CACHE BOOL "use new copy of giflib in build")
else()
set(DL_COPY_BOOST    OFF CACHE BOOL "use dl copy of boost libs")
set(DL_COPY_OPENCV   OFF CACHE BOOL "use new copy of opencv in build")
set(DL_COPY_VCLIB    OFF CACHE BOOL "use new copy of phvideocapture lib in build")
set(DL_COPY_FFMPEG   OFF CACHE BOOL "use new copy of ffmpeg in build") 
set(DL_COPY_PHASHPRO OFF CACHE BOOL "use new copy of phashpro lib in build")
set(DL_COPY_GIFLIB   OFF CACHE BOOL "use new copy of giflib in build")
endif()

# use following vars to indicate location of libraries
set(BOOST_DIR  "" CACHE STRING "boost install dir to use")
set(OPENCV_DIR "" CACHE STRING "opencv install dir to use")
set(VCLIB_DIR  "" CACHE STRING "phvideocapture lib install dir to use")
set(FFMPEG_DIR "" CACHE STRING "ffmpeg lib install dir to use")
set(PHASHPRO_DIR "" CACHE STRING "phashpro lib install dir to use")
set(GIFLIB_DIR "" CACHE STRING  "giflib lib install dir to use")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(clipseekrFindExts)

set(CLIPLIB_SRCS RedisHashStore.cpp clipseekrutils.cpp)
add_library(clipseekr-utils STATIC ${CLIPLIB_SRCS})
add_dependencies(clipseekr-utils hiredis)

set_property(TARGET clipseekr-utils APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/hiredis)
set_property(TARGET clipseekr-utils APPEND PROPERTY COMPILE_FLAGS "-g -Wall -std=c++11")
set_property(TARGET clipseekr-utils APPEND PROPERTY COMPILE_DEFINITIONS __STDC_CONSTANT_MACROS)
set_property(TARGET clipseekr-utils APPEND PROPERTY COMPILE_DEFINITIONS USE_CV_HIGHGUI=1)
set_property(TARGET clipseekr-utils PROPERTY   VERSION "0.2")
set_property(TARGET clipseekr-utils PROPERTY SOVERSION "0.2")
set_property(TARGET clipseekr-utils PROPERTY PRIVATE_HEADER  HashStore.hpp TableEntry.hpp RedisHashStore.hpp) 

if (DL_COPY_FFMPEG)
  add_dependencies(clipseekr-utils ffmpeg-lib)
endif()

target_link_libraries (clipseekr-utils ${avutillib} ${CMAKE_CURRENT_SOURCE_DIR}/hiredis/libhiredis.a)


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


if (CLIPSEEKR_TRIAL_VERSION)
  set_target_properties(clipseekr clipster PROPERTIES
	BUILD_WITH_INSTALL_RPATH TRUE
	INSTALL_RPATH_USE_LINK_PATH TRUE
	INSTALL_RPATH "\$ORIGIN/../libs:\$ORIGIN")
endif()


if (DL_COPY_BOOST)
  add_dependencies(clipster boost-libs)
  add_dependencies(clipseekr boost-libs)
endif()

if (DL_COPY_VCLIB)
  add_dependencies(clipster vclib)
  add_dependencies(clipseekr vclib)
endif()

if (DL_COPY_PHASHPRO)
  add_dependencies(clipster phashpro)
  add_dependencies(clipseekr phashpro)
endif()

if (DL_COPY_OPENCV)
  add_dependencies(clipster opencv)
  add_dependencies(clipseekr opencv)
endif()

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
