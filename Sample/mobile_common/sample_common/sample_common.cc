#include "sample_common.h"
#include "sample_common_util.h"

#include <math.h>

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

using namespace sarSmartar;

SampleCore::SampleCore(void* nativeContext, void* nativeEnv, int32_t recogMode, int32_t initMode)
: cameraController_(this)
{
}

SampleCore::~SampleCore()
{
    SAR_SMARTAR_LOGD(TAG, "nativeDestroy ***************************************************************************");

    // Release worker threads
    workerThreadController_.dispose();

    // Release drawer
    backgroundDrawer_.dispose();

    // Release targets
    CHECK_ERR(recognizer_->sarSetTargets(NULL, 0));
	for(int i = 0; i < loadedTargets_.size(); i++){
		SAR_SMARTAR_LOGD(TAG, "delete loaded target %d" , i);
		delete loadedTargets_[i].target_;
	}
	loadedTargets_.clear();

	if(sceneMapTarget_ != NULL){
	    delete sceneMapTarget_;
	    sceneMapTarget_ = NULL;
	}

	// Only clear ActiveTargets. Because ActiveTargets are loadedTargets or sceneMapTarget, already deleted
	activeTargets_.clear();

    // Release recognizer
    delete recognizer_;
    recognizer_ = NULL;

    // Release smart
    delete smart_;
    smart_ = NULL;
}

void SampleCore::doResume(void* nativeVideoOutput)
{
    SAR_SMARTAR_LOGD(TAG, "nativeResume ***************************************************************************");

    cameraController_.setUseFrontCamera(useFrontCameraSelected_);

    // Open camera
    cameraController_.open(smart_, nativeVideoOutput);

    SarCameraDeviceInfo cameraDeviceInfo;
    CHECK_ERR(cameraController_.getCameraDevice()->sarGetDeviceInfo(&cameraDeviceInfo));
    CHECK_ERR(recognizer_->sarSetCameraDeviceInfo(cameraDeviceInfo));

    SarSensorDeviceInfo sensorDeviceInfo;
    CHECK_ERR(cameraController_.getSensorDevice()->sarGetDeviceInfo(&sensorDeviceInfo));
    CHECK_ERR(recognizer_->sarSetSensorDeviceInfo(sensorDeviceInfo));

    SarFacing cameraFacing = useFrontCameraSelected_ ? SAR_FACING_FRONT : SAR_FACING_BACK;
    SarRotation cameraRotation;
    CHECK_ERR(cameraController_.getCameraDevice()->sarGetRotation(&cameraRotation));
    backgroundDrawer_.setCameraParameter(cameraFacing, cameraRotation);

    cameraController_.start();

    // Set default or saved status to camera device. If a parameter is -1, method sets default value.
    setCameraVideoImageSize(videoImageSizeSelected_);
    setVideoImageFpsRange(videoImageFpsRangeSelected_);
    setFocusMode(focusModeSelected_);
    setFlashMode(flashModeSelected_);
    setExposureMode(exposureModeSelected_);
    setWhiteBalanceMode(whiteBalanceModeSelected_);
    setSceneMode(sceneModeSelected_);

    if(!useSensorDevice_) cameraController_.getSensorDevice()->sarStop();

    // Start worker thread
    workerThreadController_.start();

    resumed_ = true;
}

void SampleCore::doPause()
{
    SAR_SMARTAR_LOGD(TAG, "nativePause ***************************************************************************");

    resumed_ = false;

    // Stop worker thread
    workerThreadController_.stop();

    // Close camera
    cameraController_.stop();
    cameraController_.close();

    // Reset status
    CHECK_ERR(recognizer_->sarReset());
    videoImageQueue_->sarClear();
}

void SampleCore::doSurfaceCreated()
{
    SAR_SMARTAR_LOGD(TAG, "nativeSurfaceCreated ***************************************************************************");
}

