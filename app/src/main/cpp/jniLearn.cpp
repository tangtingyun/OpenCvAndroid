//
// Created by gp-0168 on 2019/11/27.
//
#include <jni.h>
#include <inttypes.h>
#include <pthread.h>
#include <string>
#include <android/log.h>
#include <assert.h>
#include <opencv2/opencv.hpp>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_dex_opencvlearn_JniLearnAct_stringJniLearn(JNIEnv *env, jobject thiz) {
    std::string hello = "JniLearn";
    return env->NewStringUTF(hello.c_str());
}