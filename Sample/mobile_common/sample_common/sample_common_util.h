#pragma once

#include "SarCommon.h"
#include "SarSmart.h"
#include "SarCameraDevice.h"
#include "SarSensorDevice.h"
#include "SarScreenDevice.h"
#include "SarRecognizer.h"
#include "SarCameraImageDrawer.h"
#include "SarUtility.h"

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

#include <sys/types.h>
#include <sys/time.h>
#include <inttypes.h>
#include <vector>
#include <pthread.h> // include pthread.h after vector to avoid _STLP_OUTERMOST_HEADER_ID problem.

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#include <mach/mach_time.h>
#include <mach/kern_return.h>
#endif

#if defined (__ANDROID__) || defined(ANDROID)
#   define TARGET_SMARTAR_ANDROID
#endif

#if defined (_IOS_SDK_)
#   define TARGET_SMARTAR_IOS
#endif

using namespace sarSmartar;

const int CAMERA_OPEN_TRY_COUNT = 5;
const int CAMERA_OPEN_INTERVAL = 500; // msec

#define TAG "SmartARsample"

#define CHECK_ERR(expr) do { \
    int32_t _smartarResult = (expr); \
    if (_smartarResult < 0) { \
        SAR_SMARTAR_LOGE(TAG, "SmartAR error %d at %s : %s %d ***************************************************************************", _smartarResult, #expr, __FILE__, __LINE__); \
    } \
} while (false)

#if defined(TARGET_SMARTAR_ANDROID) || defined(TARGET_SMARTAR_IOS)
#define CHECK_CONSTRUCT(obj) do { \
    if ((obj)->sarIsConstructorFailed()) { \
        SAR_SMARTAR_LOGE(TAG, "SmartAR constructor failed at %s : %s %d ***************************************************************************", #obj, __FILE__, __LINE__); \
    } \
} while (false)
#else
#define CHECK_CONSTRUCT(obj) do { \
    if ((obj)->sarIsConstructorFailed()) { \
        SAR_SMARTAR_LOGE(TAG, "SmartAR constructor failed at %s : %s %d ***************************************************************************", #obj, __FILE__, __LINE__); \
        exit(1); \
    } \
} while (false)
#endif

inline uint64_t getCurrentMillis() {
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
    static const struct TimeSpec {
        uint32_t numer_;
        uint32_t denom_;

        TimeSpec() {
            mach_timebase_info_data_t tinfo;
            /*kern_return_t kerror =*/ mach_timebase_info(&tinfo);
            numer_ = tinfo.numer;
            denom_ = tinfo.denom * 1000000;
        }
    } timespec;

    return mach_absolute_time() * timespec.numer_ / timespec.denom_;
#else
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return static_cast<uint64_t>(t.tv_sec) * 1000 + static_cast<uint64_t>(t.tv_nsec) / 1000000;
#endif
}

//--------------------------------------------------------------------------------------------------

class CameraController : private SarCameraImageListener, private SarSensorListener {
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void onImage(const SarImageHolder& imageHolder, uint64_t timestamp, int32_t numSensorStates, SarSensorState* sensorStates) = 0;
        virtual void onStillImage(const SarImageHolder& imageHolder) = 0;
        virtual void onCameraError(int32_t error) = 0;
    };

    CameraController(Listener* listener);

    ~CameraController();

    bool open(SarSmart* smart, void* nativeVideoOutput);

    void close();

    void start();

    void stop();

    SarCameraDevice* getCameraDevice() const {
        return cameraDevice_;
    }

    SarSensorDevice* getSensorDevice() const {
        return sensorDevice_;
    }

    bool getUseFrontCamera() const {
        return useFrontCamera_;
    }

    void setUseFrontCamera(bool useFrontCamera) {
        useFrontCamera_ = useFrontCamera;
    }

private:
    // Override CameraImageListener::onImage()
    virtual void sarOnImage(const SarImageHolder& imageHolder, uint64_t timestamp);

    // Override SensorListener::onSensor()
    virtual void sarOnSensor(const SarSensorState& state);

private:
    Listener* listener_;

    SarCameraDevice* cameraDevice_;
    bool useFrontCamera_;

    SarSensorDevice* sensorDevice_;
    std::vector<SarSensorState> sensorStates_;
    std::vector<SarSensorState> sensorStatesWork_;
    bool started_;
    pthread_mutex_t sensorMutex_;

    class MyStillImageListener : public SarCameraImageListener {
    public:
        virtual void sarOnImage(const SarImageHolder& imageHolder, uint64_t timestamp) {
            owner_->listener_->onStillImage(imageHolder);
        }

        CameraController* owner_;
    } stillImageListener_;

    class MyCameraErrorListener : public SarCameraErrorListener {
    public:
        virtual void sarOnError(int32_t error) {
            owner_->listener_->onCameraError(error);
        }

        CameraController* owner_;
    } errorListener_;
};

