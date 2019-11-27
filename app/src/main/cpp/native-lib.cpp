#include <jni.h>
#include <inttypes.h>
#include <pthread.h>
#include <string>
#include <android/log.h>
#include <assert.h>
#include <opencv2/opencv.hpp>

static const char *kTAG = "hello-JniHandler";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))


extern "C"
JNIEXPORT jintArray JNICALL
Java_com_dex_opencvlearn_OpenCvActivity_grayProc(JNIEnv *env, jclass clazz, jintArray pixels,
                                                 jint w,
                                                 jint h) {
    jint *cbuf;
    cbuf = env->GetIntArrayElements(pixels, JNI_FALSE);
    if (cbuf == NULL) {
        return 0;
    }
    cv::Mat imgData(h, w, CV_8UC4, (unsigned char *) cbuf);

//    cv::Mat imgData = cv::imread("android.jpg", cv::IMREAD_UNCHANGED);

    uchar *ptr = imgData.ptr(0);
    for (int i = 0; i < w * h; i++) {
        //计算公式：Y(亮度) = 0.299*R + 0.587*G + 0.114*B
        //对于一个int四字节，其彩色值存储方式为：BGRA
        int grayScale = (int) (ptr[4 * i + 2] * 0.299 + ptr[4 * i + 1] * 0.587 +
                               ptr[4 * i + 0] * 0.114);
        ptr[4 * i + 1] = grayScale;
        ptr[4 * i + 2] = grayScale;
        ptr[4 * i + 0] = grayScale;
    }

    int size = w * h;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, cbuf);
    env->ReleaseIntArrayElements(pixels, cbuf, 0);
    return result;
}



extern "C"
typedef struct tick_context {
    JavaVM *javaVm;
    jclass jniHelperClz;
    jobject jniHelperObj;
    jclass mainActivityClz;
    jobject mainActivityObj;
    pthread_mutex_t lock;
    int done;
} TickContext;

TickContext g_ctx;


extern "C" JNIEXPORT jstring JNICALL

Java_com_dex_opencvlearn_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