void SampleCore::doSurfaceChanged(int32_t width, int32_t height)
{
    SAR_SMARTAR_LOGD(TAG, "nativeSurfaceChanged %d %d ***************************************************************************", width, height);

    if (!surfaceCreated_) {
        // Start drawers
        backgroundDrawer_.startDraw();
        contentDrawer_.startDraw();

        surfaceCreated_ = true;
    }

    screenWidth_ = width;
    screenHeight_ = height;

    glViewport(0, 0, width, height);
}

void SampleCore::doDestroySurface()
{
    SAR_SMARTAR_LOGD(TAG, "nativeDestroySurface ***************************************************************************");
    surfaceCreated_ = false;

    // Stop drawers
    backgroundDrawer_.stopDraw();
    contentDrawer_.stopDraw();
}

/*
 * CameraController::Listener::onImage() callback
 */
void SampleCore::onImage(const SarImageHolder& imageHolder, uint64_t timestamp, int32_t numSensorStates, SarSensorState* sensorStates)
{
	/*
	 * Must implement for get camera image
	 */
}

void SampleCore::resetRecognizer() {
    if (resumed_) {
    	workerThreadController_.stop();
    	CHECK_ERR(recognizer_->sarReset());
    	workerThreadController_.start();
    }
}

void SampleCore::setSearchPolicy(int32_t searchPolicy)
{
	workerThreadController_.stop();
	switch(searchPolicy){
		case SAR_SEARCH_POLICY_FAST:
			CHECK_ERR(recognizer_->sarSetSearchPolicy(SAR_SEARCH_POLICY_FAST));
    		searchPolicy_ = SAR_SEARCH_POLICY_FAST;
			break;
		case SAR_SEARCH_POLICY_PRECISIVE:
			CHECK_ERR(recognizer_->sarSetSearchPolicy(SAR_SEARCH_POLICY_PRECISIVE));
    		searchPolicy_ = SAR_SEARCH_POLICY_PRECISIVE;
			break;
		default:
			break;
	}
	workerThreadController_.start();
}

const char* SampleCore::getInitMode() {
	if(recogMode_ == SAR_RECOGNITION_MODE_SCENE_MAPPING){
		return SceneMappingInitModeArray[initMode_];
	}else{
		return "NOT_SCENE_MAPPING";
	}
}

const char* SampleCore::getState(){
	switch(recogMode_){
		case SAR_RECOGNITION_MODE_TARGET_TRACKING:
			return TargetTrackingStateArray[state_];
		case SAR_RECOGNITION_MODE_SCENE_MAPPING:
			return SceneMappingStateArray[state_];
		default:
			break;
	}
	return "ERROR_STATE";
}

void SampleCore::loadSceneMap(const char* filePath)
{
	SAR_SMARTAR_LOGD(TAG, "loadSceneMap ********************* %s", filePath);
	if(recogMode_ == SAR_RECOGNITION_MODE_SCENE_MAPPING){
		SarFileStreamIn fileStreamIn(smart_, filePath);
		CHECK_CONSTRUCT(sceneMapTarget_ = new SarSceneMapTarget(smart_, &fileStreamIn));

		cameraController_.stop();
		workerThreadController_.stop();
		CHECK_ERR(recognizer_->sarReset());
		activeTargets_.clear();
		activeTargets_.push_back(sceneMapTarget_);
		targetCount_ = 1;
	    CHECK_ERR(recognizer_->sarSetTargets(&activeTargets_[0], targetCount_));
	    CHECK_ERR(recognizer_->sarFixSceneMap(false));
	    workerThreadController_.start();
	    cameraController_.start();
	}
	return;
}

void SampleCore::saveSceneMap(const char* filePath)
{
	SAR_SMARTAR_LOGD(TAG, "saveSceneMap ********************* %s", filePath);
	workerThreadController_.stop();
	if(recogMode_ == SAR_RECOGNITION_MODE_SCENE_MAPPING){
		SarFileStreamOut fileStreamOut(smart_, filePath);
		CHECK_ERR(recognizer_->sarSaveSceneMap(&fileStreamOut));
	}
	workerThreadController_.start();
}

