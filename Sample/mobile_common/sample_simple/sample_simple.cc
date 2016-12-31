#include "sample_simple.h"
#include "../sample_common/sample_common_util.h"

using namespace sarSmartar;

SampleCoreImpl::SampleCoreImpl(void* nativeContext, void* nativeEnv, int32_t recogMode, int32_t initMode)
: SampleCore(nativeContext, nativeEnv, recogMode, initMode)
{
    SAR_SMARTAR_LOGD(TAG, "nativeCreate ***************************************************************************");

    // Misc variables
    screenWidth_ = 0;
    screenHeight_ = 0;
    resumed_ = false;
    surfaceCreated_ = false;
    cameraFrameCount_ = 0;
    drawFrameCount_ = 0;
    recognizeCount_ = 0;
    state_ = 0;
    recogMode_ = recogMode;
    initMode_ = initMode;
    useWorkerThread_ = true;
    //-1 setting is for default.
    videoImageSizeSelected_ = -1;
    videoImageFpsRangeSelected_ = -1;
    focusModeSelected_ = -1;
    flashModeSelected_ = -1;
    exposureModeSelected_ = -1;
    whiteBalanceModeSelected_ = -1;
    sceneModeSelected_ = -1;
    trackedLandmarkCount_ = 0;
    lostLandmarkCount_ = 0;
    suspendedLandmarkCount_ = 0;
    maskedLandmarkCount_ = 0;
    targetCount_ = 0;
    sceneMapTarget_ = NULL;
    currentTargetIndex_ = 0;
    showLandmarkBar_ = true;
    showLandmark_ = true;
    useFrontCameraSelected_ = false;
    useSensorDevice_ = true;
    localizeImpossibleCounter_ = 0;

    // Initialize smart
    CHECK_CONSTRUCT(smart_ = new SarSmart(nativeContext, nativeEnv, LICENSE_FILE_NAME));
#if defined(TARGET_SMARTAR_ANDROID) || defined(TARGET_SMARTAR_IOS)
    if (smart_->sarIsConstructorFailed()) { return; }
#endif

    CHECK_CONSTRUCT(recognizer_ = new SarRecognizer(smart_
    		, (SarRecognitionMode)recogMode, (SarSceneMappingInitMode)initMode));
#if defined(TARGET_SMARTAR_ANDROID) || defined(TARGET_SMARTAR_IOS)
    if (recognizer_->sarIsConstructorFailed()) { return; }
#endif

    // Initialize targets(Default is smartar01 and smartar02.)
#ifdef USE_COMPOUND_TARGET
    SarAssetStreamIn stream(smart_, DICTIONARY_FILE_NAME1);
    CHECK_CONSTRUCT(&stream);
    SarLearnedImageTarget* learnedImageTarget;
    CHECK_CONSTRUCT(learnedImageTarget = new SarLearnedImageTarget(smart_, &stream));

    SarVector2 childSize;
    CHECK_ERR(learnedImageTarget->getPhysicalSize(&childSize));
    SarChildTargetInfo childInfo;
    childInfo.position_.set(0.0f, -childSize.y_ * 0.5f, childSize.y_ * 0.5f);
    childInfo.rotation_.set(cosf(M_PI * 0.5f * 0.5f), sinf(M_PI * 0.5f * 0.5f), 0.0f, 0.0f);
    SarTarget* childTargets[1] = { learnedImageTarget };
    SarCompoundTarget* compoundTarget;
    CHECK_CONSTRUCT(compoundTarget = new SarCompoundTarget(smart_, childTargets, &childInfo, 1));
    targets_.push_back(compoundTarget);

    currentTargetIndex_ = 0;
    CHECK_ERR(recognizer_->setTargets(&targets_[currentTargetIndex_], 1));
#else //USE_COMPOUND_TARGET
    //create 2 targets from asset in default.
    loadTargetFromAsset(DICTIONARY_FILE_NAME1);
    loadTargetFromAsset(DICTIONARY_FILE_NAME2);

    // Default recognizer checks 2 targets.
    CHECK_ERR(recognizer_->sarSetTargets(&activeTargets_[currentTargetIndex_], targetCount_));
    CHECK_ERR(recognizer_->sarSetMaxTargetsPerFrame(RECOGNIZING_NUM_OF_MAX_TARGET));
#endif //USE_COMPOUND_TARGET

    // Initialize drawer
    backgroundDrawer_.init(smart_);
#ifdef DRAW_WHOLE_CAMERA_IMAGE
    backgroundDrawer_.setDrawWholeCameraImage(true);
#endif //DRAW_WHOLE_CAMERA_IMAGE

    videoImageQueue_ = new SarImageQueue(smart_);
    landmarkBuffer_.assign(SAR_MAX_NUM_LANDMARKS, SarLandmark());
    initPointBuffer_.assign(SAR_MAX_NUM_INITIALIZATION_POINTS, SarInitPoint());

	// Initialize worker threads
	int32_t numThread = (recogMode == SAR_RECOGNITION_MODE_TARGET_TRACKING)? 2 : 1;
    workerThreadController_.init(recognizer_, numThread);
}

