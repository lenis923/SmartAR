#pragma once

#include "../sample_common/sample_common.h"

class SampleCoreImpl : public SampleCore{
public:
	SampleCoreImpl(void* nativeContext, void* nativeEnv, int32_t recogMode, int32_t initMode);
    void doDrawFrame();
    bool isConstructorFailed() const { return smart_->sarIsConstructorFailed() || recognizer_->sarIsConstructorFailed(); }
    int32_t getSmartInitResultCode() const { return smart_->sarGetInitResultCode(); }

private:
    // Override CameraController::Listener::onImage()
    virtual void onImage(const SarImageHolder& imageHolder, uint64_t timestamp, int32_t numSensorStates, SarSensorState* sensorStates);

    // Override CameraController::Listener::onCameraError()
    virtual void onCameraError(int32_t error);
};

