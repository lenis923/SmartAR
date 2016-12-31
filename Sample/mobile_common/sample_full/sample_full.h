#pragma once

#include "../sample_common/sample_common.h"

//Triangulate Masks.
const unsigned int NUM_OF_TRIANGULATE_MASK = 4;//Use 4 triangulate masks
const SarVector2 triangulateMasksArray[NUM_OF_TRIANGULATE_MASK][3] = {
	{SarVector2(0.0f, 0.0f), SarVector2(0.5f, 0.0f), SarVector2(0.0f, 0.5f)},
	{SarVector2(0.5f, 0.0f), SarVector2(1.0f, 0.0f), SarVector2(1.0f, 0.5f)},
	{SarVector2(0.0f, 0.5f), SarVector2(0.0f, 1.0f), SarVector2(0.5f, 1.0f)},
	{SarVector2(0.5f, 1.0f), SarVector2(1.0f, 0.5f), SarVector2(1.0f, 1.0f)},
};

using namespace sarSmartar;

class SampleCoreImpl : public SampleCore{
public:
    SampleCoreImpl(void* nativeContext, void* nativeEnv, int32_t recogMode, int32_t initMode);
    void doResume(void* nativeVideoOutput);
    void doDrawFrame();
    void setDenseMapMode(int32_t denseMapMode);
    void captureStillImage();
    int getNumOfSupportedStillImageSize();
    void getSupportedStillImageSize(char charArray[][IMAGE_SIZE_STRING_LENGTH], int arrayLenght);
    void setCameraStillImageSize(int32_t selectNum);
    bool isConstructorFailed() const { return smart_->sarIsConstructorFailed() || recognizer_->sarIsConstructorFailed(); }
    int32_t getSmartInitResultCode() const { return smart_->sarGetInitResultCode(); }

    int getStillImageSizeSelected() {
    	return stillImageSizeSelected_;
    }

    void setUseTriangulateMasksFlag(bool flag){
    	useTriangulateMasks_ = flag;
    }

    void setRemoveLostLandmarksFlag(bool flag){
    	removeLostLandmarks_ = flag;
    }

    bool isRemoveLostLandmarks(){
        return removeLostLandmarks_;
    }

    bool isUseTriangulateMasks(){
        return useTriangulateMasks_;
    }

    void initializeCameraSettings(int videoImageSizeSelected
    		, int stillImageSizeSelected
    		, int videoImageFpsRangeSelected
    		, int focusModeSelected
    		, int flashModeSelected
    		, int exposureModeSelected
    		, int whiteBalanceModeSelected
    		, int sceneModeSelected
    		, int useFrontCameraSelected
    		, int useSensorDevice){
        videoImageSizeSelected_ = videoImageSizeSelected;
        stillImageSizeSelected_ = stillImageSizeSelected;
        videoImageFpsRangeSelected_ = videoImageFpsRangeSelected;
        focusModeSelected_ = focusModeSelected;
        flashModeSelected_ = flashModeSelected;
        exposureModeSelected_ = exposureModeSelected;
        whiteBalanceModeSelected_ = whiteBalanceModeSelected;
        sceneModeSelected_ = sceneModeSelected;
        useFrontCameraSelected_ = (useFrontCameraSelected == 0) ? true : false;
        useSensorDevice_ = (useSensorDevice == 0) ? false : true;
    }

public:
    //Call from jni methods.
    void setMediaScannerFunction(void (*func)(void)){
    	mediaScannerFunc = func;
    }
    void setStillImageFilePath(const char* filePath){
    	strncpy(stillImageFilePath_, filePath, FILE_NAME_LENGTH);
    };

private:
    // Override CameraController::Listener::onImage()
    virtual void onImage(const SarImageHolder& imageHolder, uint64_t timestamp, int32_t numSensorStates, SarSensorState* sensorStates);

    // Override CameraController::Listener::onStillImage()
    virtual void onStillImage(const SarImageHolder& imageHolder);

    // Override CameraController::Listener::onCameraError()
    virtual void onCameraError(int32_t error);

    int getBiggestStillImageSize();


private:

    bool useTriangulateMasks_;
    bool removeLostLandmarks_;

    std::vector<SarTriangle2> triangulateMasks_;


    int stillImageSizeSelected_;


    void (*mediaScannerFunc)(void);//Android function set in JNI.
    char stillImageFilePath_[FILE_NAME_LENGTH];


};

