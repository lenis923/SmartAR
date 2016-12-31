#pragma once

#include "SarCommon.h"


namespace sarSmartar {
    class SarSmart;
    class SarCameraDeviceInfo;
    class SarSensorDeviceInfo;
    class SarSensorState;

    extern CLASS_DECLSPEC const int32_t SAR_MAX_NUM_LANDMARKS;
    extern CLASS_DECLSPEC const int32_t SAR_MAX_NUM_NODE_POINTS;
    extern CLASS_DECLSPEC const int32_t SAR_MAX_NUM_INITIALIZATION_POINTS;
    extern CLASS_DECLSPEC const int32_t SAR_MAX_PROPAGATION_DURATION; //usec
    
    enum SarRecognitionMode {
        SAR_RECOGNITION_MODE_TARGET_TRACKING,
		SAR_RECOGNITION_MODE_SCENE_MAPPING,
    };
    
    enum SarSearchPolicy {
    	SAR_SEARCH_POLICY_FAST,
		SAR_SEARCH_POLICY_PRECISIVE,
    };
    
    enum SarSceneMappingInitMode {
    	SAR_SCENE_MAPPING_INIT_MODE_TARGET,
		SAR_SCENE_MAPPING_INIT_MODE_HFG,
		SAR_SCENE_MAPPING_INIT_MODE_VFG,
		SAR_SCENE_MAPPING_INIT_MODE_SFM,
		SAR_SCENE_MAPPING_INIT_MODE_DRY_RUN,
    };
    
    enum SarTargetTrackingState {
    	SAR_TARGET_TRACKING_STATE_IDLE,
		SAR_TARGET_TRACKING_STATE_SEARCH,
		SAR_TARGET_TRACKING_STATE_TRACKING,
    };
    
    enum SarSceneMappingState {
    	SAR_SCENE_MAPPING_STATE_IDLE,
		SAR_SCENE_MAPPING_STATE_SEARCH,
		SAR_SCENE_MAPPING_STATE_TRACKING,
		SAR_SCENE_MAPPING_STATE_LOCALIZE,
		SAR_SCENE_MAPPING_STATE_LOCALIZE_IMPOSSIBLE,
    };
    
    enum SarLandmarkState {
    	SAR_LANDMARK_STATE_TRACKED,
		SAR_LANDMARK_STATE_LOST,
		SAR_LANDMARK_STATE_SUSPENDED,
		SAR_LANDMARK_STATE_MASKED,
    };
    
    enum SarDenseMapMode {
    	SAR_DENSE_MAP_DISABLE,
		SAR_DENSE_MAP_SEMI_DENSE,
    };
    
    class CLASS_DECLSPEC SarTarget : SarNonCopyable
    {
    public:
        virtual WINAPI ~SarTarget() {}
        virtual int32_t WINAPI sarGetPhysicalSize(SarVector2* size) const = 0;
        
    protected:
        struct SarImpl;
        SarImpl *impl_;
        
        friend class SarCompoundTarget;
        friend class SarRecognizer;
        friend class SarRecognitionResultHolder;
    };
    
    class CLASS_DECLSPEC SarLearnedImageTarget : public SarTarget
    {
    public:
        WINAPI SarLearnedImageTarget(SarSmart* smart, SarStreamIn* stream, unsigned char* customerID = NULL, unsigned char* customerKey = NULL);
        virtual WINAPI ~SarLearnedImageTarget();
        bool WINAPI sarIsConstructorFailed() const;

        virtual int32_t WINAPI sarGetPhysicalSize(SarVector2* size) const;

    private:
        struct SarImpl;
        
        WINAPI SarLearnedImageTarget(SarImpl *impl);
        
        friend class SarRecognizer;
    };
    
    struct CLASS_DECLSPEC SarChildTargetInfo {
    	SarVector3 position_;
    	SarQuaternion rotation_;
    };
    
    class CLASS_DECLSPEC SarCompoundTarget : public SarTarget
    {
    public:
        WINAPI SarCompoundTarget(SarSmart* smart, const SarTarget* const* childTargets, const SarChildTargetInfo* childTargetInfos, int32_t numChildTargets);
        WINAPI ~SarCompoundTarget();
        bool WINAPI sarIsConstructorFailed() const;
        
        virtual int32_t WINAPI sarGetPhysicalSize(SarVector2* size) const; // * always returns 0, 0

    private:
        struct SarImpl;
        
        WINAPI SarCompoundTarget(SarImpl *impl);

        friend class SarRecognizer;
    };
    
    class CLASS_DECLSPEC SarSceneMapTarget : public SarTarget
    {
    public:
        WINAPI SarSceneMapTarget(SarSmart* smart, SarStreamIn* stream);
        WINAPI ~SarSceneMapTarget();
        bool WINAPI sarIsConstructorFailed() const;
        
        virtual int32_t WINAPI sarGetPhysicalSize(SarVector2* size) const; // * always returns 0, 0

    private:
        struct SarImpl;
        
        WINAPI SarSceneMapTarget(SarImpl *impl);

        friend class SarRecognizer;
    };
    
    struct CLASS_DECLSPEC SarRecognitionRequest {
    	SarImage image_;
        uint64_t timestamp_;
        int32_t numSensorStates_; // * the default value is 0
        SarSensorState* sensorStates_; // * the default value is NULL
        