void SampleCore::forceLocalize(){
	workerThreadController_.stop();
	CHECK_ERR(recognizer_->sarForceLocalize());
	workerThreadController_.start();
}

void SampleCore::setLearnedImageTarget(bool *isUsedArray)
{
	cameraController_.stop();
	workerThreadController_.stop();
	activeTargets_.clear();

	targetCount_ = 0;

	for(int i = 0; i < loadedTargets_.size(); i++){
		loadedTargets_[i].isUsed_ = isUsedArray[i];
		if(isUsedArray[i]){
			activeTargets_.push_back(loadedTargets_[i].target_);
			targetCount_++;
		}
	}

	if (activeTargets_.size() > 0) {
	    CHECK_ERR(recognizer_->sarSetTargets(&activeTargets_[0], targetCount_));
	}
	else {
	    CHECK_ERR(recognizer_->sarSetTargets(NULL, 0));
	}
    workerThreadController_.start();
    cameraController_.start();
	return;
}

int SampleCore::getNumOfSupportedVideoImageSize(){
	std::vector<SarSize> supportedVideoImageSizeBuffer(SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);
	return cameraController_.getCameraDevice()->sarGetSupportedVideoImageSize(&supportedVideoImageSizeBuffer[0], SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);
}

int SampleCore::getNumOfSupportedVideoImageFpsRange(){
	std::vector<SarCameraFpsRange> supportedVideoImageFpsRangeBuffer(SUPPORTED_NUM_OF_FPS_RANGE);
	return cameraController_.getCameraDevice()->sarGetSupportedVideoImageFpsRange(&supportedVideoImageFpsRangeBuffer[0], SUPPORTED_NUM_OF_FPS_RANGE);
}

int SampleCore::getNumOfSupportedFocusMode(){
	std::vector<SarFocusMode> supportedFocusModeBuffer(SUPPORTED_NUM_OF_FOCUS_MODE);
	return cameraController_.getCameraDevice()->sarGetSupportedFocusMode(&supportedFocusModeBuffer[0], SUPPORTED_NUM_OF_FOCUS_MODE);
}
int SampleCore::getNumOfSupportedFlashMode(){
	std::vector<SarFlashMode> supportedFlashModeBuffer(SUPPORTED_NUM_OF_FOCUS_MODE);
	return cameraController_.getCameraDevice()->sarGetSupportedFlashMode(&supportedFlashModeBuffer[0], SUPPORTED_NUM_OF_FLASH_MODE);
}
int SampleCore::getNumOfSupportedExposureMode(){
	std::vector<SarExposureMode> supportedExposureModeBuffer(SUPPORTED_NUM_OF_FOCUS_MODE);
	return cameraController_.getCameraDevice()->sarGetSupportedExposureMode(&supportedExposureModeBuffer[0], SUPPORTED_NUM_OF_EXPOSURE_MODE);
}
int SampleCore::getNumOfSupportedWhiteBalanceMode(){
	std::vector<SarWhiteBalanceMode> supportedWhiteBalanceModeBuffer(SUPPORTED_NUM_OF_FOCUS_MODE);
	return cameraController_.getCameraDevice()->sarGetSupportedWhiteBalanceMode(&supportedWhiteBalanceModeBuffer[0], SUPPORTED_NUM_OF_WHITE_BALANCE_MODE);
}
int SampleCore::getNumOfSupportedSceneMode(){
	std::vector<SarSceneMode> supportedSceneModeBuffer(SUPPORTED_NUM_OF_FOCUS_MODE);
	return cameraController_.getCameraDevice()->sarGetSupportedSceneMode(&supportedSceneModeBuffer[0], SUPPORTED_NUM_OF_SCENE_MODE);
}

