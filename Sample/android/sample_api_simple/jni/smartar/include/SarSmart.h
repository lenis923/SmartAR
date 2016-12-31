#pragma once

#include "SarCommon.h"


namespace sarSmartar {
    class CLASS_DECLSPEC SarSmart : SarNonCopyable
    {
    public:
        WINAPI SarSmart(void* nativeContext, void* nativeEnv, const char* filePath, SarMemoryAllocator* mallocator = NULL); // * on Android, specify Context and Env to this API.
        WINAPI ~SarSmart();
    
        bool WINAPI sarIsConstructorFailed() const;
        int32_t WINAPI sarGetInitResultCode() const;
        int32_t WINAPI sarGetNativeContext(void** nativeContext) const;

    private:
        struct SarImpl;
        SarImpl* impl_;
        
        friend class SarImage;
        friend class SarFileStreamIn;
        friend class SarAssetStreamIn;
        friend class SarFileStreamOut;
        
        friend class SarSensorDevice;
        friend class SarScreenDevice;
        friend class SarCameraDevice;
        friend class SarCameraImageDrawer;
        friend class SarRecognizer;
        
        friend class SarLearnedImageTarget;
        friend class SarCompoundTarget;
        friend class SarSceneMapTarget;
        
    };
} // end of namespace smartar
