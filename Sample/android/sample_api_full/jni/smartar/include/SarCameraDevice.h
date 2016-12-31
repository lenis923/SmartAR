#pragma once

#include "SarCommon.h"


namespace sarSmartar {
    class SarSmart;

    // * Android: Note that the constants below must match the java-code constants. 
    enum SarFocusMode {
        SAR_FOCUS_MODE_MANUAL,
		SAR_FOCUS_MODE_CONTINUOUS_AUTO_PICTURE,
		SAR_FOCUS_MODE_CONTINUOUS_AUTO_VIDEO,
		SAR_FOCUS_MODE_EDOF,
		SAR_FOCUS_MODE_FIXED,
		SAR_FOCUS_MODE_INFINITY,
		SAR_FOCUS_MODE_MACRO,
    };
    
    // * Android: Note that the constants below must match the java-code constants. 
    enum SarFlashMode {
    	SAR_FLASH_MODE_AUTO,
		SAR_FLASH_MODE_OFF,
		SAR_FLASH_MODE_ON,
		SAR_FLASH_MODE_RED_EYE,
		SAR_FLASH_MODE_TORCH,
    };
    
    // * Android: Note that the constants below must match the java-code constants. 
    enum SarExposureMode {
    	SAR_EXPOSURE_MODE_MANUAL,
		SAR_EXPOSURE_MODE_CONTINUOUS_AUTO,
    };
    
    // * Android: Note that the constants below must match the java-code constants. 
    enum SarWhiteBalanceMode {
    	SAR_WHITE_BALANCE_MODE_CONTINUOUS_AUTO,
		SAR_WHITE_BALANCE_MODE_CLOUDY_DAYLIGHT,
		SAR_WHITE_BALANCE_MODE_DAYLIGHT,
		SAR_WHITE_BALANCE_MODE_FLUORESCENT,
		SAR_WHITE_BALANCE_MODE_INCANDESCENT,
		SAR_WHITE_BALANCE_MODE_SHADE,
		SAR_WHITE_BALANCE_MODE_TWILIGHT,
		SAR_WHITE_BALANCE_MODE_WARM_FLUORESCENT,
		SAR_WHITE_BALANCE_MODE_MANUAL,
    };
    
    // * Android: Note that the constants below must match the java-code constants. 
    enum SarSceneMode {
    	SAR_SCENE_MODE_ACTION,
		SAR_SCENE_MODE_AUTO,
		SAR_SCENE_MODE_BARCODE,
		SAR_SCENE_MODE_BEACH,
		SAR_SCENE_MODE_CANDLELIGHT,
		SAR_SCENE_MODE_FIREWORKS,
		SAR_SCENE_MODE_LANDSCAPE,
		SAR_SCENE_MODE_NIGHT,
		SAR_SCENE_MODE_NIGHT_PORTRAIT,
		SAR_SCENE_MODE_PARTY,
		SAR_SCENE_MODE_PORTRAIT,
		SAR_SCENE_MODE_SNOW,
		SAR_SCENE_MODE_SPORTS,
		SAR_SCENE_MODE_STEADYPHOTO,
		SAR_SCENE_MODE_SUNSET,
		SAR_SCENE_MODE_THEATRE,
    };
    
    class CLASS_DECLSPEC SarCameraDeviceInfo : SarNonCopyable
    {
    public:
        WINAPI SarCameraDeviceInfo();
        WINAPI ~SarCameraDeviceInfo();

        WINAPI SarCameraDeviceInfo(const SarCameraDeviceInfo& rhs);
        SarCameraDeviceInfo& WINAPI operator=(const SarCameraDeviceInfo& rhs);
        
    private:
        struct SarImpl;
        SarImpl* impl_;
        
        friend class SarCameraDevice;
        friend class SarRecognizer;
    };
    
    struct CLASS_DECLSPEC SarCameraFpsRange
    {
        float min_;
        float max_;
    };
    
    class CLASS_DECLSPEC SarImageHolder : SarNonCopyable
    {
    public:
        WINAPI ~SarImageHolder();
        
        int32_t WINAPI sarGetImageSizeInBytes() const;
        int32_t WINAPI sarGetImage(SarImage* image, int32_t maxSizeInBytes) const;
        
    private:
        struct SarImpl;
        SarImpl *impl_;

        WINAPI SarImageHolder(SarImpl *impl);

        friend class SarCameraImageListenerProxy;
    };
    
    class CLASS_DECLSPEC SarCameraImageListener
    {
    public:
        virtual WINAPI ~SarCameraImageListener() {}
        virtual void WINAPI sarOnImage(const SarImageHolder& imageHolder, uint64_t timestamp) = 0;
    };
    
    class CLASS_DECLSPEC SarCameraShutterListener
    {
    public:
        virtual WINAPI ~SarCameraShutterListener() {}
        virtual void WINAPI sarOnShutter() = 0;
    };
    
    class CLASS_DECLSPEC SarCameraAutoAdjustListener
    {
    public:
        virtual WINAPI ~SarCameraAutoAdjustListener() {}
        virtual void WINAPI sarOnAutoAdjust(bool success) = 0;
    };

    class CLASS_DECLSPEC SarCameraErrorListener
    {
    public:
        virtual WINAPI ~SarCameraErrorListener() {}
        virtual void WINAPI sarOnError(int32_t error) = 0;
    };