//--------------------------------------------------------------------------------------------------

class WorkerThreadController : private SarWorkDispatchedListener {
public:
    WorkerThreadController();

    ~WorkerThreadController();

    void init(SarRecognizer* recognizer, int32_t numThreads);

    void dispose();

    void start();

    void stop();

    int32_t getRecogCount(int32_t index) {
        int count = -1;
        if (index < recogCount_.size()) {
            count = recogCount_[index];
            recogCount_[index] = 0;
        }
        return count;
    }

    uint64_t getRecogTime(int32_t index) {
        uint64_t time = -1;
        if (index < recogTime_.size()) {
            time = recogTime_[index];
            recogTime_[index] = 0;
        }
        return time;
    }

private:
    // Override WorkDispatchedListener::onWorkDispatched()
    virtual void sarOnWorkDispatched();

    static void* workerRoutine(void *param);
    void* workerRoutineImpl();

private:
    SarRecognizer* recognizer_;
    std::vector<pthread_t> threads_;
    pthread_mutex_t mutex_;
    pthread_cond_t condAwakeWorker_;
    pthread_cond_t condWorkerStopped_;
    bool done_;
    bool started_;
    int32_t numWait_;
    std::vector<int32_t> recogCount_;
    std::vector<uint64_t> recogTime_;
};

//--------------------------------------------------------------------------------------------------

class BackgroundDrawer {
public:
    BackgroundDrawer()
    : screenDevice_(NULL), cameraImageDrawer_(NULL), drawWholeCameraImage_(false),
      fovy_(0.0f), stillImageFovy_(0.0f), calibrated_(false), getfromapi_(false), lastScreenRotation_(SAR_ROTATION_0), cameraChanged_(false) {}

    ~BackgroundDrawer() {}

    void init(SarSmart* smart);

    void dispose();

    void setCameraParameter(SarFacing cameraFacing, SarRotation cameraRotation);

    void startDraw();

    void stopDraw();

    void draw(SarImage& videoImage, int32_t screenWidth, int32_t screenHeight, const SarCameraDevice& cameraDevice);

    void draw(SarImage& videoImage, int32_t screenWidth, int32_t screenHeight);

    void adjustPose(const SarVector3& fromPosition, const SarQuaternion& fromRotation, SarVector3* toPosition, SarQuaternion* toRotation) const {
        cameraImageTransform_.sarAdjustPose(fromPosition, fromRotation, toPosition, toRotation);
    }

    float getFovy() const {
        return fovy_;
    }

    float getStillImageFovy() const {
        return stillImageFovy_;
    }

    bool getCalibrated() const {
        return calibrated_;
    }

    bool isGetFromAPI() const {
    	return getfromapi_;
    }

    const SarMatrix44& getInitPointMatrix() const {
        return initPointMatrix_;
    }

    void setDrawWholeCameraImage(bool drawWholeCameraImage) {
        drawWholeCameraImage_ = drawWholeCameraImage;
    }

private:
    SarScreenDevice* screenDevice_;
    SarCameraImageDrawer* cameraImageDrawer_;
    SarCameraImageTransform cameraImageTransform_;
    bool drawWholeCameraImage_;

    float fovy_;
    float stillImageFovy_;
    bool calibrated_;
    bool getfromapi_;
    SarMatrix44 initPointMatrix_;

    SarRotation lastScreenRotation_;
    SarSize lastVideoImageSize_;
    SarSize lastScreenSize_;
    bool cameraChanged_;
};

//--------------------------------------------------------------------------------------------------

class TestContentDrawer {
public:
    TestContentDrawer()
    : lastTarget_(NULL) {}

    ~TestContentDrawer() {}

    void startDraw();

    void stopDraw();

    void draw(int32_t screenWidth, int32_t screenHeight, float fovy, bool calibrated, bool getfromapi,
            const SarMatrix44& initPointMatrix, const SarRecognitionResult* results, int32_t numResults, bool isDrawCube, bool isDrawLandMark);

    void setLandMarkBarLocation(int x, int y);

    void drawLandmarkBar(int32_t screenWidth, int32_t screenHeight, int trackedLandmarkCount
    		, int lostLandmarkCount, int suspendedLandmarkCount, int maskedLandmarkCount, int maxLandMarkCount);

    void drawTriangulateMasks();

private:
    SarRectDrawer rectDrawer_;
    SarCubeDrawer cubeDrawer_;
    SarLandmarkDrawer landmarkDrawer_;
    SarVector2 targetSize_;
    const SarTarget* lastTarget_;
    SarVector2 landMarkBarLocation_;
};
