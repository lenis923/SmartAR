#include <jni.h>

#include "sample_simple/sample_simple.h"

/*
 * Lifecycle methods.
 * */
extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeCreate
(JNIEnv *env, jobject obj, jobject nativeContext, jint recogMode, jint initMode)
{
	jint sampleCore = reinterpret_cast<jint>(new SampleCoreImpl(nativeContext, env, recogMode, initMode));
	return sampleCore;
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeDestroy
(JNIEnv *, jobject, jint sampleCore)
{
    delete reinterpret_cast<SampleCoreImpl*>(sampleCore);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeResume
(JNIEnv *env, jobject, jint sampleCore, jobject nativeVideoOutput)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doResume(nativeVideoOutput);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativePause
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doPause();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeSurfaceCreated
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doSurfaceCreated();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeSurfaceChanged
(JNIEnv *, jobject, jint sampleCore, jint width, jint height)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doSurfaceChanged(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeDrawFrame
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doDrawFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeDestroySurface
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doDestroySurface();
}

/*
 * Call by UI methods.
 * */
extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeInitializeCameraSettings
(JNIEnv *, jobject, jint sampleCore, jint videoImageSizeSelected
        , jint videoImageFpsRangeSelected
        , jint focusModeSelected
        , jint flashModeSelected
        , jint exposureModeSelected
        , jint whiteBalanceModeSelected
        , jint sceneModeSelected
        , jint useFrontCameraSelected
        , jint useSensorDevice)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->initializeCameraSettings(videoImageSizeSelected
            , videoImageFpsRangeSelected
            , focusModeSelected
            , flashModeSelected
            , exposureModeSelected
            , whiteBalanceModeSelected
            , sceneModeSelected
            , useFrontCameraSelected
            , useSensorDevice);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeCreateFailed
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->isConstructorFailed();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_simple_MainActivity_nativeSmartInitResultCode
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSmartInitResultCode();
}
