#include <math.h>
#include <algorithm>

#include "sample_common_util.h"
#include "SarUtility.h"

//--------------------------------------------------------------------------------------------------

CameraController::CameraController(Listener* listener)
: listener_(listener), cameraDevice_(NULL), useFrontCamera_(false),
sensorDevice_(NULL), started_(false)
{
    pthread_mutex_init(&sensorMutex_, NULL);
    stillImageListener_.owner_ = this;
    errorListener_.owner_ = this;
}

CameraController::~CameraController() {
    pthread_mutex_destroy(&sensorMutex_);
}

bool CameraController::open(SarSmart* smart, void* nativeVideoOutput) {
    // Create camera device.
    for (int i = 0; i < CAMERA_OPEN_TRY_COUNT && cameraDevice_ == NULL; ++i) {
        if (i != 0) {
            timespec t = {0, CAMERA_OPEN_INTERVAL * 1000 * 1000};
            nanosleep(&t, NULL);
        }

        if (useFrontCamera_) {
            int32_t frontCameraId = SarCameraDevice::SAR_INVALID_CAMERA_ID;
            CHECK_ERR(SarCameraDevice::sarGetDefaultCameraId(smart, SAR_FACING_FRONT, &frontCameraId));
            if (frontCameraId != SarCameraDevice::SAR_INVALID_CAMERA_ID) {
                cameraDevice_ = new SarCameraDevice(smart, frontCameraId);
            } else {
                cameraDevice_ = new SarCameraDevice(smart);
            }
        } else {
            cameraDevice_ = new SarCameraDevice(smart);
        }

        if (cameraDevice_->sarIsConstructorFailed()) {
            delete cameraDevice_;
            cameraDevice_ = NULL;
            SAR_SMARTAR_LOGW(TAG, "camera open failed - %d", i);
        }
    }

    if (cameraDevice_ == NULL) {
        return false;
    }

    CHECK_ERR(cameraDevice_->sarSetNativeVideoOutput(nativeVideoOutput));
    CHECK_ERR(cameraDevice_->sarSetVideoImageListener(this));
    CHECK_ERR(cameraDevice_->sarSetStillImageListener(&stillImageListener_));
    CHECK_ERR(cameraDevice_->sarSetErrorListener(&errorListener_));


    // Create sensor device.
    CHECK_CONSTRUCT(sensorDevice_ = new SarSensorDevice(smart));
    CHECK_ERR(sensorDevice_->sarSetSensorListener(this));

    return true;
}

void CameraController::close() {
    delete sensorDevice_;
    sensorDevice_ = NULL;

    delete cameraDevice_;
    cameraDevice_ = NULL;
}

void CameraController::start() {
    CHECK_ERR(cameraDevice_->sarStart());

    CHECK_ERR(sensorDevice_->sarStart());

    pthread_mutex_lock(&sensorMutex_);
    started_ = true;
    pthread_mutex_unlock(&sensorMutex_);
}

void CameraController::stop() {
    pthread_mutex_lock(&sensorMutex_);
    started_ = false;
    sensorStates_.clear();
    pthread_mutex_unlock(&sensorMutex_);

    CHECK_ERR(sensorDevice_->sarStop());

    CHECK_ERR(cameraDevice_->sarStop());
}

// Override CameraImageListener::onImage()
void CameraController::sarOnImage(const SarImageHolder& imageHolder, uint64_t timestamp)
{
    // Obtain sensor data
    pthread_mutex_lock(&sensorMutex_);
    sensorStatesWork_.clear();
    std::swap(sensorStatesWork_, sensorStates_);
    pthread_mutex_unlock(&sensorMutex_);

    // Call back
    int32_t numSensorStates = static_cast<int32_t>(sensorStatesWork_.size());
    SarSensorState* sensorStates = (sensorStatesWork_.size() > 0) ? &sensorStatesWork_.front() : NULL;
    listener_->onImage(imageHolder, timestamp, numSensorStates, sensorStates);
}

