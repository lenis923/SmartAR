#include "sample_full.h"
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
    useTriangulateMasks_ = false;
    removeLostLandmarks_ = false;
    //-1 setting is for default.
    videoImageSizeSelected_ = -1;
    stillImageSizeSelected_ = -1;
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
    // Enable mask feature
    CHECK_ERR(recognizer_->sarSetMaxTriangulateMasks(NUM_OF_TRIANGULATE_MASK));

    // Initialize targets(Default is smartar01 and smartar02.)
#ifdef USE_COMPOUND_TARGET
    SarAssetStreamIn stream(smart_, DICTIONARY_FILE_NAME1);
    CHECK_CONSTRUCT(&stream);
    SarLearnedImageTarget* learnedImageTarget;
    CHECK_CONSTRUCT(learnedImageTarget = new SarLearnedImageTarget(smart_, &stream));

    SarVector2 childSize;
    CHECK_ERR(learnedImageTarget->sarGetPhysicalSize(&childSize));
    SarChildTargetInfo childInfo;
    childInfo.position_.set(0.0f, -childSize.y_ * 0.5f, childSize.y_ * 0.5f);
    childInfo.rotation_.set(cosf(M_PI * 0.5f * 0.5f), sinf(M_PI * 0.5f * 0.5f), 0.0f, 0.0f);
    Target* childTargets[1] = { learnedImageTarget };
    SarCompoundTarget* compoundTarget;
    CHECK_CONSTRUCT(compoundTarget = new SarCompoundTarget(smart_, childTargets, &childInfo, 1));
    targets_.push_back(compoundTarget);

    currentTargetIndex_ = 0;
    CHECK_ERR(recognizer_->sarSetTargets(&targets_[currentTargetIndex_], 1));
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
    nodePointBuffer_.assign(SAR_MAX_NUM_NODE_POINTS, SarNodePoint());
    initPointBuffer_.assign(SAR_MAX_NUM_INITIALIZATION_POINTS, SarInitPoint());

	// Set triangulateMasks
    for(int i = 0; i < NUM_OF_TRIANGULATE_MASK; i++){
        SarTriangle2 triangle2;
        for(int j = 0; j < 3; j++){
        	triangle2.points_[j] = triangulateMasksArray[i][j];
        }
        triangulateMasks_.push_back(triangle2);
    }

	// Initialize worker threads
	int32_t numThread = (recogMode == SAR_RECOGNITION_MODE_TARGET_TRACKING)? 2 : 1;
    workerThreadController_.init(recognizer_, numThread);
}

