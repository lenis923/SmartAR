#pragma once

#include "../sample_common/sample_common_util.h"

//For recognizer best environments.
/*
 * Recognizing number of targets at one time. Must select appropriate number,
 * because too many recognition targets down recognition performance.
 */
const int RECOGNIZING_NUM_OF_MAX_TARGET = 2;
const int VGA_WIDTH = 640;
const int VGA_HEIGHT = 480;
const int HVGA_WIDTH = 480;
const int HVGA_HEIGHT = 320;
const sarSmartar::SarSize VGA = sarSmartar::SarSize(VGA_WIDTH, VGA_HEIGHT);//Best size of smartar for multicore is VGA(640,480).
const sarSmartar::SarSize HVGA = sarSmartar::SarSize(HVGA_WIDTH, HVGA_HEIGHT);//Best size of smartar for singlecore is HVGA(480,320).
const float BEST_FPS = 30.0f;
const sarSmartar::SarFocusMode DEFAULT_FOCUS_MODE_FIRST = SAR_FOCUS_MODE_CONTINUOUS_AUTO_VIDEO;
const sarSmartar::SarFocusMode DEFAULT_FOCUS_MODE_SECOND = SAR_FOCUS_MODE_INFINITY;
const sarSmartar::SarFocusMode DEFAULT_FOCUS_MODE_THIRD = SAR_FOCUS_MODE_MANUAL;
const sarSmartar::SarFlashMode DEFAULT_FLASH_MODE = SAR_FLASH_MODE_AUTO;
const sarSmartar::SarExposureMode DEFAULT_EXPOSURE_MODE = SAR_EXPOSURE_MODE_CONTINUOUS_AUTO;
const sarSmartar::SarWhiteBalanceMode DEFAULT_WHITE_BALANCE_MODE = SAR_WHITE_BALANCE_MODE_CONTINUOUS_AUTO;
const sarSmartar::SarSceneMode DEFAULT_SCENE_MODE = SAR_SCENE_MODE_SPORTS;

// LOCALIZE_IMPOSSIBLE_RESET_WAIT is for this sample.
// Please reset immediately in your app.
const int LOCALIZE_IMPOSSIBLE_RESET_WAIT = 200;

//Asset files
const char* const DICTIONARY_FILE_NAME1 = "smartar01.v9.dic";
const char* const DICTIONARY_FILE_NAME1_ENC = "smartar01.v9.enc.dic";
const char* const DICTIONARY_FILE_NAME2 = "smartar02.v9.dic";
const char* const LICENSE_FILE_NAME = "license.sig";

//For string return.
const int FILE_NAME_LENGTH = 256;
const int SUPPORTED_NUM_OF_IMAGE_SIZE_MAX = 32;
const int IMAGE_SIZE_STRING_LENGTH = 16;
const int IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH = 8;
const int SUPPORTED_NUM_OF_FPS_RANGE = 32;
const int FPS_RANGE_STRING_LENGTH = 32;
const int FPS_RANGE_MAX_OR_MIN_STRING_LENGTH = 16;

const char* const SceneMappingInitModeArray[5] = {
    "TARGET",
    "HFG",
    "VFG",
    "SFM",
    "DRY_RUN",
};
const char* const TargetTrackingStateArray[3] = {
    "IDLE",
    "SEARCH",
    "TRACKING"
};
const char* const SceneMappingStateArray[5] = {
    "IDLE",
    "SEARCH",
    "TRACKING",
    "LOCALIZE",
    "LOCALIZE_IMPOSSIBLE"
};

//For string return of camera device status.
const int SUPPORTED_NUM_OF_FOCUS_MODE = 7;
const int FOCUS_MODE_STRING_LENGTH = 35;
const char* const FocusModeArray[SUPPORTED_NUM_OF_FOCUS_MODE] = {
    "MANUAL",
    "CONTINUOUS_AUTO_PICTURE",
    "CONTINUOUS_AUTO_VIDEO",
    "EDOF",
    "FIXED",
    "INFINITY",
    "MACRO"
};
const int SUPPORTED_NUM_OF_FLASH_MODE = 5;
const int FLASH_MODE_STRING_LENGTH = 19;
const char* const FlashModeArray[SUPPORTED_NUM_OF_FLASH_MODE] = {
    "AUTO",
    "OFF",
    "ON",
    "RED_EYE",
    "TORCH"
};
const int SUPPORTED_NUM_OF_EXPOSURE_MODE = 2;
const int EXPOSURE_MODE_STRING_LENGTH = 30;
const char* const ExposureModeArray[SUPPORTED_NUM_OF_EXPOSURE_MODE] = {
    "MANUAL",
    "CONTINUOUS_AUTO"
};
const int SUPPORTED_NUM_OF_WHITE_BALANCE_MODE = 9;
const int WHITE_BALANCE_MODE_STRING_LENGTH = 36;
const char* const WhiteBalanceModeArray[SUPPORTED_NUM_OF_WHITE_BALANCE_MODE] = {
    "CONTINUOUS_AUTO",
    "CLOUDY_DAYLIGHT",
    "DAYLIGHT",
    "FLUORESCENT",
    "INCANDESCENT",
    "SHADE",
    "TWILIGHT",
    "WARM_FLUORESCENT",
    "MANUAL"
};
const int SUPPORTED_NUM_OF_SCENE_MODE = 16;
const int SCENE_MODE_STRING_LENGTH = 26;
const char* const SceneModeArray[SUPPORTED_NUM_OF_SCENE_MODE] = {
    "ACTION",
    "AUTO",
    "BARCODE",
    "BEACH",
    "CANDLELIGHT",
    "FIREWORKS",
    "LANDSCAPE",
    "NIGHT",
    "NIGHT_PORTRAIT",
    "PARTY",
    "PORTRAIT",
    "SNOW",
    "SPORTS",
    "STEADYPHOTO",
    "SUNSET",
    "THEATRE"
};