void SampleCore::getSupportedVideoImageSize(char charArray[][IMAGE_SIZE_STRING_LENGTH], int arrayLength){
	std::vector<SarSize> supportedVideoImageSizeBuffer(arrayLength);
	int supportedVideoImageSizeCount
	= cameraController_.getCameraDevice()->sarGetSupportedVideoImageSize(&supportedVideoImageSizeBuffer[0], arrayLength);
	for(int i = 0; i < supportedVideoImageSizeCount; i++){
		strcpy(charArray[i], "");
		char strBufferWidth[IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH] = "";
		char strBufferHeight[IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH] = "";
		snprintf(strBufferWidth, IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH, "%d", supportedVideoImageSizeBuffer[i].width_);
		snprintf(strBufferHeight, IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH, "%d", supportedVideoImageSizeBuffer[i].height_);

		strncat(charArray[i], strBufferWidth, IMAGE_SIZE_STRING_LENGTH);
		strncat(charArray[i], "*", IMAGE_SIZE_STRING_LENGTH - strlen(charArray[i]));
		strncat(charArray[i], strBufferHeight, IMAGE_SIZE_STRING_LENGTH - strlen(charArray[i]));
	}
}


void SampleCore::getSupportedVideoImageFpsRange(char charArray[][FPS_RANGE_STRING_LENGTH], int arrayLength){
	std::vector<SarCameraFpsRange> supportedVideoImageFpsRangeBuffer(arrayLength);
	int supportedVideoImageFpsRangeCount
	= cameraController_.getCameraDevice()->sarGetSupportedVideoImageFpsRange(&supportedVideoImageFpsRangeBuffer[0], arrayLength);
	for(int i = 0; i < supportedVideoImageFpsRangeCount; i++){
		strcpy(charArray[i], "");
		char strBufferMin[FPS_RANGE_MAX_OR_MIN_STRING_LENGTH] = "";
		char strBufferMax[FPS_RANGE_MAX_OR_MIN_STRING_LENGTH] = "";
		snprintf(strBufferMin, FPS_RANGE_MAX_OR_MIN_STRING_LENGTH, "%f", supportedVideoImageFpsRangeBuffer[i].min_);
		snprintf(strBufferMax, FPS_RANGE_MAX_OR_MIN_STRING_LENGTH, "%f", supportedVideoImageFpsRangeBuffer[i].max_);

		//Delete unnecessary '0' after '.' in float char sequence.
		int floatPoint = -1;
		for(int j = 0; j < strlen(strBufferMin); j++){
			if(strBufferMin[j] == '.') floatPoint = j;
		}
		if(floatPoint >= 0){
			while(strlen(strBufferMin) > 0){
				int readingPoint = static_cast<int>(strlen(strBufferMin) - 1);
				if(readingPoint == floatPoint) break;
				if(strBufferMin[readingPoint] == '0'){
					if(readingPoint - 1 == floatPoint) break;
					strBufferMin[readingPoint] = '\0';
				}else{
					break;
				}
			}
		}
		floatPoint = -1;
		for(int j = 0; j < strlen(strBufferMax); j++){
			if(strBufferMax[j] == '.') floatPoint = j;
		}
		if(floatPoint >= 0){
			while(strlen(strBufferMax) > 0){
				int readingPoint = static_cast<int>(strlen(strBufferMax) - 1);
				if(readingPoint == floatPoint) break;
				if(strBufferMax[readingPoint] == '0'){
					if(readingPoint - 1 == floatPoint) break;
					strBufferMax[readingPoint] = '\0';
				}else{
					break;
				}
			}
		}


		strncat(charArray[i], strBufferMin, FPS_RANGE_STRING_LENGTH);
		strncat(charArray[i], " - ", FPS_RANGE_STRING_LENGTH - strlen(charArray[i]));
		strncat(charArray[i], strBufferMax, FPS_RANGE_STRING_LENGTH - strlen(charArray[i]));
	}
}