void SampleCoreImpl::doDrawFrame()
{
    if (!resumed_) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

	int32_t numResults;
	int recogNum = RECOGNIZING_NUM_OF_MAX_TARGET > targetCount_
			? targetCount_ : RECOGNIZING_NUM_OF_MAX_TARGET;
	result_.clear();
	result_.resize(recogNum);

	result_[0].landmarks_ = &landmarkBuffer_.front();
	result_[0].maxLandmarks_ = static_cast<int32_t>(landmarkBuffer_.size());
	result_[0].initPoints_ = &initPointBuffer_.front();
	result_[0].maxInitPoints_ = static_cast<int32_t>(initPointBuffer_.size());

	CHECK_ERR(numResults = recognizer_->sarGetResults(&result_[0], recogNum));

	if(recogMode_ == SAR_RECOGNITION_MODE_TARGET_TRACKING){
		state_ = result_[0].targetTrackingState_;
	}else{
		state_ = result_[0].sceneMappingState_;
		//count landmarks
		trackedLandmarkCount_ = 0;
		lostLandmarkCount_ = 0;
		suspendedLandmarkCount_ = 0;
		maskedLandmarkCount_ = 0;

		for (int i = 0; i < result_[0].numLandmarks_; i++) {
			switch (result_[0].landmarks_[i].state_) {
				case SAR_LANDMARK_STATE_TRACKED:
					//Tracked Landmark:GLEEN
					trackedLandmarkCount_++;
					break;
				case SAR_LANDMARK_STATE_LOST:
					//Lost landmark:RED
					lostLandmarkCount_++;
					break;
				case SAR_LANDMARK_STATE_SUSPENDED:
					//Suspended landmark:CYAN
					suspendedLandmarkCount_++;
					break;
				case SAR_LANDMARK_STATE_MASKED:
					//Masked landmark:YELLOW
					maskedLandmarkCount_++;
					break;
				default:
					break;
			}
		}
	}
	// Draw camera image
    SarImage *videoImage;
	if (numResults > 0) {
		videoImage = videoImageQueue_->sarRetrieve(result_[0].timestamp_);
	} else {
		videoImage = videoImageQueue_->sarRetrieve();
	}
    if (videoImage != NULL) {
		backgroundDrawer_.draw(*videoImage, screenWidth_, screenHeight_, *cameraController_.getCameraDevice());
	} else {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}


	// Draw contents
	for(int i = 0; i < numResults; i++){
	    //SAR_SMARTAR_LOGD(TAG, "numResults = %d", numResults);
		SarRecognitionResult adjustedResult = result_[i];
		backgroundDrawer_.adjustPose(result_[i].position_, result_[i].rotation_,
				&adjustedResult.position_, &adjustedResult.rotation_);
		contentDrawer_.draw(screenWidth_, screenHeight_,
				backgroundDrawer_.getFovy(), backgroundDrawer_.getCalibrated(), backgroundDrawer_.isGetFromAPI(), backgroundDrawer_.getInitPointMatrix(),
				&adjustedResult, numResults, true, showLandmark_);

	    SarVector2 targetSize_;
        if (adjustedResult.target_ != NULL) {
            CHECK_ERR(adjustedResult.target_->sarGetPhysicalSize(&targetSize_));
        } else {
            targetSize_.set(0.0f, 0.0f);
        }
	}
	if(showLandmarkBar_){
		contentDrawer_.drawLandmarkBar(screenWidth_, screenHeight_, trackedLandmarkCount_, lostLandmarkCount_, suspendedLandmarkCount_, maskedLandmarkCount_, SAR_MAX_NUM_LANDMARKS);
	}
}

/*
 * CameraController::Listener::onImage() callback
 */
void SampleCoreImpl::onImage(const SarImageHolder& imageHolder, uint64_t timestamp, int32_t numSensorStates, SarSensorState* sensorStates)
{
    ++cameraFrameCount_;

    if (!(resumed_ && surfaceCreated_)) return;
	// Obtain and enqueue image
	int32_t sizeInBytes = imageHolder.sarGetImageSizeInBytes();
	SarImage *image = videoImageQueue_->sarNewImage(sizeInBytes, timestamp);
	if (image == NULL) return;
	imageHolder.sarGetImage(image, sizeInBytes);
	if (image->getWidth() != selectedSize_.width_ || image->getHeight() != selectedSize_.height_) return;
	videoImageQueue_->sarEnqueue();

	// Create recognition request
	SarRecognitionRequest request;
	request.image_ = *image;
	request.timestamp_ = timestamp;
	request.sensorStates_ = sensorStates;
	request.numSensorStates_ = numSensorStates;

	if (recogMode_ == SAR_RECOGNITION_MODE_SCENE_MAPPING){
		// Check error state
		if (state_ == SAR_SCENE_MAPPING_STATE_LOCALIZE_IMPOSSIBLE) {
			localizeImpossibleCounter_++;
			// LOCALIZE_IMPOSSIBLE_RESET_WAIT is for this sample.
			// Please reset immediately in your app.
			if (localizeImpossibleCounter_ > LOCALIZE_IMPOSSIBLE_RESET_WAIT) {
				SAR_SMARTAR_LOGD(TAG, "localize impossible, so reset recognizer");
				workerThreadController_.stop();
				CHECK_ERR(recognizer_->sarReset());
				workerThreadController_.start();
				localizeImpossibleCounter_ = 0;
			}
		}
	}

	// Issue recognition request
	if(useWorkerThread_){
		CHECK_ERR(recognizer_->sarDispatch(request));
	}else{
        CHECK_ERR(recognizer_->sarRun(request));
	}
}

/*
 * CameraController::Listener::onCameraError() callback
 */
void SampleCoreImpl::onCameraError(int32_t error)
{
    SAR_SMARTAR_LOGD(TAG, "onCameraError::error code is %d ***************************************************************************", error);
}