void queryRuntimeInfo(JNIEnv *env, jobject instance) {
    jmethodID versionFunc = env->GetStaticMethodID(
            g_ctx.jniHelperClz,
            "getBuildVersion", "()Ljava/lang/String;");

    if (!versionFunc) {
        LOGE("Failed to retrieve getBuildVersion() methodID @ lint %d", __LINE__);
        return;
    }

    jstring buildVersion = static_cast<jstring>(env->CallStaticObjectMethod(g_ctx.jniHelperClz,
                                                                            versionFunc));

    const char *version = env->GetStringUTFChars(buildVersion, NULL);

    if (!version) {
        LOGE("Unable to get version string @ lint %d", __LINE__);
        return;
    }
    LOGI("Android Version - %s", version);
    env->ReleaseStringChars(buildVersion, (const jchar *) (version));
    env->DeleteLocalRef(buildVersion);

    jmethodID memFunc = env->GetMethodID(g_ctx.jniHelperClz, "getRuntimeMemorySize", "()J");

    if (!memFunc) {
        LOGE("Failed to retrieve getRuntimeMemorySize() methodId @ line %d", __LINE__);
        return;
    }

    jlong result = env->CallLongMethod(instance, memFunc);
    LOGI("Runtime free memory size: % "
                 PRId64, result);
    (void) result;
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    memset(&g_ctx, 0, sizeof(g_ctx));
    g_ctx.javaVm = vm;
    if (vm->GetEnv((void **) (&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clz = env->FindClass("com/dex/opencvlearn/JniHandler");

    g_ctx.jniHelperClz = static_cast<jclass>(env->NewGlobalRef(clz));
    jmethodID jniHelperCtor = env->GetMethodID(g_ctx.jniHelperClz,
                                               "<init>", "()V");

    jobject handler = env->NewObject(g_ctx.jniHelperClz, jniHelperCtor);

    g_ctx.jniHelperObj = env->NewGlobalRef(handler);
    queryRuntimeInfo(env, g_ctx.jniHelperObj);
    g_ctx.done = 0;
    g_ctx.mainActivityObj = NULL;
    return JNI_VERSION_1_6;
}

void sendJavaMsg(JNIEnv *env, jobject instance, jmethodID func, const char *msg) {
    jstring javaMsg = env->NewStringUTF(msg);
    env->CallVoidMethod(instance, func, javaMsg);
    env->DeleteLocalRef(javaMsg);
}

void *UpdateTicks(void *context) {
    TickContext *pctx = static_cast<TickContext *>(context);
    JavaVM *javaVm = pctx->javaVm;
    JNIEnv *env;
    jint res = javaVm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (res != JNI_OK) {
        res = javaVm->AttachCurrentThread(&env, NULL);
        if (JNI_OK != res) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
            return NULL;
        }
    }

    jmethodID statusId = env->GetMethodID(pctx->jniHelperClz, "updateStatus",
                                          "(Ljava/lang/String;)V");

    sendJavaMsg(env, pctx->jniHelperObj, statusId, "TickerThread status: initializing...");

    jmethodID timerId = env->GetMethodID(pctx->mainActivityClz, "updateTimer", "()V");

    struct timeval beginTime, curTime, usedTime, leftTime;
    const struct timeval kOneSecond = {
            (__kernel_time_t) 1,
            (__kernel_suseconds_t) 0
    };

    sendJavaMsg(env, pctx->jniHelperObj, statusId, "TickerThread status: start ticking...");

    while (1) {
        gettimeofday(&beginTime, NULL);
        pthread_mutex_lock(&pctx->lock);
        int done = pctx->done;
        if (pctx->done) {
            pctx->done = 0;
        }
        pthread_mutex_unlock(&pctx->lock);
        if (done) {
            break;
        }
        env->CallVoidMethod(pctx->mainActivityObj, timerId);

        gettimeofday(&curTime, NULL);
        timersub(&curTime, &beginTime, &usedTime);
        timersub(&kOneSecond, &usedTime, &leftTime);
        struct timespec sleepTime;
        sleepTime.tv_sec = leftTime.tv_sec;
        sleepTime.tv_nsec = leftTime.tv_usec * 1000;

        if (sleepTime.tv_sec <= 1) {
            nanosleep(&sleepTime, NULL);
        } else {
            sendJavaMsg(env, pctx->jniHelperObj, statusId,
                        "TickerThread error: processing too long!");
        }
    }
    sendJavaMsg(env, pctx->jniHelperObj, statusId,
                "TickerThread status: ticking stopped");
    javaVm->DetachCurrentThread();
    return context;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_dex_opencvlearn_MainActivity_startTicks(JNIEnv *env, jobject instance) {
    pthread_t threadInfo_;
    pthread_attr_t threadAttr_;

    pthread_attr_init(&threadAttr_);
    pthread_attr_setdetachstate(&threadAttr_, PTHREAD_CREATE_DETACHED);

    pthread_mutex_init(&g_ctx.lock, NULL);

    jclass clz = env->GetObjectClass(instance);
    g_ctx.mainActivityClz = static_cast<jclass>(env->NewGlobalRef(clz));
    g_ctx.mainActivityObj = env->NewGlobalRef(instance);

    int result = pthread_create(&threadInfo_, &threadAttr_, UpdateTicks, &g_ctx);
    assert(result == 0);

    pthread_attr_destroy(&threadAttr_);

    (void) result;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dex_opencvlearn_MainActivity_StopTicks(JNIEnv *env, jobject thiz) {
    pthread_mutex_lock(&g_ctx.lock);
    g_ctx.done = 1;
    pthread_mutex_unlock(&g_ctx.lock);

    // waiting for ticking thread to flip the done flag
    struct timespec sleepTime;
    memset(&sleepTime, 0, sizeof(sleepTime));
    sleepTime.tv_nsec = 100000000;
    while (g_ctx.done) {
        nanosleep(&sleepTime, NULL);
    }

    // release object we allocated from StartTicks() function
    env->DeleteGlobalRef(g_ctx.mainActivityClz);
    env->DeleteGlobalRef(g_ctx.mainActivityObj);
    g_ctx.mainActivityObj = NULL;
    g_ctx.mainActivityClz = NULL;

    pthread_mutex_destroy(&g_ctx.lock);
}