void SampleCoreImpl::doResume(void* nativeVideoOutput)
{
	SampleCore::doResume(nativeVideoOutput);
    setCameraStillImageSize(stillImageSizeSelected_);
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
	if (recogNum <= 0) {
		recogNum = 1;
	}
	result_.clear();
	result_.resize(recogNum);

	result_[0].landmarks_ = &landmarkBuffer_.front();
	result_[0].maxLandmarks_ = static_cast<int32_t>(landmarkBuffer_.size());
	result_[0].nodePoints_ = &nodePointBuffer_.front();
	result_[0].maxNodePoints_ = static_cast<int32_t>(nodePointBuffer_.size());
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
					//If removeLostLandmarks_ is true, remove lost landmarks.
					if(removeLostLandmarks_){
						CHECK_ERR(recognizer_->sarRemoveLandmark(result_[0].landmarks_[i]));
					}
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
	if(useTriangulateMasks_){
		contentDrawer_.drawTriangulateMasks();
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

	if (useTriangulateMasks_){
		request.numTriangulateMasks_ = static_cast<int32_t>(triangulateMasks_.size());
		request.triangulateMasks_ = &triangulateMasks_.front();
	}

	// Issue recognition request
	if(useWorkerThread_){
		CHECK_ERR(recognizer_->sarDispatch(request));
	}else{
        CHECK_ERR(recognizer_->sarRun(request));
	}
}

void SampleCoreImpl::setDenseMapMode(int32_t denseMapMode)
{
	workerThreadController_.stop();
	switch(denseMapMode){
		case SAR_DENSE_MAP_DISABLE:
			CHECK_ERR(recognizer_->sarSetDenseMapMode(SAR_DENSE_MAP_DISABLE));
			denseMapMode_ = SAR_DENSE_MAP_DISABLE;
			break;
		case SAR_DENSE_MAP_SEMI_DENSE:
			CHECK_ERR(recognizer_->sarSetDenseMapMode(SAR_DENSE_MAP_SEMI_DENSE));
			denseMapMode_ = SAR_DENSE_MAP_SEMI_DENSE;
			break;
		default:
			break;
	}
	workerThreadController_.start();
}

/*
 * Capture still image
 */
void SampleCoreImpl::captureStillImage() {
    if (resumed_) {
        CHECK_ERR(cameraController_.getCameraDevice()->sarCaptureStillImage());
    }
}

/*
 * CameraController::Listener::onStillImage() callback, invoked by captureStillImage()
 */
void SampleCoreImpl::onStillImage(const SarImageHolder& imageHolder) {
    SAR_SMARTAR_LOGD(TAG, "onStillImage ***************************************************************************");
	int32_t sizeInBytes = imageHolder.sarGetImageSizeInBytes();
    SarImage image(smart_);
    std::vector<unsigned char> imageChar(sizeInBytes);
    image.setData(&imageChar.front());
	int32_t ok = imageHolder.sarGetImage(&image, sizeInBytes);
    
    if (ok == SAR_OK && sizeInBytes > 0) {
#ifdef __ANDROID__
        SarFileStreamOut fileStreamOut(smart_, stillImageFilePath_);
        fileStreamOut.sarWrite(image.getData(), sizeInBytes);
        mediaScannerFunc();
#elif defined _IOS_SDK_
        // This function is implemented in SampleViewController.mm
        void SaveJpegToCameraRoll(unsigned char *jpegData, int32_t jpegDataLength);
        SaveJpegToCameraRoll(&imageChar[0], sizeInBytes);
#endif
    }
}

/*
 * CameraController::Listener::onCameraError() callback
 */
void SampleCoreImpl::onCameraError(int32_t error) {
    SAR_SMARTAR_LOGD(TAG, "onCameraError::error code is %d ***************************************************************************", error);
}

int SampleCoreImpl::getNumOfSupportedStillImageSize(){
	std::vector<SarSize> supportedStillImageSizeBuffer(SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);
	return cameraController_.getCameraDevice()->sarGetSupportedStillImageSize(&supportedStillImageSizeBuffer[0], SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);
}

void SampleCoreImpl::getSupportedStillImageSize(char charArray[][IMAGE_SIZE_STRING_LENGTH], int arrayLength){
	std::vector<SarSize> supportedStillImageSizeBuffer(arrayLength);
	int supportedStillImageSizeCount
	= cameraController_.getCameraDevice()->sarGetSupportedStillImageSize(&supportedStillImageSizeBuffer[0], arrayLength);
	for(int i = 0; i < supportedStillImageSizeCount; i++){
		strcpy(charArray[i], "");
		char strBufferWidth[IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH] = "";
		char strBufferHeight[IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH] = "";
		snprintf(strBufferWidth, IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH, "%d", supportedStillImageSizeBuffer[i].width_);
		snprintf(strBufferHeight, IMAGE_SIZE_WIDTH_OR_HEIGHT_STRING_LENGTH, "%d", supportedStillImageSizeBuffer[i].height_);

		strncat(charArray[i], strBufferWidth, IMAGE_SIZE_STRING_LENGTH);
		strncat(charArray[i], "*", IMAGE_SIZE_STRING_LENGTH - strlen(charArray[i]));
		strncat(charArray[i], strBufferHeight, IMAGE_SIZE_STRING_LENGTH - strlen(charArray[i]));
	}
}

void SampleCoreImpl::setCameraStillImageSize(int32_t selectNum){
	cameraController_.stop();

	stillImageSizeSelected_ = selectNum;
	if(selectNum == -1) {
		stillImageSizeSelected_ = getBiggestStillImageSize();//For default setting.
		selectNum = stillImageSizeSelected_;
	}

	std::vector<SarSize> supportedStillImageSizeBuffer(SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);
	int supportedStillImageSizeCount = cameraController_.getCameraDevice()
			->sarGetSupportedStillImageSize(&supportedStillImageSizeBuffer[0]
			        , SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);

	if(supportedStillImageSizeCount > 0)
		cameraController_.getCameraDevice()->sarSetStillImageSize(
				supportedStillImageSizeBuffer[selectNum].width_, supportedStillImageSizeBuffer[selectNum].height_);

    SarCameraDeviceInfo cameraDeviceInfo;
    CHECK_ERR(cameraController_.getCameraDevice()->sarGetDeviceInfo(&cameraDeviceInfo));
    CHECK_ERR(recognizer_->sarSetCameraDeviceInfo(cameraDeviceInfo));

    cameraController_.start();
}

int SampleCoreImpl::getBiggestStillImageSize(){
	std::vector<SarSize> supportedStillImageSizeBuffer(SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);
	int supportedStillImageSizeCount =cameraController_.getCameraDevice()
			->sarGetSupportedStillImageSize(&supportedStillImageSizeBuffer[0]
			        , SUPPORTED_NUM_OF_IMAGE_SIZE_MAX);

	int curBiggestIndex = 0;
	float biggestMeasuresize  = 0;
	for(int i = 0; i < supportedStillImageSizeCount; i++){

		float curMeasuresize = supportedStillImageSizeBuffer[i].width_ * supportedStillImageSizeBuffer[i].height_;
        if (curMeasuresize > biggestMeasuresize) {
        	curBiggestIndex = i;
        	biggestMeasuresize = curMeasuresize;
        }
	}
	return curBiggestIndex;
}