void SampleCore::getSupportedFocusMode(char charArray[][FOCUS_MODE_STRING_LENGTH], int arrayLength){
	std::vector<SarFocusMode> supportedFocusModeBuffer(arrayLength);
	int supportedFocusModeCount
	= cameraController_.getCameraDevice()->sarGetSupportedFocusMode(&supportedFocusModeBuffer[0], arrayLength);
	for(int i = 0; i < supportedFocusModeCount; i++){
		strcpy(charArray[i], FocusModeArray[supportedFocusModeBuffer[i]]);
	}
}
void SampleCore::getSupportedFlashMode(char charArray[][FLASH_MODE_STRING_LENGTH], int arrayLength){
	std::vector<SarFlashMode> supportedFlashModeBuffer(arrayLength);
	int supportedFlashModeCount
	= cameraController_.getCameraDevice()->sarGetSupportedFlashMode(&supportedFlashModeBuffer[0], arrayLength);
	for(int i = 0; i < supportedFlashModeCount; i++){
		strcpy(charArray[i], FlashModeArray[supportedFlashModeBuffer[i]]);
	}
}
void SampleCore::getSupportedExposureMode(char charArray[][EXPOSURE_MODE_STRING_LENGTH], int arrayLength){
	std::vector<SarExposureMode> supportedExposureModeBuffer(arrayLength);
	int supportedExposureModeCount
	= cameraController_.getCameraDevice()->sarGetSupportedExposureMode(&supportedExposureModeBuffer[0], arrayLength);
	for(int i = 0; i < supportedExposureModeCount; i++){
		strcpy(charArray[i], ExposureModeArray[supportedExposureModeBuffer[i]]);
	}
}
void SampleCore::getSupportedWhiteBalanceMode(char charArray[][WHITE_BALANCE_MODE_STRING_LENGTH], int arrayLength){
	std::vector<SarWhiteBalanceMode> supportedWhiteBalanceModeBuffer(arrayLength);
	int supportedWhiteBalanceModeCount
	= cameraController_.getCameraDevice()->sarGetSupportedWhiteBalanceMode(&supportedWhiteBalanceModeBuffer[0], arrayLength);
	for(int i = 0; i < supportedWhiteBalanceModeCount; i++){
		strcpy(charArray[i], WhiteBalanceModeArray[supportedWhiteBalanceModeBuffer[i]]);
	}
}
void SampleCore::getSupportedSceneMode(char charArray[][SCENE_MODE_STRING_LENGTH], int arrayLength){
	std::vector<SarSceneMode> supportedSceneModeBuffer(arrayLength);
	int supportedSceneModeCount
	= cameraController_.getCameraDevice()->sarGetSupportedSceneMode(&supportedSceneModeBuffer[0], arrayLength);
	for(int i = 0; i < supportedSceneModeCount; i++){
		strcpy(charArray[i], SceneModeArray[supportedSceneModeBuffer[i]]);
	}
}

void SampleCore::setCameraVideoImageSize(int32_t selectNum){
	cameraController_.stop();

	videoImageSizeSelected_ = selectNum;
	if(selectNum == -1){
		if(sarIsMultiCore()){
			videoImageSizeSelected_ = getNearestVideoImageSize(VGA);
		}else{
			videoImageSizeSelected_ = getNearestVideoImageSize(HVGA);
		}
	}
	std::vector<SarSize> supportedVideoImageSizeBuffer(SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);
	/*int supportedVideoImageSizeCount =*/cameraController_.getCameraDevice()
			->sarGetSupportedVideoImageSize(&supportedVideoImageSizeBuffer[0]
			        , SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);

	int width = supportedVideoImageSizeBuffer[videoImageSizeSelected_].width_;
	int height = supportedVideoImageSizeBuffer[videoImageSizeSelected_].height_;

	cameraController_.getCameraDevice()->sarSetVideoImageSize(width, height);
	selectedSize_ = SarSize(width, height);

    SarCameraDeviceInfo cameraDeviceInfo;
    CHECK_ERR(cameraController_.getCameraDevice()->sarGetDeviceInfo(&cameraDeviceInfo));
    CHECK_ERR(recognizer_->sarSetCameraDeviceInfo(cameraDeviceInfo));

    cameraController_.start();
}