        // at present for scene mapping
        int32_t numTriangulateMasks_; // * the default value is 0
        const SarTriangle2* triangulateMasks_; // * the default value is NULL
        
        WINAPI SarRecognitionRequest();
    };
    
    struct CLASS_DECLSPEC SarLandmark
    {
        uint32_t id_;
        SarLandmarkState state_;
        SarVector3 position_;
    };

    struct CLASS_DECLSPEC SarNodePoint
    {
        uint32_t id_;
        SarVector3 position_;
    };

    struct CLASS_DECLSPEC SarInitPoint
    {
        uint32_t id_;
        SarVector2 position_;
    };
    
    struct CLASS_DECLSPEC SarRecognitionResult
    {
        const SarTarget* target_;
        bool isRecognized_;
        SarVector3 position_;
        SarQuaternion rotation_;
        
        uint64_t timestamp_;
        
        SarVector3 velocity_;
        SarVector3 angularVelocity_;
        
        // for target tracking
        SarTargetTrackingState targetTrackingState_;
        
        // for scene mapping
        SarSceneMappingState sceneMappingState_;
        
        int32_t numLandmarks_;
        int32_t maxLandmarks_; // * the default value is 0
        SarLandmark* landmarks_; // * the default value is NULL
        
        int32_t numNodePoints_;
        int32_t maxNodePoints_; // * the default value is 0
        SarNodePoint* nodePoints_; // * the default value is NULL

        int32_t numInitPoints_;
        int32_t maxInitPoints_; // * the default value is 0
        SarInitPoint* initPoints_; // * the default value is NULL
        
        WINAPI SarRecognitionResult();
    };
    
    class CLASS_DECLSPEC SarWorkDispatchedListener
    {
    public:
        virtual WINAPI ~SarWorkDispatchedListener() {}
        virtual void WINAPI sarOnWorkDispatched() = 0;
    };
    
    class CLASS_DECLSPEC SarRecognitionResultHolder : SarNonCopyable
    {
    public:
        virtual int32_t WINAPI sarGetNumResults() const;
        virtual int32_t WINAPI sarGetResults(SarRecognitionResult* results, int32_t maxResults) const; // * returns the number of data written to the specified buffer or error code
        virtual int32_t WINAPI sarGetResult(const SarTarget& target, SarRecognitionResult* result) const;
        
    private:
        struct SarImpl;
        SarImpl *impl_;
        
        SarRecognitionResultHolder(SarImpl *impl);
        ~SarRecognitionResultHolder();
        
        friend class SarRecognizedListenerProxy;
    };
    
    class CLASS_DECLSPEC SarRecognizedListener
    {
    public:
        virtual WINAPI ~SarRecognizedListener() {}
        virtual void WINAPI sarOnRecognized(const SarRecognitionResultHolder& resultHolder) = 0;
    };

    class CLASS_DECLSPEC SarRecognizer : SarNonCopyable
    {
    public:
        WINAPI SarRecognizer(SarSmart* smart, SarRecognitionMode recogMode = SAR_RECOGNITION_MODE_TARGET_TRACKING, SarSceneMappingInitMode initMode = SAR_SCENE_MAPPING_INIT_MODE_TARGET);
        WINAPI ~SarRecognizer();
        bool WINAPI sarIsConstructorFailed() const;
        
        // setting
        int32_t WINAPI sarSetCameraDeviceInfo(const SarCameraDeviceInfo& info);
        int32_t WINAPI sarSetSensorDeviceInfo(const SarSensorDeviceInfo& info);
        int32_t WINAPI sarSetTargets(const SarTarget* const* targets, int32_t numTargets);
        
        // start and stop
        int32_t WINAPI sarReset();
        
        // run
        int32_t WINAPI sarRun(const SarRecognitionRequest& request);
        int32_t WINAPI sarDispatch(const SarRecognitionRequest& request);
        int32_t WINAPI sarRunWorker();
        int32_t WINAPI sarSetWorkDispatchedListener(SarWorkDispatchedListener* listener);
        
        // get results
        int32_t WINAPI sarGetNumResults() const;
        int32_t WINAPI sarGetResults(SarRecognitionResult* results, int32_t maxResults) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetResult(const SarTarget& target, SarRecognitionResult* result) const;
        int32_t WINAPI sarSetRecognizedListener(SarRecognizedListener* listener);
        
        // at present for target tracking only
        int32_t WINAPI sarSetMaxTargetsPerFrame(int32_t maxTargets);
        int32_t WINAPI sarSetSearchPolicy(SarSearchPolicy policy);
        
        // at present for scene mapping only
        int32_t WINAPI sarPropagateResult(const SarRecognitionResult& fromResult, SarRecognitionResult* toResult, uint64_t timestamp, bool useVelocity = true) const;
        int32_t WINAPI sarSetMaxTriangulateMasks(int32_t maxMasks);
        
        // for scene mapping
        int32_t WINAPI sarSaveSceneMap(SarStreamOut* stream) const;
        int32_t WINAPI sarFixSceneMap(bool isFix);
        int32_t WINAPI sarForceLocalize();
        int32_t WINAPI sarRemoveLandmark(const SarLandmark& landmark);
        int32_t WINAPI sarSetDenseMapMode(SarDenseMapMode mode);

    private:
        struct SarImpl;
        SarImpl* impl_;
        
        friend class SarSensorDeviceInfo;
        friend class SarRecognitionResultHolder;
    };
} // end of namespace smartar