using namespace sarSmartar;

class SampleCore : public CameraController::Listener{
public:
    SampleCore(void* nativeContext, void* nativeEnv, int32_t recogMode, int32_t initMode);
    ~SampleCore();
    void doResume(void* nativeVideoOutput);
    void doPause();
    void doSurfaceCreated();
    void doSurfaceChanged(int32_t width, int32_t height);
    void doDestroySurface();

    void saveSceneMap(const char* filePath);
    void loadSceneMap(const char* filePath);
    void setLearnedImageTarget(bool *isUsedArray);
    void getLoadedTarget(char charArray[][FILE_NAME_LENGTH], bool *boolArray);
    void loadTarget(const char* directory, const char* fileName);
    void loadTargetFromAsset(const char* assetFileName);
    void changeCamera(void* nativeVideoOutput);
    void forceLocalize();
    void resetRecognizer();
    void setSearchPolicy(int32_t searchPolicy);

    int getNumOfSupportedVideoImageSize();
    int getNumOfSupportedVideoImageFpsRange();
    int getNumOfSupportedFocusMode();
    int getNumOfSupportedFlashMode();
    int getNumOfSupportedExposureMode();
    int getNumOfSupportedWhiteBalanceMode();
    int getNumOfSupportedSceneMode();
    void getSupportedVideoImageSize(char charArray[][IMAGE_SIZE_STRING_LENGTH], int arrayLenght);
    void getSupportedVideoImageFpsRange(char charArray[][FPS_RANGE_STRING_LENGTH], int arrayLenght);
    void getSupportedFocusMode(char charArray[][FOCUS_MODE_STRING_LENGTH], int arrayLength);
    void getSupportedFlashMode(char charArray[][FLASH_MODE_STRING_LENGTH], int arrayLength);
    void getSupportedExposureMode(char charArray[][EXPOSURE_MODE_STRING_LENGTH], int arrayLength);
    void getSupportedWhiteBalanceMode(char charArray[][WHITE_BALANCE_MODE_STRING_LENGTH], int arrayLength);
    void getSupportedSceneMode(char charArray[][SCENE_MODE_STRING_LENGTH], int arrayLength);
    void setCameraVideoImageSize(int32_t selectNum);
    void setVideoImageFpsRange(int32_t selectNum);
    void setFocusMode(int32_t selectNum);
    void setFlashMode(int32_t selectNum);
    void setExposureMode(int32_t selectNum);
    void setWhiteBalanceMode(int32_t selectNum);
    void setSceneMode(int32_t selectNum);

    const char* getInitMode();
    const char* getState();

    int getInitModeNum(){
    	return initMode_;
    }

    int getNumOfLoadedTarget(){
    	return static_cast<int>(loadedTargets_.size());
    }

    int getCameraFrameCount() {
        int count = cameraFrameCount_;
        cameraFrameCount_ = 0;
        return count;
    }

    int32_t getRecogCount(int32_t index) {
        return workerThreadController_.getRecogCount(index);
    }

    uint64_t getRecogTime(int32_t index) {
        return workerThreadController_.getRecogTime(index);
    }

    bool isRecogModeSceneMapping() {
        return recogMode_;
    }

    bool isSearchPolicyPresicive() {
        return searchPolicy_;
    }

    bool isDenseMapModeSemiDense() {
        return denseMapMode_;
    }

    int getTrackedLandmarkCount() {
        return trackedLandmarkCount_;
    }
    int getLostLandmarkCount() {
        return lostLandmarkCount_;
    }
    int getSuspendedLandmarkCount() {
        return suspendedLandmarkCount_;
    }
    int getMaskedLandmarkCount() {
        return maskedLandmarkCount_;
    }

    int getVideoImageSizeSelected() {
    	return videoImageSizeSelected_;
    }
    int getVideoImageFpsRangeSelected() {
    	return videoImageFpsRangeSelected_;
    }