void SampleCore::setVideoImageFpsRange(int32_t selectNum){
	cameraController_.stop();

	videoImageFpsRangeSelected_ = selectNum;
	if(selectNum == -1)
		videoImageFpsRangeSelected_ = getNearestVideoImageFpsRange(BEST_FPS);//For default setting.

	std::vector<SarCameraFpsRange> supportedVideoImageFpsRangeBuffer(SUPPORTED_NUM_OF_FPS_RANGE);
	int supportedVideoImageFpsRangeCount = cameraController_.getCameraDevice()
			->sarGetSupportedVideoImageFpsRange(&supportedVideoImageFpsRangeBuffer[0]
			        , SUPPORTED_NUM_OF_FPS_RANGE);

	if(supportedVideoImageFpsRangeCount > 0)
		cameraController_.getCameraDevice()->sarSetVideoImageFpsRange(
				supportedVideoImageFpsRangeBuffer[videoImageFpsRangeSelected_].min_, supportedVideoImageFpsRangeBuffer[videoImageFpsRangeSelected_].max_);

    SarCameraDeviceInfo cameraDeviceInfo;
    CHECK_ERR(cameraController_.getCameraDevice()->sarGetDeviceInfo(&cameraDeviceInfo));
    CHECK_ERR(recognizer_->sarSetCameraDeviceInfo(cameraDeviceInfo));

    cameraController_.start();
}

void SampleCore::setFocusMode(int32_t selectNum){
	std::vector<SarFocusMode> supportedFocusModeBuffer(FOCUS_MODE_STRING_LENGTH);
	int supportedFocusModeCount = cameraController_.getCameraDevice()->sarGetSupportedFocusMode(&supportedFocusModeBuffer[0], FOCUS_MODE_STRING_LENGTH);
	if(selectNum == -1){
		for(int i = 0; i < supportedFocusModeCount; i++){
			if(supportedFocusModeBuffer[i] == DEFAULT_FOCUS_MODE_FIRST) selectNum = i;
		}
		if(selectNum == -1){
			for(int i = 0; i < supportedFocusModeCount; i++){
				if(supportedFocusModeBuffer[i] == DEFAULT_FOCUS_MODE_SECOND) selectNum = i;
			}
		}
		if(selectNum == -1){
			for(int i = 0; i < supportedFocusModeCount; i++){
				if(supportedFocusModeBuffer[i] == DEFAULT_FOCUS_MODE_THIRD) selectNum = i;
			}
		}
		if(selectNum == -1) selectNum = 0;
	}
	cameraController_.getCameraDevice()->sarSetFocusMode(supportedFocusModeBuffer[selectNum]);
	focusModeSelected_ = selectNum;
}
void SampleCore::setFlashMode(int32_t selectNum){
	std::vector<SarFlashMode> supportedFlashModeBuffer(FLASH_MODE_STRING_LENGTH);
	int supportedFlashModeCount = cameraController_.getCameraDevice()->sarGetSupportedFlashMode(&supportedFlashModeBuffer[0], FLASH_MODE_STRING_LENGTH);
	if(selectNum == -1){
		for(int i = 0; i < supportedFlashModeCount; i++){
			if(supportedFlashModeBuffer[i] == DEFAULT_FLASH_MODE){
				selectNum = i;
			}
		}
		if(selectNum == -1)selectNum = 0;
	}
	cameraController_.getCameraDevice()->sarSetFlashMode(supportedFlashModeBuffer[selectNum]);
	flashModeSelected_ = selectNum;
}
void SampleCore::setExposureMode(int32_t selectNum){
	std::vector<SarExposureMode> supportedExposureModeBuffer(EXPOSURE_MODE_STRING_LENGTH);
	int supportedExposureModeCount = cameraController_.getCameraDevice()->sarGetSupportedExposureMode(&supportedExposureModeBuffer[0], EXPOSURE_MODE_STRING_LENGTH);
	if(selectNum == -1){
		for(int i = 0; i < supportedExposureModeCount; i++){
			if(supportedExposureModeBuffer[i] == DEFAULT_EXPOSURE_MODE){
				selectNum = i;
			}
		}
		if(selectNum == -1)selectNum = 0;
	}
	cameraController_.getCameraDevice()->sarSetExposureMode(supportedExposureModeBuffer[selectNum]);
	exposureModeSelected_ = selectNum;
}
void SampleCore::setWhiteBalanceMode(int32_t selectNum){
	std::vector<SarWhiteBalanceMode> supportedWhiteBalanceModeBuffer(WHITE_BALANCE_MODE_STRING_LENGTH);
	int supportedWhiteBalanceModeCount = cameraController_.getCameraDevice()->sarGetSupportedWhiteBalanceMode(&supportedWhiteBalanceModeBuffer[0], WHITE_BALANCE_MODE_STRING_LENGTH);
	if(selectNum == -1){
		for(int i = 0; i < supportedWhiteBalanceModeCount; i++){
			if(supportedWhiteBalanceModeBuffer[i] == DEFAULT_WHITE_BALANCE_MODE){
				selectNum = i;
			}
		}
		if(selectNum == -1)selectNum = 0;
	}
	cameraController_.getCameraDevice()->sarSetWhiteBalanceMode(supportedWhiteBalanceModeBuffer[selectNum]);
	whiteBalanceModeSelected_ = selectNum;
}
void SampleCore::setSceneMode(int32_t selectNum){
	std::vector<SarSceneMode> supportedSceneModeBuffer(SCENE_MODE_STRING_LENGTH);
	int supportedSceneModeCount = cameraController_.getCameraDevice()->sarGetSupportedSceneMode(&supportedSceneModeBuffer[0], SCENE_MODE_STRING_LENGTH);
	if(selectNum == -1){
		for(int i = 0; i < supportedSceneModeCount; i++){
			if(supportedSceneModeBuffer[i] == DEFAULT_SCENE_MODE){
				selectNum = i;
			}
		}
		if(selectNum == -1)selectNum = 0;
	}
	cameraController_.getCameraDevice()->sarSetSceneMode(supportedSceneModeBuffer[selectNum]);
	sceneModeSelected_ = selectNum;
}

