#include <jni.h>

#include "sample_full/sample_full.h"

/*
 * Call java function methods.
 */
//To call java method in onImage() callback.
JNIEnv* mediaScannerJNIEnv_;
jobject mediaScannerglobalRef_;
void mediaScannerFunctionInJNI(){
	//Call MediaScanner method in Java.
	jclass activityClass = mediaScannerJNIEnv_->GetObjectClass(mediaScannerglobalRef_);
	jmethodID getMethodID = mediaScannerJNIEnv_->GetMethodID(activityClass, "mediaScan", "()V");
	if (getMethodID != 0){
		mediaScannerJNIEnv_->CallVoidMethod(mediaScannerglobalRef_, getMethodID);
	} else {
		SAR_SMARTAR_LOGD("TAG","Function mediaScan not found.");
	}
	mediaScannerJNIEnv_->DeleteLocalRef(activityClass);
	mediaScannerJNIEnv_->DeleteGlobalRef(mediaScannerglobalRef_);
	mediaScannerglobalRef_ = NULL;
	return;
}

/*
 * Lifecycle methods.
 * */
extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeCreate
(JNIEnv *env, jobject obj, jobject nativeContext, jint recogMode, jint initMode)
{
	jint sampleCore = reinterpret_cast<jint>(new SampleCoreImpl(nativeContext, env, recogMode, initMode));
	return sampleCore;
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeDestroy
(JNIEnv *, jobject, jint sampleCore)
{
    delete reinterpret_cast<SampleCoreImpl*>(sampleCore);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeResume
(JNIEnv *env, jobject, jint sampleCore, jobject nativeVideoOutput)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doResume(nativeVideoOutput);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativePause
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doPause();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSurfaceCreated
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doSurfaceCreated();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSurfaceChanged
(JNIEnv *, jobject, jint sampleCore, jint width, jint height)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doSurfaceChanged(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeDrawFrame
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doDrawFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeDestroySurface
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->doDestroySurface();
}

/*
 * Call by UI methods.
 * */
extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSaveSceneMap
(JNIEnv *env, jobject obj, jint sampleCore, jstring filePath)
{
	const char* utfFilePath = env->GetStringUTFChars(filePath, NULL);
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->saveSceneMap(utfFilePath);
	env->ReleaseStringUTFChars(filePath, utfFilePath);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeLoadSceneMap
(JNIEnv *env, jobject, jint sampleCore, jstring filePath)
{
	const char* utfFilePath = env->GetStringUTFChars(filePath, NULL);
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->loadSceneMap(utfFilePath);
	env->ReleaseStringUTFChars(filePath, utfFilePath);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetLearnedImageTarget
(JNIEnv *env, jobject, jint sampleCore, jbooleanArray boolArrayObj)
{
	jboolean *booleanArray = env->GetBooleanArrayElements(boolArrayObj, NULL);
	bool boolArray[10];
	for (int i = 0; i < 10; i++) {
		boolArray[i] = booleanArray[i];
	}
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setLearnedImageTarget(boolArray);
	env->ReleaseBooleanArrayElements(boolArrayObj, booleanArray, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetLoadedTarget
(JNIEnv *env, jobject, jint sampleCore, jobjectArray stringObj, jbooleanArray boolArrayObj) {
	int arrayLength = env->GetArrayLength(stringObj);
	char charArray[arrayLength][FILE_NAME_LENGTH];
	if(boolArrayObj == NULL){
		reinterpret_cast<SampleCoreImpl*>(sampleCore)->getLoadedTarget(charArray, NULL);
		for (int i = 0; i < arrayLength; i++) {
			env->SetObjectArrayElement(stringObj, i, env->NewStringUTF(charArray[i]));
		}
	}else{
		jboolean *booleanArray = env->GetBooleanArrayElements(boolArrayObj, NULL);
		bool boolArray[arrayLength];
		reinterpret_cast<SampleCoreImpl*>(sampleCore)->getLoadedTarget(charArray, boolArray);
		for (int i = 0; i < arrayLength; i++) {
			booleanArray[i] = boolArray[i];
			env->SetObjectArrayElement(stringObj, i, env->NewStringUTF(charArray[i]));
		}
		env->ReleaseBooleanArrayElements(boolArrayObj, booleanArray, 0);
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeLoadTarget
(JNIEnv *env, jobject, jint sampleCore, jstring filePath, jstring label)
{
	const char* utfFilePath = env->GetStringUTFChars(filePath, NULL);
	const char* utfLabel = env->GetStringUTFChars(label, NULL);
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->loadTarget(utfFilePath, utfLabel);
	env->ReleaseStringUTFChars(filePath, utfFilePath);
	env->ReleaseStringUTFChars(label, utfLabel);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeChangeCamera
(JNIEnv *env, jobject, jint sampleCore, jobject nativeVideoOutput)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->changeCamera(nativeVideoOutput);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeForceLocalize
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->forceLocalize();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeResetRecognizer
(JNIEnv *, jobject, jint sampleCore)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->resetRecognizer();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetSearchPolicy
(JNIEnv *, jobject, jint sampleCore, jint searchPolicy)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setSearchPolicy(searchPolicy);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetDenseMapMode
(JNIEnv *, jobject, jint sampleCore, jint denseMapMode)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setDenseMapMode(denseMapMode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeCaptureStillImage
(JNIEnv *env, jobject obj, jint sampleCore, jstring filePath)
{
	const char* utfFilePath = env->GetStringUTFChars(filePath, NULL);
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setStillImageFilePath(utfFilePath);
	env->ReleaseStringUTFChars(filePath, utfFilePath);

    mediaScannerJNIEnv_ = env;
	if(mediaScannerglobalRef_ != NULL) {
		mediaScannerJNIEnv_->DeleteGlobalRef(mediaScannerglobalRef_);
		mediaScannerglobalRef_ = NULL;
	}
	mediaScannerglobalRef_ = env->NewGlobalRef(obj);
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setMediaScannerFunction(mediaScannerFunctionInJNI);
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->captureStillImage();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfSupportedVideoImageSize
(JNIEnv *env, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfSupportedVideoImageSize();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfSupportedStillImageSize
(JNIEnv *env, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfSupportedStillImageSize();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfSupportedVideoImageFpsRange
(JNIEnv *env, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfSupportedVideoImageFpsRange();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfSupportedFocusMode
(JNIEnv *env, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfSupportedFocusMode();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfSupportedFlashMode
(JNIEnv *env, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfSupportedFlashMode();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfSupportedExposureMode
(JNIEnv *env, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfSupportedExposureMode();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfSupportedWhiteBalanceMode
(JNIEnv *env, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfSupportedWhiteBalanceMode();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfSupportedSceneMode
(JNIEnv *env, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfSupportedSceneMode();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSupportedVideoImageSize
(JNIEnv *env, jobject, jint sampleCore, jobjectArray sizeArray) {
	int arrayLength = env->GetArrayLength(sizeArray);
	char charArray[arrayLength][IMAGE_SIZE_STRING_LENGTH];
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSupportedVideoImageSize(charArray, arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		env->SetObjectArrayElement(sizeArray, i, env->NewStringUTF(charArray[i]));
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSupportedStillImageSize
(JNIEnv *env, jobject, jint sampleCore, jobjectArray sizeArray) {
	int arrayLength = env->GetArrayLength(sizeArray);
	char charArray[arrayLength][IMAGE_SIZE_STRING_LENGTH];
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSupportedStillImageSize(charArray, arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		env->SetObjectArrayElement(sizeArray, i, env->NewStringUTF(charArray[i]));
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSupportedVideoImageFpsRange
(JNIEnv *env, jobject, jint sampleCore, jobjectArray sizeArray) {
	int arrayLength = env->GetArrayLength(sizeArray);
	char charArray[arrayLength][FPS_RANGE_STRING_LENGTH];
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSupportedVideoImageFpsRange(charArray, arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		env->SetObjectArrayElement(sizeArray, i, env->NewStringUTF(charArray[i]));
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSupportedFocusMode
(JNIEnv *env, jobject, jint sampleCore, jobjectArray sizeArray) {
	int arrayLength = env->GetArrayLength(sizeArray);
	char charArray[arrayLength][FOCUS_MODE_STRING_LENGTH];
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSupportedFocusMode(charArray, arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		env->SetObjectArrayElement(sizeArray, i, env->NewStringUTF(charArray[i]));
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSupportedFlashMode
(JNIEnv *env, jobject, jint sampleCore, jobjectArray sizeArray) {
	int arrayLength = env->GetArrayLength(sizeArray);
	char charArray[arrayLength][FLASH_MODE_STRING_LENGTH];
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSupportedFlashMode(charArray, arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		env->SetObjectArrayElement(sizeArray, i, env->NewStringUTF(charArray[i]));
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSupportedExposureMode
(JNIEnv *env, jobject, jint sampleCore, jobjectArray sizeArray) {
	int arrayLength = env->GetArrayLength(sizeArray);
	char charArray[arrayLength][EXPOSURE_MODE_STRING_LENGTH];
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSupportedExposureMode(charArray, arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		env->SetObjectArrayElement(sizeArray, i, env->NewStringUTF(charArray[i]));
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSupportedWhiteBalanceMode
(JNIEnv *env, jobject, jint sampleCore, jobjectArray sizeArray) {
	int arrayLength = env->GetArrayLength(sizeArray);
	char charArray[arrayLength][WHITE_BALANCE_MODE_STRING_LENGTH];
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSupportedWhiteBalanceMode(charArray, arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		env->SetObjectArrayElement(sizeArray, i, env->NewStringUTF(charArray[i]));
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSupportedSceneMode
(JNIEnv *env, jobject, jint sampleCore, jobjectArray sizeArray) {
	int arrayLength = env->GetArrayLength(sizeArray);
	char charArray[arrayLength][SCENE_MODE_STRING_LENGTH];
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSupportedSceneMode(charArray, arrayLength);
	for (int i = 0; i < arrayLength; i++) {
		env->SetObjectArrayElement(sizeArray, i, env->NewStringUTF(charArray[i]));
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetCameraStillImageSize
(JNIEnv *, jobject, jint sampleCore, jint selectNum){
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setCameraStillImageSize(selectNum);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetVideoImageFpsRange
(JNIEnv *, jobject, jint sampleCore, jint selectNum){
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setVideoImageFpsRange(selectNum);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetFocusMode
(JNIEnv *, jobject, jint sampleCore, jint selectNum){
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setFocusMode(selectNum);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetFlashMode
(JNIEnv *, jobject, jint sampleCore, jint selectNum){
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setFlashMode(selectNum);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetExposureMode
(JNIEnv *, jobject, jint sampleCore, jint selectNum){
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setExposureMode(selectNum);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetWhiteBalanceMode
(JNIEnv *, jobject, jint sampleCore, jint selectNum){
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setWhiteBalanceMode(selectNum);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetSceneMode
(JNIEnv *, jobject, jint sampleCore, jint selectNum){
	reinterpret_cast<SampleCoreImpl*>(sampleCore)->setSceneMode(selectNum);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetInitMode
(JNIEnv *env, jobject, jint sampleCore)
{
	return env->NewStringUTF(reinterpret_cast<SampleCoreImpl*>(sampleCore)->getInitMode());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetInitModeNum
(JNIEnv *env, jobject, jint sampleCore)
{
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getInitModeNum();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetState
(JNIEnv *env, jobject, jint sampleCore)
{
	return env->NewStringUTF(reinterpret_cast<SampleCoreImpl*>(sampleCore)->getState());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetNumOfLoadedTarget
(JNIEnv *, jobject, jint sampleCore) {
	return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getNumOfLoadedTarget();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeCameraFrameCount
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getCameraFrameCount();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeRecogCount
(JNIEnv *, jobject, jint sampleCore, jint index)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getRecogCount(index);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeRecogTime
(JNIEnv *, jobject, jint sampleCore, jint index)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getRecogTime(index);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsRecogModeSceneMapping
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isRecogModeSceneMapping();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsSearchPolicyPresicive
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isSearchPolicyPresicive();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsDenseMapModeSemiDense
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isDenseMapModeSemiDense();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetTrackedLandmarkCount
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getTrackedLandmarkCount();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetLostLandmarkCount
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getLostLandmarkCount();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSuspendedLandmarkCount
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSuspendedLandmarkCount();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetMaskedLandmarkCount
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getMaskedLandmarkCount();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetVideoImageSizeSelected
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getVideoImageSizeSelected();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetStillImageSizeSelected
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getStillImageSizeSelected();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetVideoImageFpsRangeSelected
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getVideoImageFpsRangeSelected();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsUseWorkerThread
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isUseWorkerThread();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetUseWorkerThreadFlag
(JNIEnv *, jobject, jint sampleCore, jboolean flag)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setUseWorkerThreadFlag((bool)flag);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetUseTriangulateMasksFlag
(JNIEnv *, jobject, jint sampleCore, jboolean flag)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setUseTriangulateMasksFlag((bool)flag);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetFixSceneMapFlag
(JNIEnv *, jobject, jint sampleCore, jboolean flag)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setFixSceneMapFlag((bool)flag);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetRemoveLostLandmarksFlag
(JNIEnv *, jobject, jint sampleCore, jboolean flag)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setRemoveLostLandmarksFlag(flag);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetFocusModeSelected
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getFocusModeSelected();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetFlashModeSelected
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getFlashModeSelected();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetExposureModeSelected
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getExposureModeSelected();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetWhiteBalanceModeSelected
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getWhiteBalanceModeSelected();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeGetSceneModeSelected
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSceneModeSelected();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetLandMarkBarLocation
(JNIEnv *env, jobject, jint sampleCore, jint x, jint y)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setLandMarkBarLocation(x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetShowLandMarkFlag
(JNIEnv *, jobject, jint sampleCore, jboolean flag)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setShowLandMarkFlag(flag);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSetShowLandMarkBarFlag
(JNIEnv *, jobject, jint sampleCore, jboolean flag)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->setShowLandMarkbarFlag(flag);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeStopSensorDevice
(JNIEnv *, jobject, jint sampleCore, jboolean flag)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->stopSensorDevice();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeStartSensorDevice
(JNIEnv *, jobject, jint sampleCore, jboolean flag)
{
    reinterpret_cast<SampleCoreImpl*>(sampleCore)->startSensorDevice();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsUseFrontCamera
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isUseFrontCamera();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsUseSensorDevice
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isUseSensorDevice();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsFixSceneMap
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isFixSceneMap();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsRemoveLostLandmarks
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isRemoveLostLandmarks();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeIsUseTriangulateMasks
(JNIEnv *, jobject, jint sampleCore)
{
	return (jboolean)reinterpret_cast<SampleCoreImpl*>(sampleCore)->isUseTriangulateMasks();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeInitializeCameraSettings
(JNIEnv *, jobject, jint sampleCore, jint videoImageSizeSelected
		, jint stillImageSizeSelected
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
    		, stillImageSizeSelected
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
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeCreateFailed
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->isConstructorFailed();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sony_smartar_sampleapi_full_MainActivity_nativeSmartInitResultCode
(JNIEnv *, jobject, jint sampleCore)
{
    return reinterpret_cast<SampleCoreImpl*>(sampleCore)->getSmartInitResultCode();
}