// Override SensorListener::onSensor()
void CameraController::sarOnSensor(const SarSensorState& state)
{
    pthread_mutex_lock(&sensorMutex_);
    if (started_) {
        sensorStates_.push_back(state);
    }
    pthread_mutex_unlock(&sensorMutex_);
}

//--------------------------------------------------------------------------------------------------

WorkerThreadController::WorkerThreadController()
: recognizer_(NULL), done_(false), started_(false), numWait_(0)
{
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&condAwakeWorker_, NULL);
    pthread_cond_init(&condWorkerStopped_, NULL);
}

WorkerThreadController::~WorkerThreadController() {
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&condAwakeWorker_);
    pthread_cond_destroy(&condWorkerStopped_);
}

void WorkerThreadController::init(SarRecognizer* recognizer, int32_t numThreads) {
    recognizer_ = recognizer;

    CHECK_ERR(recognizer->sarSetWorkDispatchedListener(this));

    pthread_mutex_lock(&mutex_);
    done_ = false;
    pthread_mutex_unlock(&mutex_);

    threads_.resize(numThreads);
    pthread_t* end = &threads_.front() + threads_.size();
    for (pthread_t* thread = &threads_.front(); thread != end; ++thread) {
        pthread_create(thread , NULL , workerRoutine , this);
    }

    recogCount_.resize(numThreads);
    recogTime_.resize(numThreads);
}

void WorkerThreadController::dispose() {
    pthread_mutex_lock(&mutex_);
    done_ = true;
    pthread_cond_broadcast(&condAwakeWorker_);
    pthread_mutex_unlock(&mutex_);

    pthread_t* end = &threads_.front() + threads_.size();
    for (pthread_t* thread = &threads_.front(); thread != end; ++thread) {
        pthread_join(*thread , NULL);
    }

    recognizer_ = NULL;
}

void WorkerThreadController::start() {
    pthread_mutex_lock(&mutex_);
    started_ = true;
    pthread_mutex_unlock(&mutex_);
}