    struct CLASS_DECLSPEC SarCameraArea
    {
        float left_;
        float top_;
        float right_;
        float bottom_;
        float weight_;
    };
    
    class CLASS_DECLSPEC SarCameraDevice : SarNonCopyable
    {
    public:
        static const int SAR_INVALID_CAMERA_ID = -1;

        WINAPI SarCameraDevice(SarSmart* smart);
        WINAPI SarCameraDevice(SarSmart* smart, int32_t cameraId, void* nativeDevice = NULL);
        WINAPI ~SarCameraDevice();
        bool WINAPI sarIsConstructorFailed() const;
        
        // setting
        int32_t WINAPI sarSetNativeVideoOutput(void* nativeVideoOutput); // * on Android, specify SurfaceView to this API.
        int32_t WINAPI sarSetVideoImageListener(SarCameraImageListener* listener);
        int32_t WINAPI sarSetVideoImageSize(int32_t width, int32_t height);
        int32_t WINAPI sarSetVideoImageFormat(SarImageFormat format);
        int32_t WINAPI sarSetVideoImageFpsRange(float min, float max);
        int32_t WINAPI sarSetStillImageListener(SarCameraImageListener* listener);
        int32_t WINAPI sarSetStillImageSize(int32_t width, int32_t height);
        int32_t WINAPI sarSetStillImageFormat(SarImageFormat format);
        int32_t WINAPI sarSetShutterListener(SarCameraShutterListener* listener);
        int32_t WINAPI sarSetFocusMode(SarFocusMode mode);
        int32_t WINAPI sarSetFocusAreas(SarCameraArea* areas, int32_t numAreas);
        int32_t WINAPI sarSetExposureMode(SarExposureMode mode);
        int32_t WINAPI sarSetExposureAreas(SarCameraArea* areas, int32_t numAreas);
        int32_t WINAPI sarSetFlashMode(SarFlashMode mode);
        int32_t WINAPI sarSetWhiteBalanceMode(SarWhiteBalanceMode mode);
        int32_t WINAPI sarSetSceneMode(SarSceneMode mode);
        int32_t WINAPI sarSetAutoFocusListener(SarCameraAutoAdjustListener* listener);
        int32_t WINAPI sarSetAutoExposureListener(SarCameraAutoAdjustListener* listener);
        int32_t WINAPI sarSetAutoWhiteBalanceListener(SarCameraAutoAdjustListener* listener);
        int32_t WINAPI sarSetErrorListener(SarCameraErrorListener* listener);
        int32_t WINAPI sarSetOwningNativeDevice(bool isOwning);
        
        // get info
        static int32_t WINAPI sarGetDefaultCameraId(SarSmart* smart, SarFacing facing, int32_t* cameraId);
        int32_t WINAPI sarGetSupportedVideoImageSize(SarSize* sizes, int32_t maxSizes) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetSupportedVideoImageFormat(SarImageFormat* formats, int32_t maxFormats) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetSupportedVideoImageFpsRange(SarCameraFpsRange* ranges, int32_t maxRanges) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetSupportedStillImageSize(SarSize* sizes, int32_t maxSizes) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetSupportedStilImageFormat(SarImageFormat* formats, int32_t maxFormats) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetSupportedFocusMode(SarFocusMode* modes, int32_t maxModes) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetMaxNumFocusAreas() const;
        int32_t WINAPI sarGetSupportedFlashMode(SarFlashMode* modes, int32_t maxModes) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetSupportedExposureMode(SarExposureMode* modes, int32_t maxModes) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetMaxNumExposureAreas() const;
        int32_t WINAPI sarGetSupportedWhiteBalanceMode(SarWhiteBalanceMode* modes, int32_t maxModes) const; // * returns the number of data written to the specified buffer or error code
        int32_t WINAPI sarGetSupportedSceneMode(SarSceneMode* modes, int32_t maxModes) const; // * returns the number of data written to the specified buffer or error code
        
        int32_t WINAPI sarGetDeviceInfo(SarCameraDeviceInfo* info) const;
        int32_t WINAPI sarGetDeviceInfo(SarCameraDeviceInfo* info, int32_t scaledWidth, int32_t scaledHeight, bool isStillImage = false) const;
        int32_t WINAPI sarGetFovY(float* fovy, float heightRatio = 1.0f, bool* calibrated = NULL) const;
        int32_t WINAPI sarGetFovY(float* fovy, float heightRatio, bool* calibrated, bool isStillImage) const;
        int32_t WINAPI sarGetFovY(float* fovy, int targetWidth, int targetHeight, bool* getfromapi = NULL, bool* calibrated = NULL) const;	// Android only, get fovy by Android API, otherwise return error

        int32_t WINAPI sarGetFacing(SarFacing* facing) const;
        int32_t WINAPI sarGetRotation(SarRotation* rotation) const;
        int32_t WINAPI sarGetNativeDevice(void** nativeDevice) const;
        
        // start and stop
        int32_t WINAPI sarStart();
        int32_t WINAPI sarStop();
        
        // misc
        int32_t WINAPI sarCaptureStillImage();
        int32_t WINAPI sarRunAutoFocus();
        int32_t WINAPI sarRunAutoExposure();
        int32_t WINAPI sarRunAutoWhiteBalance();

    private:
        struct SarImpl;
        SarImpl* impl_;
    };
} // end of namespace smartar
