# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

message("CURRENT_DIR:" ${CMAKE_SOURCE_DIR})

set(CMAKE_VERBOSE_MAKEFILE on)

#设置头文件路径
include_directories(include)


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)

add_library( # Sets the name of the library.
        jniLearn-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        jniLearn.cpp)


add_library(
        opencv-lib
        SHARED
        IMPORTED)
set_target_properties(opencv-lib
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libopencv_java4.so)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# 在ndk中查找jnigraphics库 取别名jnigraphics-lib
# jnigraphics包含图形操作的库
find_library(jnigraphics-lib jnigraphics)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
        opencv-lib
        # Links the target library to the log library
        # included in the NDK.
        ${jnigraphics-lib}
        ${log-lib})

target_link_libraries( # Specifies the target library.
        jniLearn-lib
        opencv-lib
        # Links the target library to the log library
        # included in the NDK.
        ${jnigraphics-lib}
        ${log-lib})