void SampleCore::getLoadedTarget(char charArray[][FILE_NAME_LENGTH], bool *boolArray)
{
	for(int i = 0; i < loadedTargets_.size(); i++){
		strcpy(charArray[i], "");
		strncat(charArray[i], loadedTargets_[i].fileName_, FILE_NAME_LENGTH);

		if(boolArray != NULL)
			boolArray[i] = loadedTargets_[i].isUsed_;
	}
}

/*
 * Create recognition target from file. In this sample, there is no limitation
 * for not to waste memory. Must set a limit for use in your application.
 */
void SampleCore::loadTarget(const char* directory, const char* fileName)
{
	SarFileStreamIn fileStreamIn(smart_, directory);
	targetEntry newStruct;

    CHECK_CONSTRUCT(newStruct.target_ = new SarLearnedImageTarget(smart_, &fileStreamIn));
    strncpy(newStruct.fileName_, fileName, FILE_NAME_LENGTH);
    newStruct.isUsed_ = false;

    loadedTargets_.push_back(newStruct);
	return;
}

void SampleCore::loadTargetFromAsset(const char* assetFileName){
    SarAssetStreamIn stream(smart_, assetFileName);
    CHECK_CONSTRUCT(&stream);
    targetEntry newTargetEntry;
    CHECK_CONSTRUCT(newTargetEntry.target_ = new SarLearnedImageTarget(smart_, &stream));
    strcpy(newTargetEntry.fileName_, assetFileName);
    newTargetEntry.isUsed_ = true;
    loadedTargets_.push_back(newTargetEntry);
    activeTargets_.push_back(newTargetEntry.target_);
    targetCount_++;
}