    bool isUseWorkerThread(){
    	return useWorkerThread_;
    }
    void setUseWorkerThreadFlag(bool flag){
    	useWorkerThread_ = flag;
    }
    void setFixSceneMapFlag (bool flag){
    	workerThreadController_.stop();
    	CHECK_ERR(recognizer_->sarFixSceneMap(flag));
    	workerThreadController_.start();
    	fixSceneMap_ = flag;
    }

    int getFocusModeSelected() {
    	return focusModeSelected_;
    }
    int getFlashModeSelected() {
    	return flashModeSelected_;
    }
    int getExposureModeSelected() {
    	return exposureModeSelected_;
    }
    int getWhiteBalanceModeSelected() {
    	return whiteBalanceModeSelected_;
    }
    int getSceneModeSelected() {
    	return sceneModeSelected_;
    }

    void setLandMarkBarLocation(int x, int y){
    	contentDrawer_.setLandMarkBarLocation(x, y);
    }

    void setShowLandMarkFlag(bool flag){
    	showLandmark_ = flag;
    }

    void setShowLandMarkbarFlag(bool flag){
    	showLandmarkBar_ = flag;
    }

    void stopSensorDevice(){
    	cameraController_.getSensorDevice()->sarStop();
    	useSensorDevice_ = false;
    }

    void startSensorDevice(){
    	cameraController_.getSensorDevice()->sarStart();
    	useSensorDevice_ = true;
    }

    bool isUseFrontCamera(){
        return useFrontCameraSelected_;
    }

    bool isUseSensorDevice(){
        return useSensorDevice_;
    }

    bool isFixSceneMap(){
        return fixSceneMap_;
    }

    void initializeCameraSettings(int videoImageSizeSelected
    		, int videoImageFpsRangeSelected
    		, int focusModeSelected
    		, int flashModeSelected
    		, int exposureModeSelected
    		, int whiteBalanceModeSelected
    		, int sceneModeSelected
    		, int useFrontCameraSelected
    		, int useSensorDevice){
        videoImageSizeSelected_ = videoImageSizeSelected;
        videoImageFpsRangeSelected_ = videoImageFpsRangeSelected;
        focusModeSelected_ = focusModeSelected;
        flashModeSelected_ = flashModeSelected;
        exposureModeSelected_ = exposureModeSelected;
        whiteBalanceModeSelected_ = whiteBalanceModeSelected;
        sceneModeSelected_ = sceneModeSelected;
        useFrontCameraSelected_ = (useFrontCameraSelected == 0) ? true : false;
        useSensorDevice_ = (useSensorDevice == 0) ? false : true;
    }

protected:
    // Override CameraController::Listener::onImage()
    virtual void onImage(const SarImageHolder& imageHolder, uint64_t timestamp, int32_t numSensorStates, SarSensorState* sensorStates);

    // Override CameraController::Listener::onStillImage()
    virtual void onStillImage(const SarImageHolder& imageHolder);

    // Override CameraCotroller::Listener::onCameraError()
    virtual void onCameraError(int32_t error);

    int getNearestVideoImageSize(const sarSmartar::SarSize targetSize);
    int getNearestVideoImageFpsRange(const float bestFps);

protected:
    // Variables for core components
    SarSmart* smart_;
    SarRecognizer* recognizer_;

    int32_t currentTargetIndex_;
    std::vector<SarTarget*> activeTargets_;
    struct targetEntry{
    	SarLearnedImageTarget* target_;
    	char fileName_[FILE_NAME_LENGTH];
        bool isUsed_;
    };
    std::vector<targetEntry> loadedTargets_;
    int targetCount_;
    SarSceneMapTarget* sceneMapTarget_;
    std::vector<SarRecognitionResult> result_;

    WorkerThreadController workerThreadController_;
    CameraController cameraController_;

    // Variables for drawers
    BackgroundDrawer backgroundDrawer_;
    TestContentDrawer contentDrawer_;
    sarSmartar::SarImageQueue* videoImageQueue_;
    std::vector<SarLandmark> landmarkBuffer_;
    std::vector<SarNodePoint> nodePointBuffer_;
    std::vector<SarInitPoint> initPointBuffer_;

    // Misc variables
    int32_t screenWidth_;
    int32_t screenHeight_;
    bool resumed_;
    bool surfaceCreated_;
    bool useWorkerThread_;
    int cameraFrameCount_;
    int drawFrameCount_;
    int recognizeCount_;
    int state_;
    int recogMode_;
    int initMode_;
    int searchPolicy_;
    int denseMapMode_;
    int trackedLandmarkCount_;
    int lostLandmarkCount_;
    int suspendedLandmarkCount_;
    int maskedLandmarkCount_;

    int videoImageSizeSelected_;
    sarSmartar::SarSize selectedSize_;
    int videoImageFpsRangeSelected_;
    int focusModeSelected_;
    int flashModeSelected_;
    int exposureModeSelected_;
    int whiteBalanceModeSelected_;
    int sceneModeSelected_;

    bool showLandmark_;
    bool showLandmarkBar_;
    bool useSensorDevice_;
    bool fixSceneMap_;
    bool useFrontCameraSelected_;

    int localizeImpossibleCounter_;
};