void WorkerThreadController::stop() {
    pthread_mutex_lock(&mutex_);
    started_ = false;
    while (!done_ && numWait_ < threads_.size()) {
        pthread_cond_wait(&condWorkerStopped_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
}

void WorkerThreadController::sarOnWorkDispatched()
{
    pthread_mutex_lock(&mutex_);
    pthread_cond_broadcast(&condAwakeWorker_);
    pthread_mutex_unlock(&mutex_);
}

void* WorkerThreadController::workerRoutine(void *param)
{
    return static_cast<WorkerThreadController*>(param)->workerRoutineImpl();
}

void* WorkerThreadController::workerRoutineImpl()
{
    const pthread_t* threads = &threads_.front();
    int32_t thread_index = static_cast<int32_t>(std::find(threads, threads + threads_.size(), pthread_self()) - threads);
    SAR_SMARTAR_LOGD(TAG, "worker thread %d begin ***************************************************************************", thread_index);

    for (;;) {
        // Wait when there is nothing to do
        pthread_mutex_lock(&mutex_);
        if (!done_) { //TODO: check error code
            ++numWait_;
            pthread_cond_broadcast(&condWorkerStopped_);
            pthread_cond_wait(&condAwakeWorker_, &mutex_);
            --numWait_;
        }
        bool done = done_;
        bool started = started_;
        pthread_mutex_unlock(&mutex_);

        // Exit if application is terminating
        if (done) {
            break;
        }

        // Execute recognition
        if (started) {
            uint64_t startTime = getCurrentMillis();
            recognizer_->sarRunWorker();
            uint64_t endTime = getCurrentMillis();
            recogTime_[thread_index] += endTime - startTime;
            ++recogCount_[thread_index];
        }
    }

    SAR_SMARTAR_LOGD(TAG, "worker thread %d end ***************************************************************************", thread_index);

    return NULL;
}

//--------------------------------------------------------------------------------------------------

void BackgroundDrawer::init(SarSmart* smart) {
    // Initialize screenDevice
    CHECK_CONSTRUCT(screenDevice_ = new SarScreenDevice(smart));

    // Initialize camera drawer
    CHECK_CONSTRUCT(cameraImageDrawer_ = new SarCameraImageDrawer(smart));
}

void BackgroundDrawer::dispose() {
    // Release camera drawer
    delete cameraImageDrawer_;
    cameraImageDrawer_ = NULL;

    // Release screenDevice
    delete screenDevice_;
    screenDevice_ = NULL;
}

void BackgroundDrawer::setCameraParameter(SarFacing cameraFacing, SarRotation cameraRotation) {
    cameraImageTransform_.sarSetCameraRotation(cameraRotation);
    cameraImageTransform_.sarSetCameraFacing(cameraFacing);
    cameraChanged_ = true;
}

void BackgroundDrawer::startDraw() {
    CHECK_ERR(cameraImageDrawer_->sarStart());
}

void BackgroundDrawer::stopDraw() {
    CHECK_ERR(cameraImageDrawer_->sarStop());
}

void BackgroundDrawer::draw(SarImage& videoImage, int32_t screenWidth, int32_t screenHeight, const SarCameraDevice& cameraDevice) {
    // Check geometry of screen and camera image
    SarRotation screenRotation;
    CHECK_ERR(screenDevice_->sarGetRotation(&screenRotation));
    bool screenChanged = screenRotation != lastScreenRotation_
            || videoImage.getWidth() != lastVideoImageSize_.width_ || videoImage.getHeight() != lastVideoImageSize_.height_
            || screenWidth != lastScreenSize_.width_ || screenHeight != lastScreenSize_.height_
            || cameraChanged_;
    if (screenChanged) {
        lastScreenRotation_ = screenRotation;
        lastVideoImageSize_.width_ = videoImage.getWidth();
        lastVideoImageSize_.height_ = videoImage.getHeight();
        lastScreenSize_.width_ = screenWidth;
        lastScreenSize_.height_ = screenHeight;
        cameraChanged_ = false;

        cameraImageTransform_.sarSetScreenRotation(screenRotation);
        CHECK_ERR(cameraImageDrawer_->sarSetRotation(cameraImageTransform_.sarGetCameraImageRotation()));
        CHECK_ERR(cameraImageDrawer_->sarSetFlipX(cameraImageTransform_.sarGetCameraImageFlipX()));
        CHECK_ERR(cameraImageDrawer_->sarSetFlipY(cameraImageTransform_.sarGetCameraImageFlipY()));
    }

    int32_t rotImgWidth = videoImage.getWidth();
    int32_t rotImgHeight = videoImage.getHeight();
    int32_t rotScrWidth = screenWidth;
    int32_t rotScrHeight = screenHeight;
    if (cameraImageTransform_.sarGetCameraImageRotation() == SAR_ROTATION_90
        || cameraImageTransform_.sarGetCameraImageRotation() == SAR_ROTATION_270) {
        std::swap(rotImgWidth, rotImgHeight);
        std::swap(rotScrWidth, rotScrHeight);
    }

    float rangeX;
    float rangeY;
    if (drawWholeCameraImage_) {
        rangeX = std::min(1.0f, screenHeight * rotImgWidth  / static_cast<float>(rotImgHeight * screenWidth));
        rangeY = std::min(1.0f, screenWidth  * rotImgHeight / static_cast<float>(rotImgWidth  * screenHeight));
    }

    // Calc fov
    if (screenChanged) {
        float drawHeight;
        if (drawWholeCameraImage_) {
            drawHeight = rotImgHeight / rangeY;
        } else {
            drawHeight = std::min(rotImgHeight, rotImgWidth * screenHeight / screenWidth);
        }
        float drawHeightRatio = drawHeight / videoImage.getHeight();

        CHECK_ERR(cameraDevice.sarGetFovY(&fovy_, drawHeightRatio, &calibrated_));
        CHECK_ERR(cameraDevice.sarGetFovY(&stillImageFovy_, 1.0f, NULL, true));

        if (!calibrated_) {
        	float fovyTemp = 0.0f;
       		if (cameraDevice.sarGetFovY(&fovyTemp, screenWidth, screenHeight, &getfromapi_, &calibrated_) == SAR_OK
       				&& getfromapi_) {
       			stillImageFovy_ = fovy_ = fovyTemp;
                SAR_SMARTAR_LOGD(TAG, "@@@@@ sarGetFovY() = %f, getfromapi_ = %s, calibrated_ = %s, (%d, %d)",
                		fovy_, (getfromapi_ ? "true" : "false"), (calibrated_ ? "true" : "false"), screenWidth, screenHeight);
       		}
        }
    }

    // Draw camera image
    SarRect imageRect;
    if (drawWholeCameraImage_) {
        cameraImageDrawer_->sarSetDrawRange(-rangeX, -rangeY, rangeX, rangeY);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        cameraImageDrawer_->sarDraw(videoImage);

        imageRect.set(
                (1.0f / -rangeX + 1.0f) * videoImage.getWidth() / 2,
                (1.0f / -rangeY + 1.0f) * videoImage.getHeight() / 2,
                (1.0f / rangeX + 1.0f) * videoImage.getWidth() / 2,
                (1.0f / rangeY + 1.0f) * videoImage.getHeight() / 2);
    } else {
        int32_t merginX = std::max(0, videoImage.getWidth()  - videoImage.getHeight() * rotScrWidth  / rotScrHeight) / 2;
        int32_t merginY = std::max(0, videoImage.getHeight() - videoImage.getWidth()  * rotScrHeight / rotScrWidth)  / 2;
        SarRect rect(merginX, merginY, videoImage.getWidth() - merginX, videoImage.getHeight() - merginY);
        CHECK_ERR(cameraImageDrawer_->sarDraw(videoImage, rect));

        imageRect = rect;
    }

    // Update initialization point matrix
    // * float x = (initPoint->position_.x_ - imageRect.left_) / imageRect.width() * 2.0f - 1.0f;
    // * float y = (initPoint->position_.y_ - imageRect.top_) / imageRect.height() * 2.0f - 1.0f;
    // * Matrix44 matrix = translateM(identityMatrix, x, y, 0);
    float scaleX = 2.0f / imageRect.width();
    float scaleY = 2.0f / imageRect.height();
    float offsetX = 1.0f - imageRect.right_ * scaleX;
    float offsetY = 1.0f - imageRect.bottom_ * scaleY;
    initPointMatrix_ = cameraImageTransform_.sarGetAdjustmentMatrix();
    initPointMatrix_ = sarTranslateM(initPointMatrix_, offsetX, offsetY, 0.0f);
    initPointMatrix_ = sarScaleM(initPointMatrix_, scaleX, scaleY, 0.0f);
}

void BackgroundDrawer::draw(SarImage& videoImage, int32_t screenWidth, int32_t screenHeight) {
    // Check geometry of screen and camera image
    SarRotation screenRotation;
    CHECK_ERR(screenDevice_->sarGetRotation(&screenRotation));
    bool screenChanged = screenRotation != lastScreenRotation_
            || videoImage.getWidth() != lastVideoImageSize_.width_
            || videoImage.getHeight() != lastVideoImageSize_.height_
            || screenWidth != lastScreenSize_.width_
            || screenHeight != lastScreenSize_.height_
            || cameraChanged_;
    if (screenChanged) {
        lastScreenRotation_ = screenRotation;
        lastVideoImageSize_.width_ = videoImage.getWidth();
        lastVideoImageSize_.height_ = videoImage.getHeight();
        lastScreenSize_.width_ = screenWidth;
        lastScreenSize_.height_ = screenHeight;
        cameraChanged_ = false;

        cameraImageTransform_.sarSetScreenRotation(screenRotation);
        CHECK_ERR(cameraImageDrawer_->sarSetRotation(cameraImageTransform_.sarGetCameraImageRotation()));
        CHECK_ERR(cameraImageDrawer_->sarSetFlipX(cameraImageTransform_.sarGetCameraImageFlipX()));
        CHECK_ERR(cameraImageDrawer_->sarSetFlipY(cameraImageTransform_.sarGetCameraImageFlipY()));
    }

    int32_t rotImgWidth = videoImage.getWidth();
    int32_t rotImgHeight = videoImage.getHeight();
    int32_t rotScrWidth = screenWidth;
    int32_t rotScrHeight = screenHeight;
    if (cameraImageTransform_.sarGetCameraImageRotation() == SAR_ROTATION_90
        || cameraImageTransform_.sarGetCameraImageRotation() == SAR_ROTATION_270) {
        std::swap(rotImgWidth, rotImgHeight);
        std::swap(rotScrWidth, rotScrHeight);
    }

    //float rangeX;
    //float rangeY;

    // Calc fov
    if (screenChanged) {
        float drawHeight;
        drawHeight = std::min(rotImgHeight, rotImgWidth * screenHeight / screenWidth);
        //float drawHeightRatio = drawHeight / videoImage.getHeight();
    }

    // Draw camera image
    SarRect imageRect;
    int32_t merginX = std::max(0, videoImage.getWidth()  - videoImage.getHeight() * rotScrWidth  / rotScrHeight) / 2;
    int32_t merginY = std::max(0, videoImage.getHeight() - videoImage.getWidth()  * rotScrHeight / rotScrWidth)  / 2;
    SarRect rect(merginX, merginY, videoImage.getWidth() - merginX, videoImage.getHeight() - merginY);
    CHECK_ERR(cameraImageDrawer_->sarDraw(videoImage, rect));

    imageRect = rect;

    float scaleX = 2.0f / imageRect.width();
    float scaleY = 2.0f / imageRect.height();
    float offsetX = 1.0f - imageRect.right_ * scaleX;
    float offsetY = 1.0f - imageRect.bottom_ * scaleY;
    initPointMatrix_ = cameraImageTransform_.sarGetAdjustmentMatrix();
    initPointMatrix_ = sarTranslateM(initPointMatrix_, offsetX, offsetY, 0.0f);
    initPointMatrix_ = sarScaleM(initPointMatrix_, scaleX, scaleY, 0.0f);
}

//--------------------------------------------------------------------------------------------------

void TestContentDrawer::startDraw() {
    CHECK_ERR(rectDrawer_.sarStart());
    CHECK_ERR(cubeDrawer_.sarStart());
    CHECK_ERR(landmarkDrawer_.sarStart());
}

void TestContentDrawer::stopDraw() {
    CHECK_ERR(rectDrawer_.sarStop());
    CHECK_ERR(cubeDrawer_.sarStop());
    CHECK_ERR(landmarkDrawer_.sarStop());
}

void TestContentDrawer::setLandMarkBarLocation(int x, int y){
	landMarkBarLocation_ = SarVector2(x, y);
}

void TestContentDrawer::draw(int32_t screenWidth, int32_t screenHeight, float fovy, bool calibrated, bool getfromapi,
        const SarMatrix44& initPointMatrix, const SarRecognitionResult* results, int32_t numResults, bool isDrawCube, bool isShowLandmark) {
    static const float DEFAULT_RECT_SIZE = 0.125f;

    static const float RECT_CALIB_R = 0.0f;
    static const float RECT_CALIB_G = 1.0f;
    static const float RECT_CALIB_B = 1.0f;
    static const float RECT_CALIB_A = 1.0f;
    static const float RECT_GETAPI_R = 1.0f;
    static const float RECT_GETAPI_G = 0.0f;
    static const float RECT_GETAPI_B = 1.0f;
    static const float RECT_GETAPI_A = 1.0f;
    static const float RECT_NOT_CALIB_R = 1.0f;
    static const float RECT_NOT_CALIB_G = 1.0f;
    static const float RECT_NOT_CALIB_B = 0.0f;
    static const float RECT_NOT_CALIB_A = 1.0f;

    static const float AXIS_LENGTH = 1.0f;

    static const float AXIS_X_R = 1.0f;
    static const float AXIS_X_G = 0.0f;
    static const float AXIS_X_B = 0.0f;

    static const float AXIS_Y_R = 0.0f;
    static const float AXIS_Y_G = 1.0f;
    static const float AXIS_Y_B = 0.0f;

    static const float AXIS_Z_R = 0.0f;
    static const float AXIS_Z_G = 0.0f;
    static const float AXIS_Z_B = 1.0f;

    static const float AXIS_A1 = 1.0f;
    static const float AXIS_A2 = 0.1f;

    static const float CUBE_SIZE_RATIO = 0.5f;
    static const float CUBE_HEIGHT_RATIO = 0.7f;

    static const float CUBE_R = 0.6f;
    static const float CUBE_G = 0.5f;
    static const float CUBE_B = 1.0f;
    static const float CUBE_A = 0.5f;

    if (numResults == 0) {
        return;
    }

    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw initialization points
    CHECK_ERR(landmarkDrawer_.sarDraw(initPointMatrix, results[0].initPoints_, results[0].numInitPoints_));

    if (!results[0].isRecognized_) {
        return;
    }

    // Setup model view projection matrix
    float aspect = static_cast<float>(screenWidth) / screenHeight;
    SarMatrix44 modelViewMatrix;
    sarConvertPose2Matrix(results[0].position_, results[0].rotation_, &modelViewMatrix);
    SarMatrix44 projectionMatrix = sarSetPerspectiveM(fovy, aspect, 0.01f, 100.0f);
    SarMatrix44 pmvMatrix = projectionMatrix * modelViewMatrix;

    // Draw Rectangle
    if (results[0].target_ != lastTarget_) {
        lastTarget_ = results[0].target_;
        if (results[0].target_ != NULL) {
            CHECK_ERR(results[0].target_->sarGetPhysicalSize(&targetSize_));
        } else {
            targetSize_.set(0.0f, 0.0f);
        }
    }

    float sx = (targetSize_.x_ == 0.0f) ? DEFAULT_RECT_SIZE : targetSize_.x_;
    float sy = (targetSize_.y_ == 0.0f) ? DEFAULT_RECT_SIZE : targetSize_.y_;
    float r, g, b, a;
    if (calibrated) {
        r = RECT_CALIB_R;
        g = RECT_CALIB_G;
        b = RECT_CALIB_B;
        a = RECT_CALIB_A;
    } else if (getfromapi) {
        r = RECT_GETAPI_R;
        g = RECT_GETAPI_G;
        b = RECT_GETAPI_B;
        a = RECT_GETAPI_A;
    } else {
        r = RECT_NOT_CALIB_R;
        g = RECT_NOT_CALIB_G;
        b = RECT_NOT_CALIB_B;
        a = RECT_NOT_CALIB_A;
    }
    rectDrawer_.sarDraw(pmvMatrix, sx, sy, r, g, b, a, true);
    rectDrawer_.sarDraw(pmvMatrix, sx, 0.0f, r, g, b, a, true);
    rectDrawer_.sarDraw(pmvMatrix, 0.0f, sy, r, g, b, a, true);

    // Draw axis
    SarMatrix44 xMatrix = sarTranslateM(pmvMatrix, (AXIS_LENGTH - sx/2) * 0.5f + sx/2, 0.0f, 0.0f);
    rectDrawer_.sarDraw(xMatrix, AXIS_LENGTH - sx/2, 0.0f, AXIS_X_R, AXIS_X_G, AXIS_X_B, AXIS_A1, true);
    SarMatrix44 xMatrix2 = sarTranslateM(pmvMatrix, -((AXIS_LENGTH - sx/2) * 0.5f + sx/2), 0.0f, 0.0f);
    rectDrawer_.sarDraw(xMatrix2, AXIS_LENGTH - sx/2, 0.0f, AXIS_X_R, AXIS_X_G, AXIS_X_B, AXIS_A2, true);
    SarMatrix44 yMatrix = sarTranslateM(pmvMatrix, 0.0f, (AXIS_LENGTH - sy/2) * 0.5f + sy/2, 0.0f);
    rectDrawer_.sarDraw(yMatrix, 0.0f, AXIS_LENGTH - sy/2, AXIS_Y_R, AXIS_Y_G, AXIS_Y_B, AXIS_A1, true);
    SarMatrix44 yMatrix2 = sarTranslateM(pmvMatrix, 0.0f, -((AXIS_LENGTH - sy/2) * 0.5f + sy/2), 0.0f);
    rectDrawer_.sarDraw(yMatrix2, 0.0f, AXIS_LENGTH- sy/2, AXIS_Y_R, AXIS_Y_G, AXIS_Y_B, AXIS_A2, true);

    SarMatrix44 rotMatrix;
    rotMatrix.sarSetRotation(sarRotationToQuaternion(M_PI * 0.5f, 0.0f, 1.0f, 0.0f));
    SarMatrix44 zMatrix = sarTranslateM(pmvMatrix, 0.0f, 0.0f, AXIS_LENGTH * 0.5f) * rotMatrix;
    rectDrawer_.sarDraw(zMatrix, AXIS_LENGTH, 0.0f, AXIS_Z_R, AXIS_Z_G, AXIS_Z_B, AXIS_A1, true);

    // Draw cube
    if(isDrawCube){
    	sx *= CUBE_SIZE_RATIO;
    	sy *= CUBE_SIZE_RATIO;
    	float sz = std::min(sx, sy) * CUBE_HEIGHT_RATIO;
    	SarMatrix44 cubeMatrix = sarTranslateM(pmvMatrix, 0.0f, 0.0f, sz * 0.5f);
    	cubeDrawer_.sarDraw(cubeMatrix, sx, sy, sz, CUBE_R, CUBE_G, CUBE_B, CUBE_A, false);
    }

    if(isShowLandmark){
    	// Draw node points
        CHECK_ERR(landmarkDrawer_.sarDraw(pmvMatrix, results[0].nodePoints_, results[0].numNodePoints_));

        // Draw landmarks
    	CHECK_ERR(landmarkDrawer_.sarDraw(pmvMatrix, results[0].landmarks_, results[0].numLandmarks_));
    }
}

void TestContentDrawer::drawLandmarkBar(int32_t screenWidth, int32_t screenHeight, int trackedLandmarkCount
		, int lostLandmarkCount, int suspendedLandmarkCount, int maskedLandmarkCount, int maxLandMarkCount) {
	glClear(GL_DEPTH_BUFFER_BIT);

	int totalCount = trackedLandmarkCount + lostLandmarkCount + suspendedLandmarkCount + maskedLandmarkCount;
	float barLength = totalCount==0 ? 0 : 1.0f / (totalCount);

    float trackedLandmarkBarX = barLength * trackedLandmarkCount;
    float lostLandmarkBarX = barLength * lostLandmarkCount;
    float suspendedLandmarkBarX = barLength * suspendedLandmarkCount;
    float maskedLandmarkBarX = barLength * maskedLandmarkCount;
    float LandmarkBarY = 40.0f / screenHeight;

	//Draw landmarkbar
    SarMatrix44 modelViewMatrix;
    SarQuaternion zeroRotation;
    SarVector3 zeroVector;
    sarConvertPose2Matrix(zeroVector, zeroRotation, &modelViewMatrix);
    float x = (landMarkBarLocation_.x_ - screenWidth / 2) / (screenWidth / 2);
    float y = (landMarkBarLocation_.y_ - screenHeight / 2) / (screenHeight / 2);


    SarMatrix44 locationMatrix = sarTranslateM(SarMatrix44::IDENTITY, x + trackedLandmarkBarX / 2, y + LandmarkBarY * 1 / 2, 0);
    x += trackedLandmarkBarX;
    rectDrawer_.sarDraw(modelViewMatrix * locationMatrix, trackedLandmarkBarX, LandmarkBarY, 0.0f, 1.0f, 0.0f, 1.0f, false);
    locationMatrix = sarTranslateM(SarMatrix44::IDENTITY, x + lostLandmarkBarX / 2, y + LandmarkBarY * 1 / 2, 0);
    x += lostLandmarkBarX;
    rectDrawer_.sarDraw(modelViewMatrix * locationMatrix, lostLandmarkBarX, LandmarkBarY, 1.0f, 0.0f, 0.0f, 1.0f, false);
    locationMatrix = sarTranslateM(SarMatrix44::IDENTITY, x + suspendedLandmarkBarX / 2, y + LandmarkBarY * 1 / 2, 0);
    x += suspendedLandmarkBarX;
    rectDrawer_.sarDraw(modelViewMatrix * locationMatrix, suspendedLandmarkBarX, LandmarkBarY, 0.0f, 1.0f, 1.0f, 1.0f, false);
    locationMatrix = sarTranslateM(SarMatrix44::IDENTITY, x + maskedLandmarkBarX / 2, y + LandmarkBarY * 1 / 2, 0);
    rectDrawer_.sarDraw(modelViewMatrix * locationMatrix, maskedLandmarkBarX, LandmarkBarY, 1.0f, 1.0f, 0.0f, 1.0f, false);
}

void TestContentDrawer::drawTriangulateMasks() {
	glClear(GL_DEPTH_BUFFER_BIT);
	float maskBarX = sqrt(2.0f);
	float maskBarY = 0.01f;

    SarMatrix44 rotMat1;
    rotMat1.sarSetRotation(sarRotationToQuaternion(M_PI * 3/4, 0.0f, 0.0f, 1.0f));
    SarMatrix44 rotMat2;
    rotMat2.sarSetRotation(sarRotationToQuaternion(M_PI * 1/4, 0.0f, 0.0f, 1.0f));

    //Draw masks
    SarMatrix44 modelViewMatrix;
    SarQuaternion zeroRotation;
    SarVector3 zeroVector;
    sarConvertPose2Matrix(zeroVector, zeroRotation, &modelViewMatrix);

    SarMatrix44 locationMatrix = sarTranslateM(SarMatrix44::IDENTITY, 0.5f, 0.5f, 0);
    rectDrawer_.sarDraw(modelViewMatrix * locationMatrix * rotMat1, maskBarX, maskBarY, 1.0f, 1.0f, 0.0f, 0.3f, false);
    locationMatrix = sarTranslateM(SarMatrix44::IDENTITY, -0.5f, -0.5f, 0);
    rectDrawer_.sarDraw(modelViewMatrix * locationMatrix * rotMat1, maskBarX, maskBarY, 1.0f, 1.0f, 0.0f, 0.3f, false);
    locationMatrix = sarTranslateM(SarMatrix44::IDENTITY, 0.5f, -0.5f, 0);
    rectDrawer_.sarDraw(modelViewMatrix * locationMatrix * rotMat2, maskBarX, maskBarY, 1.0f, 1.0f, 0.0f, 0.3f, false);
    locationMatrix = sarTranslateM(SarMatrix44::IDENTITY, -0.5f, 0.5f, 0);
    rectDrawer_.sarDraw(modelViewMatrix * locationMatrix * rotMat2, maskBarX, maskBarY, 1.0f, 1.0f, 0.0f, 0.3f, false);
}