void SampleCore::changeCamera(void* nativeVideoOutput)
{
    SAR_SMARTAR_LOGD(TAG, "nativeChangeCamera ***************************************************************************");

    if (resumed_) {
        doPause();
        useFrontCameraSelected_ = !useFrontCameraSelected_;
        videoImageSizeSelected_ = -1;//Set videoImageSize default.
        doResume(nativeVideoOutput);
    }
}

/*
 * CameraController::Listener::onStillImage() callback
 */
void SampleCore::onStillImage(const SarImageHolder& imageHolder) {
	/*
	 * Must implement for capture still image
	 */
}

/*
 * CameraController::Listener::onCameraError() callback
 */
void SampleCore::onCameraError(int32_t error) {
    /*
     * Must implement for get camera image
     */
}

int SampleCore::getNearestVideoImageSize(const SarSize targetSize){
	//Select best size(const Size bestSize) or nearest one.
	std::vector<SarSize> supportedVideoImageSizeBuffer(SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);
	int supportedVideoImageSizeCount =cameraController_.getCameraDevice()
			->sarGetSupportedVideoImageSize(&supportedVideoImageSizeBuffer[0]
			        , SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);

	int curNearestIndex = 0;
	float curDiff = 0;
	for(int i = 0; i < supportedVideoImageSizeCount; i++){
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
        float diffX = static_cast<float>(fabs(static_cast<double>(supportedVideoImageSizeBuffer[i].width_ - targetSize.width_)))
        		/ std::min(supportedVideoImageSizeBuffer[i].width_, targetSize.width_);
        float diffY = static_cast<float>(fabs(static_cast<double>(supportedVideoImageSizeBuffer[i].height_ - targetSize.height_)))
        		/ std::min(supportedVideoImageSizeBuffer[i].height_, targetSize.height_);
#else
        float diffX = static_cast<float>(abs(supportedVideoImageSizeBuffer[i].width_ - targetSize.width_))
        / std::min(supportedVideoImageSizeBuffer[i].width_, targetSize.width_);
        float diffY = static_cast<float>(abs(supportedVideoImageSizeBuffer[i].height_ - targetSize.height_))
        / std::min(supportedVideoImageSizeBuffer[i].height_, targetSize.height_);
#endif
        float diff = diffX + diffY;

        bool isAlmostSame = fabsf(diff - curDiff) < 10e-6f;
        bool isCurrentBigger = supportedVideoImageSizeBuffer[i].width_ * supportedVideoImageSizeBuffer[i].height_
                < supportedVideoImageSizeBuffer[curNearestIndex].width_ * supportedVideoImageSizeBuffer[curNearestIndex].height_;
        if (i == 0 ||diff < curDiff || (isAlmostSame && isCurrentBigger)) {
        	curNearestIndex = i;
            curDiff = diff;
        }
	}
	return curNearestIndex;
}

int SampleCore::getNearestVideoImageFpsRange(const float bestFps){
	//Select best fps or nearest one.
	std::vector<SarCameraFpsRange> supportedVideoImageFpsRangeBuffer(SUPPORTED_NUM_OF_FPS_RANGE);
	int supportedVideoImageFpsRangeCount = cameraController_.getCameraDevice()
			->sarGetSupportedVideoImageFpsRange(&supportedVideoImageFpsRangeBuffer[0]
			        , SUPPORTED_NUM_OF_FPS_RANGE);

	int curNearestIndex = 0;
	float curMaxDiff = 0;
	for(int i = 0; i < supportedVideoImageFpsRangeCount; i++){
        float maxDiff = fabsf(bestFps - supportedVideoImageFpsRangeBuffer[i].max_);
        bool isAlmostSame = fabsf(maxDiff - curMaxDiff) < 10e-6f;
        bool isCurrentMinBigger = supportedVideoImageFpsRangeBuffer[curNearestIndex].min_ < supportedVideoImageFpsRangeBuffer[i].min_;
        if (i == 0 || maxDiff < curMaxDiff || (isAlmostSame && isCurrentMinBigger) ) {
        	curNearestIndex = i;
            curMaxDiff = maxDiff;
        }
	}
	return curNearestIndex;
}
